#include "ims_datagrid.h"
#include "ims_theme.h"
#include "ims_fx.h"
#include "ims_toast.h"
#include "globalfunctions.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QKeySequence>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QLocale>
#include <QTextStream>
#include <QStringList>
#include <QSignalBlocker>
#include <QSet>
#include <QVector>
#include <QDebug>
#include <QVariant>
#include <QBrush>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace {
constexpr int SortRole = Qt::UserRole + 1;

// 0 = good, 1 = warning, 2 = bad, 3 = neutral
int statusTone(const QString &text)
{
    const QString t = text.toLower();
    static const QStringList bad  = {"cancel", "out of", "expired", "inactive", "risk", "resign", "remove",
                                     "fail", "terminat", "locked"};
    static const QStringList warn = {"pending", "low", "upcoming", "unread", "scheduled", "draft", "contract",
                                     "part-time", "intern"};
    static const QStringList good = {"complete", "active", "in stock", "employed", "read", "add", "info",
                                     "full-time", "paid"};
    for (const QString &k : bad)  if (t.contains(k)) return 2;
    for (const QString &k : warn) if (t.contains(k)) return 1;
    for (const QString &k : good) if (t.contains(k)) return 0;
    return 3;
}

QString toneName(int tone)
{
    return tone == 2 ? "bad" : tone == 1 ? "warn" : tone == 0 ? "ok" : QString();
}
} // namespace

// ---------------------------------------------------------------------------
// Proxy: free-text search + status filter
// ---------------------------------------------------------------------------
class GridProxy : public QSortFilterProxyModel
{
public:
    explicit GridProxy(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    QString text;
    int statusColumn = -1;
    QString statusValue;     // empty = all

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        const QAbstractItemModel *m = sourceModel();
        if (!m)
            return true;

        if (statusColumn >= 0 && !statusValue.isEmpty()
            && m->index(row, statusColumn, parent).data(Qt::DisplayRole).toString() != statusValue)
            return false;

        if (!text.isEmpty()) {
            for (int c = 0; c < m->columnCount(); ++c) {
                if (m->index(row, c, parent).data(Qt::DisplayRole).toString()
                        .contains(text, Qt::CaseInsensitive))
                    return true;
            }
            return false;
        }
        return true;
    }
};

