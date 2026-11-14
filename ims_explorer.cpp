#include "ims_explorer.h"
#include "ims_theme.h"
#include "ims_fx.h"
#include "ims_toast.h"
#include "globalfunctions.h"

#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QKeySequence>
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
#include <QDebug>
#include <QVariant>
#include <QBrush>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace {
constexpr int SortRole   = Qt::UserRole + 1;   // value used for sorting / export
constexpr int StatusRole = Qt::UserRole + 2;   // 0 = ok, 1 = low, 2 = out

enum Column { ColId, ColName, ColCategory, ColSupplier, ColQty, ColMin,
              ColPrice, ColValue, ColReorder, ColStatus, ColDate, ColCount };
} // namespace

// ---------------------------------------------------------------------------
// Proxy: text + category + low-stock filtering
// ---------------------------------------------------------------------------
class ExplorerProxy : public QSortFilterProxyModel
{
public:
    explicit ExplorerProxy(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    QString text;
    QString category;       // empty = all
    bool lowOnly = false;

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        const QAbstractItemModel *m = sourceModel();
        if (!m)
            return true;

        if (lowOnly && m->index(row, ColId, parent).data(StatusRole).toInt() == 0)
            return false;

        if (!category.isEmpty()
            && m->index(row, ColCategory, parent).data(Qt::DisplayRole).toString() != category)
            return false;

        if (!text.isEmpty()) {
            bool hit = false;
            for (int c = ColId; c <= ColSupplier && !hit; ++c)
                hit = m->index(row, c, parent).data(Qt::DisplayRole).toString()
                          .contains(text, Qt::CaseInsensitive);
            if (!hit)
                return false;
        }
        return true;
    }
};

// ---------------------------------------------------------------------------
InventoryExplorer::InventoryExplorer(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("InventoryExplorer");
    setProperty("ims_managed", true);   // ThemeManager::prepare leaves our roles alone

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(16);

    // ---- header ----------------------------------------------------------
    auto *head = new QVBoxLayout;
    head->setSpacing(2);
    auto *title = new QLabel(tr("Inventory Explorer"), this);
    title->setProperty("role", "title");
    auto *sub = new QLabel(tr("Search, filter, sort and export every product in stock."), this);
    sub->setProperty("role", "muted");
    head->addWidget(title);
    head->addWidget(sub);
    root->addLayout(head);

    // ---- KPI strip -------------------------------------------------------
    auto *stats = new QHBoxLayout;
    stats->setSpacing(16);
    auto makeStat = [this, stats](const QString &caption) -> QLabel * {
        auto *card = new QFrame(this);
        card->setProperty("role", "card");
        auto *v = new QVBoxLayout(card);
        v->setContentsMargins(18, 14, 18, 14);
        v->setSpacing(4);
        auto *cap = new QLabel(caption, card);
        cap->setProperty("role", "muted");
        auto *val = new QLabel("0", card);
        val->setProperty("role", "stat");
        v->addWidget(cap);
        v->addWidget(val);
        Fx::installHoverLift(card);
        stats->addWidget(card, 1);
        return val;
    };
    m_statSkus      = makeStat(tr("Products"));
    m_statUnits     = makeStat(tr("Units in stock"));
    m_statValue     = makeStat(tr("Inventory value"));
    m_statAttention = makeStat(tr("Need attention"));
    root->addLayout(stats);

    // ---- toolbar ---------------------------------------------------------
    auto *bar = new QHBoxLayout;
    bar->setSpacing(10);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Search by ID, name, category or supplier   (Ctrl+F)"));
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumWidth(300);

    m_category = new QComboBox(this);
    m_category->setMinimumWidth(190);

    m_lowOnly = new QCheckBox(tr("Low stock only"), this);

    auto *refreshBtn = new QPushButton(tr("Refresh"), this);
    auto *exportBtn = new QPushButton(tr("Export CSV"), this);
    exportBtn->setProperty("role", "primary");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setCursor(Qt::PointingHandCursor);

    bar->addWidget(m_search, 2);
    bar->addWidget(m_category, 1);
    bar->addWidget(m_lowOnly);
    bar->addStretch(1);
    bar->addWidget(refreshBtn);
    bar->addWidget(exportBtn);
    root->addLayout(bar);

    // ---- table -----------------------------------------------------------
    m_model = new QStandardItemModel(0, ColCount, this);
    m_model->setHorizontalHeaderLabels({tr("ID"), tr("Product"), tr("Category"), tr("Supplier"),
                                        tr("Qty"), tr("Min level"), tr("Unit price"),
                                        tr("Stock value"), tr("Reorder"), tr("Status"),
                                        tr("Purchased")});

    m_proxy = new ExplorerProxy(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortRole(SortRole);

    m_table = new QTableView(this);
    m_table->setModel(m_proxy);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(ColName, Qt::AscendingOrder);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->setFrameShape(QFrame::NoFrame);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(ThemeManager::instance().rowHeight());
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(ColName, QHeaderView::Stretch);
    root->addWidget(m_table, 1);

    m_footer = new QLabel(this);
    m_footer->setProperty("role", "muted");
    root->addWidget(m_footer);

    // ---- wiring ----------------------------------------------------------
    connect(m_search, &QLineEdit::textChanged, this, [this]() { applyFilters(); });
    connect(m_category, &QComboBox::currentIndexChanged, this, [this]() { applyFilters(); });
    connect(m_lowOnly, &QCheckBox::toggled, this, [this]() { applyFilters(); });
    connect(refreshBtn, &QPushButton::clicked, this, [this]() {
        refresh();
        Toast::notify(this, tr("Inventory refreshed"), Toast::Info, 1800);
    });
    connect(exportBtn, &QPushButton::clicked, this, [this]() { exportCsv(); });

    auto *find = new QShortcut(QKeySequence::Find, this);
    find->setContext(Qt::WidgetWithChildrenShortcut);
    connect(find, &QShortcut::activated, this, [this]() { focusSearch(); });

    connect(&ThemeManager::instance(), &ThemeManager::appearanceChanged, this, [this]() {
        m_table->verticalHeader()->setDefaultSectionSize(ThemeManager::instance().rowHeight());
        refresh();
    });

    refresh();
}

void InventoryExplorer::focusSearch()
{
    m_search->setFocus();
    m_search->selectAll();
}

// ---------------------------------------------------------------------------
void InventoryExplorer::refresh()
{
    const ThemeManager &tm = ThemeManager::instance();
    const QString keepCategory = m_proxy->category;

    m_model->removeRows(0, m_model->rowCount());

    QSqlQuery q;
    const bool ok = q.exec(
        "SELECT p.product_id, p.name, p.category, COALESCE(s.name, '-'), "
        "       p.quantity, p.minimum_stock_level, p.price, p.purchase_date "
        "FROM products p LEFT JOIN suppliers s ON s.supplier_id = p.supplier_id "
        "ORDER BY p.name");
    if (!ok) {
        qDebug() << "InventoryExplorer query failed:" << q.lastError().text();
        Toast::notify(this, tr("Could not load products: %1").arg(q.lastError().text()), Toast::Error);
        return;
    }

    auto cell = [](const QString &text, const QVariant &sortValue, bool right) {
        auto *it = new QStandardItem(text);
        it->setEditable(false);
        it->setData(sortValue, SortRole);
        it->setTextAlignment((right ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
        return it;
    };

    QStringList categories;
    qlonglong units = 0;
    double totalValue = 0.0;
    int attention = 0;
    int skus = 0;
    const QLocale loc;

    while (q.next()) {
        const qlonglong id = q.value(0).toLongLong();
        const QString name = q.value(1).toString();
        const QString cat = q.value(2).toString();
        const QString supplier = q.value(3).toString();
        const int qty = q.value(4).toInt();
        const int minLevel = q.value(5).toInt();
        const double price = q.value(6).toDouble();
        const QString date = q.value(7).toString();

        const double value = qty * price;
        const int status = (qty <= 0) ? 2 : (qty <= minLevel ? 1 : 0);
        const int reorder = (status > 0) ? qMax(0, minLevel * 2 - qty) : 0;

        ++skus;
        units += qty;
        totalValue += value;
        if (status > 0)
            ++attention;
        if (!categories.contains(cat))
            categories << cat;

        QList<QStandardItem *> row;
        row << cell(QString::number(id), id, true)
            << cell(name, name.toLower(), false)
            << cell(cat, cat.toLower(), false)
            << cell(supplier, supplier.toLower(), false)
            << cell(loc.toString(qty), qty, true)
            << cell(loc.toString(minLevel), minLevel, true)
            << cell(loc.toString(price, 'f', 2), price, true)
            << cell(loc.toString(value, 'f', 2), value, true)
            << cell(reorder > 0 ? loc.toString(reorder) : QStringLiteral("-"), reorder, true);

        // status "chip": colour-coded text on a tinted background
        const QString statusText = (status == 2) ? tr("Out of stock")
                                 : (status == 1) ? tr("Low stock") : tr("In stock");
        const QString fg = (status == 2) ? "bad" : (status == 1) ? "warn" : "ok";
        auto *st = cell(statusText, status, false);
        st->setForeground(QBrush(tm.color(fg)));
        st->setBackground(QBrush(tm.color(fg + "Bg")));
        QFont bold = st->font();
        bold.setBold(true);
        st->setFont(bold);
        row << st;
        row << cell(date, date, false);

        row.at(ColId)->setData(status, StatusRole);
        if (status > 0)
            row.at(ColQty)->setForeground(QBrush(tm.color(fg)));

        m_model->appendRow(row);
    }

    // category filter (rebuilt without firing filter updates)
    categories.sort(Qt::CaseInsensitive);
    {
        const QSignalBlocker block(m_category);
        m_category->clear();
        m_category->addItem(tr("All categories"), QString());
        for (const QString &c : categories)
            m_category->addItem(c, c);
        const int idx = m_category->findData(keepCategory);
        m_category->setCurrentIndex(idx >= 0 ? idx : 0);
    }

    // KPI strip
    m_statSkus->setText(loc.toString(skus));
    m_statUnits->setText(loc.toString(units));
    m_statValue->setText(loc.toString(totalValue, 'f', 2));
    m_statAttention->setText(loc.toString(attention));
    for (QLabel *l : {m_statSkus, m_statUnits, m_statValue, m_statAttention})
        Fx::countUp(l, 800);

    applyFilters();
}

void InventoryExplorer::applyFilters()
{
    m_proxy->text = m_search->text().trimmed();
    m_proxy->category = m_category->currentData().toString();
    m_proxy->lowOnly = m_lowOnly->isChecked();
    m_proxy->invalidate();
    updateFooter();
}

void InventoryExplorer::updateFooter()
{
    m_footer->setText(tr("Showing %1 of %2 products").arg(m_proxy->rowCount()).arg(m_model->rowCount()));
}

// ---------------------------------------------------------------------------
void InventoryExplorer::exportCsv()
{
    if (m_proxy->rowCount() == 0) {
        Toast::notify(this, tr("There is nothing to export."), Toast::Warning);
        return;
    }

    const QString suggested = QDir::homePath() + "/inventory_"
                              + QDate::currentDate().toString("yyyy-MM-dd") + ".csv";
    const QString path = QFileDialog::getSaveFileName(this, tr("Export inventory"), suggested,
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
    out.setGenerateByteOrderMark(true);   // so Excel opens it with the right encoding

    QStringList header;
    for (int c = 0; c < m_model->columnCount(); ++c)
        header << esc(m_model->headerData(c, Qt::Horizontal).toString());
    out << header.join(',') << "\n";

    const int rows = m_proxy->rowCount();
    for (int r = 0; r < rows; ++r) {
        QStringList line;
        for (int c = 0; c < m_proxy->columnCount(); ++c) {
            const QModelIndex idx = m_proxy->index(r, c);
            const QVariant v = idx.data(SortRole);
            QString s;
            if (c == ColStatus) {
                s = idx.data(Qt::DisplayRole).toString();
            } else if (v.userType() == QMetaType::Double) {
                s = QString::number(v.toDouble(), 'f', 2);
            } else if (v.userType() == QMetaType::LongLong || v.userType() == QMetaType::Int) {
                s = QString::number(v.toLongLong());
            } else {
                s = idx.data(Qt::DisplayRole).toString();
            }
            line << esc(s);
        }
        out << line.join(',') << "\n";
    }
    out.flush();
    file.close();

    GlobalFunctions::audit_logs("Export Inventory",
                                QString("Exported %1 products to CSV").arg(rows));
    Toast::notify(this, tr("Exported %1 products to CSV").arg(rows), Toast::Success);
}