// ---------------------------------------------------------------------------
DataGrid::DataGrid(const GridSpec &spec, QWidget *parent)
    : QWidget(parent), m_spec(spec)
{
    setObjectName("DataGrid_" + spec.id);
    setProperty("ims_managed", true);

    for (int i = 0; i < m_spec.columns.size(); ++i) {
        if (m_spec.columns.at(i).type == GridColumn::Status) {
            m_statusColumn = i;
            break;
        }
    }

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 16);
    root->setSpacing(14);

    // ---- header ----------------------------------------------------------
    auto *title = new QLabel(m_spec.title, this);
    title->setProperty("role", "title");
    auto *sub = new QLabel(m_spec.subtitle, this);
    sub->setProperty("role", "muted");
    auto *head = new QVBoxLayout;
    head->setSpacing(2);
    head->addWidget(title);
    head->addWidget(sub);
    root->addLayout(head);

    // ---- KPI strip -------------------------------------------------------
    if (!m_spec.stats.isEmpty()) {
        auto *strip = new QHBoxLayout;
        strip->setSpacing(14);
        for (const GridStat &st : m_spec.stats) {
            auto *card = new QFrame(this);
            card->setProperty("role", "card");
            auto *v = new QVBoxLayout(card);
            v->setContentsMargins(16, 12, 16, 12);
            v->setSpacing(2);
            auto *cap = new QLabel(st.label, card);
            cap->setProperty("role", "muted");
            auto *val = new QLabel("0", card);
            val->setProperty("role", "stat");
            v->addWidget(cap);
            v->addWidget(val);
            Fx::installHoverLift(card);
            strip->addWidget(card, 1);
            m_statLabels << val;
        }
        root->addLayout(strip);
    }

    // ---- toolbar ---------------------------------------------------------
    auto *bar = new QHBoxLayout;
    bar->setSpacing(10);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Search every column   (Ctrl+F)"));
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumWidth(280);
    bar->addWidget(m_search, 2);

    if (m_statusColumn >= 0) {
        m_status = new QComboBox(this);
        m_status->setMinimumWidth(170);
        bar->addWidget(m_status, 1);
    }
    bar->addStretch(1);

    auto *colBtn = new QPushButton(tr("Columns"), this);
    auto *colMenu = new QMenu(colBtn);
    colBtn->setMenu(colMenu);
    bar->addWidget(colBtn);

    m_detailBtn = new QPushButton(tr("Details"), this);
    m_detailBtn->setCheckable(true);
    m_detailBtn->setChecked(true);
    bar->addWidget(m_detailBtn);

    auto *refreshBtn = new QPushButton(tr("Refresh"), this);
    auto *exportBtn = new QPushButton(tr("Export CSV"), this);
    exportBtn->setProperty("role", "primary");
    bar->addWidget(refreshBtn);
    bar->addWidget(exportBtn);
    for (QPushButton *b : {colBtn, m_detailBtn, refreshBtn, exportBtn})
        b->setCursor(Qt::PointingHandCursor);
    root->addLayout(bar);

    // ---- table + detail panel -------------------------------------------
    m_model = new QStandardItemModel(0, m_spec.columns.size(), this);
    QStringList headers;
    for (const GridColumn &c : m_spec.columns)
        headers << c.header;
    m_model->setHorizontalHeaderLabels(headers);

    m_proxy = new GridProxy(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortRole(SortRole);
    m_proxy->statusColumn = m_statusColumn;

    m_table = new QTableView(this);
    m_table->setModel(m_proxy);
    m_table->setSortingEnabled(true);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->setFrameShape(QFrame::NoFrame);
    m_table->setTextElideMode(Qt::ElideRight);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(ThemeManager::instance().rowHeight());
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setMaximumSectionSize(420);
    m_table->horizontalHeader()->setStretchLastSection(true);

    for (int c = 0; c < m_spec.columns.size(); ++c) {
        auto *act = colMenu->addAction(m_spec.columns.at(c).header);
        act->setCheckable(true);
        act->setChecked(m_spec.columns.at(c).visible);
        m_table->setColumnHidden(c, !m_spec.columns.at(c).visible);
        connect(act, &QAction::toggled, this, [this, c](bool on) { m_table->setColumnHidden(c, !on); });
    }

    // detail drawer
    m_detailCard = new QFrame(this);
    m_detailCard->setProperty("role", "card");
    m_detailCard->setFixedWidth(320);
    auto *dl = new QVBoxLayout(m_detailCard);
    dl->setContentsMargins(16, 14, 16, 14);
    dl->setSpacing(8);
    auto *dTitle = new QLabel(tr("Record details"), m_detailCard);
    dTitle->setProperty("role", "muted");
    dl->addWidget(dTitle);
    auto *scroll = new QScrollArea(m_detailCard);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *inner = new QWidget(scroll);
    m_detailBody = new QVBoxLayout(inner);
    m_detailBody->setContentsMargins(0, 0, 4, 0);
    m_detailBody->setSpacing(4);
    scroll->setWidget(inner);
    dl->addWidget(scroll, 1);

    auto *body = new QHBoxLayout;
    body->setSpacing(14);
    body->addWidget(m_table, 1);
    body->addWidget(m_detailCard);
    root->addLayout(body, 1);

    m_footer = new QLabel(this);
    m_footer->setProperty("role", "muted");
    root->addWidget(m_footer);

    // ---- wiring ----------------------------------------------------------
    connect(m_search, &QLineEdit::textChanged, this, [this]() { applyFilters(); });
    if (m_status)
        connect(m_status, &QComboBox::currentIndexChanged, this, [this]() { applyFilters(); });
    connect(m_detailBtn, &QPushButton::toggled, this, [this](bool on) { m_detailCard->setVisible(on); });
    connect(refreshBtn, &QPushButton::clicked, this, [this]() {
        refresh();
        Toast::notify(this, tr("%1 refreshed").arg(m_spec.title), Toast::Info, 1600);
    });
    connect(exportBtn, &QPushButton::clicked, this, [this]() { exportCsv(); });
    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this]() { showDetails(); });

    auto *find = new QShortcut(QKeySequence::Find, this);
    find->setContext(Qt::WidgetWithChildrenShortcut);
    connect(find, &QShortcut::activated, this, [this]() { focusSearch(); });

    connect(&ThemeManager::instance(), &ThemeManager::appearanceChanged, this, [this]() {
        m_table->verticalHeader()->setDefaultSectionSize(ThemeManager::instance().rowHeight());
        refresh();
    });

    refresh();
}

void DataGrid::focusSearch()
{
    m_search->setFocus();
    m_search->selectAll();
}

// ---------------------------------------------------------------------------
void DataGrid::refresh()
{
    const ThemeManager &tm = ThemeManager::instance();
    const QLocale loc;
    const QString keepStatus = m_proxy->statusValue;

    m_model->removeRows(0, m_model->rowCount());

    QSqlQuery q;
    if (!q.exec(m_spec.sql)) {
        qDebug() << "DataGrid" << m_spec.id << "query failed:" << q.lastError().text();
        Toast::notify(this, tr("Could not load %1: %2").arg(m_spec.title, q.lastError().text()), Toast::Error);
        return;
    }

    QSet<QString> statuses;
    QVector<double> sums(m_spec.columns.size(), 0.0);

    while (q.next()) {
        QList<QStandardItem *> row;
        for (int c = 0; c < m_spec.columns.size(); ++c) {
            const GridColumn &col = m_spec.columns.at(c);
            const QVariant raw = q.value(c);
            QString text;
            QVariant sortValue;
            bool right = false;

            switch (col.type) {
            case GridColumn::Int:
                text = loc.toString(raw.toLongLong());
                sortValue = raw.toLongLong();
                right = true;
                if (col.sum) sums[c] += raw.toDouble();
                break;
            case GridColumn::Money:
                text = loc.toString(raw.toDouble(), 'f', 2);
                sortValue = raw.toDouble();
                right = true;
                if (col.sum) sums[c] += raw.toDouble();
                break;
            case GridColumn::Percent:
                text = loc.toString(raw.toDouble(), 'f', 0) + "%";
                sortValue = raw.toDouble();
                right = true;
                break;
            case GridColumn::Date:
                text = raw.toString();
                sortValue = text;
                break;
            case GridColumn::Status:
                text = raw.toString();
                sortValue = text.toLower();
                statuses.insert(text);
                break;
            default:
                text = raw.toString();
                sortValue = text.toLower();
                break;
            }

            auto *it = new QStandardItem(text);
            it->setEditable(false);
            it->setData(sortValue, SortRole);
            it->setTextAlignment((right ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);

            if (col.type == GridColumn::Status) {
                const QString tn = toneName(statusTone(text));
                if (!tn.isEmpty()) {
                    it->setForeground(QBrush(tm.color(tn)));
                    it->setBackground(QBrush(tm.color(tn + "Bg")));
                    QFont f = it->font();
                    f.setBold(true);
                    it->setFont(f);
                }
            }
            row << it;
        }
        m_model->appendRow(row);
    }

    // status filter options
    if (m_status) {
        QStringList list = statuses.values();
        list.sort(Qt::CaseInsensitive);
        const QSignalBlocker block(m_status);
        m_status->clear();
        m_status->addItem(tr("All %1").arg(m_spec.columns.at(m_statusColumn).header.toLower()), QString());
        for (const QString &s : list)
            m_status->addItem(s, s);
        const int idx = m_status->findData(keepStatus);
        m_status->setCurrentIndex(idx >= 0 ? idx : 0);
    }

    // KPI cards
    for (int i = 0; i < m_spec.stats.size() && i < m_statLabels.size(); ++i) {
        const GridStat &st = m_spec.stats.at(i);
        QSqlQuery sq;
        QString shown = "-";
        if (sq.exec(st.sql) && sq.next()) {
            const double v = sq.value(0).toDouble();
            shown = (st.format == 'm') ? loc.toString(v, 'f', 2)
                  : (st.format == 'p') ? loc.toString(v, 'f', 1) + "%"
                                       : loc.toString(qRound64(v));
        }
        m_statLabels.at(i)->setText(shown);
        Fx::countUp(m_statLabels.at(i), 700);
    }

    // remember footer totals
    QStringList parts;
    for (int c = 0; c < m_spec.columns.size(); ++c) {
        const GridColumn &col = m_spec.columns.at(c);
        if (!col.sum)
            continue;
        parts << QString("%1: %2").arg(col.header,
                    col.type == GridColumn::Money ? loc.toString(sums[c], 'f', 2) : loc.toString(qRound64(sums[c])));
    }
    m_footer->setProperty("totals", parts.join(QString("   ") + QChar(0x00B7) + QString("   ")));

    applyFilters();
    showDetails();
}

void DataGrid::applyFilters()
{
    m_proxy->text = m_search->text().trimmed();
    m_proxy->statusValue = m_status ? m_status->currentData().toString() : QString();
    m_proxy->invalidate();
    updateFooter();
}

void DataGrid::updateFooter()
{
    QString s = tr("Showing %1 of %2 rows").arg(m_proxy->rowCount()).arg(m_model->rowCount());
    const QString totals = m_footer->property("totals").toString();
    if (!totals.isEmpty())
        s += QString("      ") + QChar(0x03A3) + QString("  ") + totals + tr("   (all rows)");
    m_footer->setText(s);
}

// ---------------------------------------------------------------------------
void DataGrid::showDetails()
{
    while (QLayoutItem *it = m_detailBody->takeAt(0)) {
        if (it->widget())
            it->widget()->deleteLater();
        delete it;
    }

    const QModelIndex cur = m_table->currentIndex();
    if (!cur.isValid()) {
        auto *hint = new QLabel(tr("Select a row to see every field of the record."));
        hint->setProperty("role", "muted");
        hint->setWordWrap(true);
        m_detailBody->addWidget(hint);
        m_detailBody->addStretch(1);
        return;
    }

    const int srow = m_proxy->mapToSource(cur).row();
    for (int c = 0; c < m_spec.columns.size(); ++c) {
        const GridColumn &col = m_spec.columns.at(c);
        const QStandardItem *it = m_model->item(srow, c);
        const QString value = it ? it->text() : QString();

        auto *key = new QLabel(col.header);
        key->setProperty("role", "muted");
        m_detailBody->addWidget(key);

        auto *val = new QLabel(value.isEmpty() ? QStringLiteral("-") : value);
        val->setWordWrap(true);
        val->setTextInteractionFlags(Qt::TextSelectableByMouse);
        if (col.type == GridColumn::Status) {
            const QString tn = toneName(statusTone(value));
            if (!tn.isEmpty())
                val->setProperty("role", "chip-" + tn);
            m_detailBody->addWidget(val, 0, Qt::AlignLeft);
        } else {
            m_detailBody->addWidget(val);
        }
        m_detailBody->addSpacing(6);
    }
    m_detailBody->addStretch(1);
}

// ---------------------------------------------------------------------------
void DataGrid::exportCsv()
{
    if (m_proxy->rowCount() == 0) {
        Toast::notify(this, tr("There is nothing to export."), Toast::Warning);
        return;
    }

    const QString suggested = QDir::homePath() + "/" + m_spec.id + "_"
                              + QDate::currentDate().toString("yyyy-MM-dd") + ".csv";
    const QString path = QFileDialog::getSaveFileName(this, tr("Export %1").arg(m_spec.title), suggested,
                                                      tr("CSV files (*.csv)"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        Toast::notify(this, tr("Could not write %1").arg(path), Toast::Error);
        return;
    }

    auto esc = [](QString s) {
        s.replace('"', "\"\"");
        return "\"" + s + "\"";
    };

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    out.setGenerateByteOrderMark(true);

    QList<int> cols;
    QStringList header;
    for (int c = 0; c < m_spec.columns.size(); ++c) {
        if (m_table->isColumnHidden(c))
            continue;
        cols << c;
        header << esc(m_spec.columns.at(c).header);
    }
    out << header.join(',') << "\n";

    const int rows = m_proxy->rowCount();
    for (int r = 0; r < rows; ++r) {
        QStringList line;
        for (int c : cols) {
            const QModelIndex idx = m_proxy->index(r, c);
            const QVariant v = idx.data(SortRole);
            const GridColumn::Type t = m_spec.columns.at(c).type;
            QString s;
            if (t == GridColumn::Money || t == GridColumn::Percent)
                s = QString::number(v.toDouble(), 'f', 2);
            else if (t == GridColumn::Int)
                s = QString::number(v.toLongLong());
            else
                s = idx.data(Qt::DisplayRole).toString();
            line << esc(s);
        }
        out << line.join(',') << "\n";
    }
    out.flush();
    file.close();

    GlobalFunctions::audit_logs("Export " + m_spec.title,
                                QString("Exported %1 rows from %2 to CSV").arg(rows).arg(m_spec.title));
    Toast::notify(this, tr("Exported %1 rows to CSV").arg(rows), Toast::Success);
}
