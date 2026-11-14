#include "ims_overview.h"
#include "ims_theme.h"
#include "ims_fx.h"

#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QProgressBar>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QVariantAnimation>
#include <QSqlQuery>
#include <QDate>
#include <QLocale>
#include <QMap>
#include <QVector>
#include <QStringList>

// ---------------------------------------------------------------------------
// MiniBars - tiny themed bar chart for the last N days
// ---------------------------------------------------------------------------
class MiniBars : public QWidget
{
public:
    explicit MiniBars(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMouseTracking(true);
        setMinimumHeight(170);
    }

    void setData(const QVector<double> &v, const QStringList &labels, const QStringList &tips)
    {
        m_values = v;
        m_labels = labels;
        m_tips = tips;
        if (!ThemeManager::instance().animationsEnabled()) {
            m_progress = 1.0;
            update();
            return;
        }
        auto *a = new QVariantAnimation(this);
        a->setDuration(700);
        a->setStartValue(0.0);
        a->setEndValue(1.0);
        a->setEasingCurve(QEasingCurve::OutCubic);
        connect(a, &QVariantAnimation::valueChanged, this, [this](const QVariant &x) {
            m_progress = x.toDouble();
            update();
        });
        a->start(QAbstractAnimation::DeleteWhenStopped);
    }

protected:
    QRectF plot() const { return QRectF(rect()).adjusted(4, 10, -4, -24); }

    int indexAt(const QPoint &pos) const
    {
        const int n = m_values.size();
        if (n == 0)
            return -1;
        const QRectF r = plot();
        const double step = r.width() / n;
        const int i = int((pos.x() - r.left()) / step);
        return (i >= 0 && i < n) ? i : -1;
    }

    void mouseMoveEvent(QMouseEvent *e) override
    {
        const int i = indexAt(e->position().toPoint());
        if (i != m_hover) {
            m_hover = i;
            update();
        }
        if (i >= 0 && i < m_tips.size())
            QToolTip::showText(e->globalPosition().toPoint(), m_tips.at(i), this);
    }

    void leaveEvent(QEvent *) override
    {
        m_hover = -1;
        update();
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const ThemeManager &tm = ThemeManager::instance();
        const int n = m_values.size();
        const QRectF r = plot();

        // baseline
        p.setPen(QPen(tm.color("border"), 1));
        p.drawLine(QPointF(r.left(), r.bottom()), QPointF(r.right(), r.bottom()));
        if (n == 0)
            return;

        double mx = 0;
        for (double v : m_values)
            mx = qMax(mx, v);
        if (mx <= 0)
            mx = 1;

        const double step = r.width() / n;
        const double bw = step * 0.62;
        QFont small = font();
        small.setPointSizeF(qMax(7.0, font().pointSizeF() - 2));
        p.setFont(small);

        for (int i = 0; i < n; ++i) {
            const double h = (r.height() - 6) * (m_values[i] / mx) * m_progress;
            const QRectF b(r.left() + i * step + (step - bw) / 2, r.bottom() - h, bw, h);
            QColor c = tm.color("btnPrimary");
            if (i != m_hover && i != n - 1)
                c.setAlpha(150);
            QPainterPath path;
            path.addRoundedRect(b, 2, 2);
            p.fillPath(path, c);

            if (i < m_labels.size() && (n <= 14 || i % 2 == 0)) {
                p.setPen(tm.color("text3"));
                p.drawText(QRectF(r.left() + i * step, r.bottom() + 4, step, 16), Qt::AlignCenter, m_labels.at(i));
            }
        }
    }

private:
    QVector<double> m_values;
    QStringList m_labels;
    QStringList m_tips;
    double m_progress = 1.0;
    int m_hover = -1;
};

// ---------------------------------------------------------------------------
namespace {

void clear(QLayout *l)
{
    while (QLayoutItem *it = l->takeAt(0)) {
        if (it->widget())
            it->widget()->deleteLater();
        delete it;
    }
}

QFrame *card(QWidget *parent, QVBoxLayout **bodyOut, const QString &title, const QString &sub = QString())
{
    auto *f = new QFrame(parent);
    f->setProperty("role", "card");
    auto *v = new QVBoxLayout(f);
    v->setContentsMargins(18, 14, 18, 16);
    v->setSpacing(8);
    auto *t = new QLabel(title, f);
    t->setStyleSheet("QLabel { font-weight: 600; }");
    v->addWidget(t);
    if (!sub.isEmpty()) {
        auto *s = new QLabel(sub, f);
        s->setProperty("role", "muted");
        v->addWidget(s);
    }
    *bodyOut = v;
    return f;
}

QString money(double v)
{
    return QLocale().toString(v, 'f', 2);
}

double scalar(const QString &sql, double def = 0.0)
{
    QSqlQuery q;
    if (q.exec(sql) && q.next())
        return q.value(0).toDouble();
    return def;
}
} // namespace

// ---------------------------------------------------------------------------
OverviewPage::OverviewPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("OverviewPage");
    setProperty("ims_managed", true);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);

    auto *content = new QWidget(scroll);
    scroll->setWidget(content);
    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);

    // ---- header + quick actions -----------------------------------------
    auto *head = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->setSpacing(2);
    auto *title = new QLabel(tr("Overview"), content);
    title->setProperty("role", "title");
    m_subtitle = new QLabel(content);
    m_subtitle->setProperty("role", "muted");
    titles->addWidget(title);
    titles->addWidget(m_subtitle);
    head->addLayout(titles, 1);

    struct Act { const char *text; const char *key; bool primary; };
    const Act acts[] = {
        {"Add product", "add_product", true}, {"Record sale", "add_sale", false},
        {"New order", "add_order", false},    {"Adjust stock", "adjust_stock", false},
        {"Data Center", "datacenter", false}, {"Appearance", "appearance", false},
    };
    for (const Act &a : acts) {
        auto *b = new QPushButton(tr(a.text), content);
        if (a.primary)
            b->setProperty("role", "primary");
        b->setCursor(Qt::PointingHandCursor);
        const QString key = a.key;
        connect(b, &QPushButton::clicked, this, [this, key]() { emit actionRequested(key); });
        head->addWidget(b);
    }
    root->addLayout(head);

    // ---- KPI cards -------------------------------------------------------
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);
    int idx = 0;
    auto kpi = [&](const QString &caption, const QString &hint, QLabel *&valueOut) {
        auto *f = new QFrame(content);
        f->setProperty("role", "card");
        f->setProperty("accent", QString::number(idx % 4));
        auto *v = new QVBoxLayout(f);
        v->setContentsMargins(18, 14, 18, 14);
        v->setSpacing(2);
        auto *cap = new QLabel(caption, f);
        cap->setProperty("role", "muted");
        valueOut = new QLabel("0", f);
        valueOut->setProperty("role", "stat");
        auto *h = new QLabel(hint, f);
        h->setProperty("role", "muted");
        v->addWidget(cap);
        v->addWidget(valueOut);
        v->addWidget(h);
        Fx::installHoverLift(f);
        grid->addWidget(f, idx / 3, idx % 3);
        ++idx;
    };
    kpi(tr("Revenue"),          tr("last 30 days"),        m_revenue);
    kpi(tr("Sales"),            tr("last 30 days"),        m_sales);
    kpi(tr("Pending orders"),   tr("waiting for action"),  m_pending);
    kpi(tr("Need attention"),   tr("low or out of stock"), m_attention);
    kpi(tr("Active promotions"), tr("running today"),      m_promos);
    kpi(tr("Stock value"),      tr("at unit price"),       m_stockValue);
    for (int c = 0; c < 3; ++c)
        grid->setColumnStretch(c, 1);
    root->addLayout(grid);

    // ---- revenue bars + top products ------------------------------------
    auto *mid = new QHBoxLayout;
    mid->setSpacing(14);
    {
        QVBoxLayout *body = nullptr;
        QFrame *f = card(content, &body, tr("Revenue, last 14 days"));
        m_barsTotal = new QLabel(f);
        m_barsTotal->setProperty("role", "muted");
        body->addWidget(m_barsTotal);
        m_bars = new MiniBars(f);
        body->addWidget(m_bars, 1);
        mid->addWidget(f, 3);
    }
    {
        QFrame *f = card(content, &m_topBody, tr("Top products"), tr("Units sold in the last 30 days"));
        mid->addWidget(f, 2);
    }
    root->addLayout(mid);

    // ---- alerts + activity ----------------------------------------------
    auto *low = new QHBoxLayout;
    low->setSpacing(14);
    {
        QFrame *f = card(content, &m_alertBody, tr("Stock alerts"), tr("Lowest stock first"));
        low->addWidget(f, 1);
    }
    {
        QFrame *f = card(content, &m_activityBody, tr("Recent activity"), tr("Latest audit-log entries"));
        low->addWidget(f, 1);
    }
    root->addLayout(low);
    root->addStretch(1);

    refresh();
}

// ---------------------------------------------------------------------------
void OverviewPage::refresh()
{
    const QLocale loc;

    // header
    QSqlQuery biz;
    QString business;
    if (biz.exec("SELECT business_name FROM users WHERE business_name IS NOT NULL LIMIT 1") && biz.next())
        business = biz.value(0).toString();
    m_subtitle->setText(loc.toString(QDate::currentDate(), "dddd, d MMMM yyyy")
                        + (business.isEmpty() ? QString() : QString("   |   ") + business));

    // KPI cards
    const double rev = scalar("SELECT COALESCE(SUM(total_price),0) FROM sales WHERE date(sale_date) >= date('now','-29 days')");
    const double cnt = scalar("SELECT COUNT(*) FROM sales WHERE date(sale_date) >= date('now','-29 days')");
    m_revenue->setText(money(rev));
    m_sales->setText(loc.toString(qRound64(cnt)));
    m_pending->setText(loc.toString(qRound64(scalar("SELECT COUNT(*) FROM orders WHERE status = 'Pending'"))));
    m_attention->setText(loc.toString(qRound64(scalar("SELECT COUNT(*) FROM products WHERE quantity <= minimum_stock_level"))));
    m_promos->setText(loc.toString(qRound64(scalar(
        "SELECT COUNT(*) FROM promotions WHERE date('now') BETWEEN date(start_date) AND date(end_date)"))));
    m_stockValue->setText(money(scalar("SELECT COALESCE(SUM(quantity * price),0) FROM products")));
    for (QLabel *l : {m_revenue, m_sales, m_pending, m_attention, m_promos, m_stockValue})
        Fx::countUp(l, 800);

    // 14-day bars
    QMap<QString, double> perDay;
    QSqlQuery dq;
    if (dq.exec("SELECT date(sale_date), SUM(total_price) FROM sales "
                "WHERE date(sale_date) >= date('now','-13 days') GROUP BY date(sale_date)")) {
        while (dq.next())
            perDay.insert(dq.value(0).toString(), dq.value(1).toDouble());
    }
    QVector<double> values;
    QStringList labels, tips;
    double total14 = 0;
    for (int i = 13; i >= 0; --i) {
        const QDate day = QDate::currentDate().addDays(-i);
        const double v = perDay.value(day.toString("yyyy-MM-dd"), 0.0);
        values << v;
        labels << QString::number(day.day());
        tips << QString("%1: %2").arg(loc.toString(day, "ddd d MMM"), money(v));
        total14 += v;
    }
    m_bars->setData(values, labels, tips);
    m_barsTotal->setText(tr("Total %1").arg(money(total14)));

    // top products
    clear(m_topBody);
    // keep the two header labels (title + subtitle) which are the first two items
    // -> rebuilt below; simplest is to re-add them
    auto *t1 = new QLabel(tr("Top products"));
    t1->setStyleSheet("QLabel { font-weight: 600; }");
    auto *t2 = new QLabel(tr("Units sold in the last 30 days"));
    t2->setProperty("role", "muted");
    m_topBody->addWidget(t1);
    m_topBody->addWidget(t2);

    QSqlQuery tq;
    QVector<QPair<QString, qlonglong>> top;
    if (tq.exec("SELECT p.name, SUM(s.quantity_sold) AS q FROM sales s JOIN products p ON p.product_id = s.product_id "
                "WHERE date(s.sale_date) >= date('now','-29 days') GROUP BY p.product_id ORDER BY q DESC LIMIT 6")) {
        while (tq.next())
            top.append(qMakePair(tq.value(0).toString(), tq.value(1).toLongLong()));
    }
    if (top.isEmpty()) {
        auto *none = new QLabel(tr("No sales in the last 30 days."));
        none->setProperty("role", "muted");
        m_topBody->addWidget(none);
    }
    const qlonglong best = top.isEmpty() ? 1 : qMax<qlonglong>(1, top.first().second);
    for (const auto &row : top) {
        auto *w = new QWidget;
        auto *h = new QHBoxLayout(w);
        h->setContentsMargins(0, 2, 0, 2);
        h->setSpacing(10);
        auto *name = new QLabel(row.first);
        name->setMinimumWidth(130);
        name->setMaximumWidth(190);
        auto *bar = new QProgressBar;
        bar->setRange(0, 100);
        bar->setValue(int(100.0 * row.second / best));
        bar->setTextVisible(false);
        bar->setFixedHeight(6);
        auto *units = new QLabel(loc.toString(row.second));
        units->setMinimumWidth(36);
        units->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        h->addWidget(name);
        h->addWidget(bar, 1);
        h->addWidget(units);
        m_topBody->addWidget(w);
    }
    m_topBody->addStretch(1);

    // stock alerts
    clear(m_alertBody);
    auto *a1 = new QLabel(tr("Stock alerts"));
    a1->setStyleSheet("QLabel { font-weight: 600; }");
    auto *a2 = new QLabel(tr("Lowest stock first"));
    a2->setProperty("role", "muted");
    m_alertBody->addWidget(a1);
    m_alertBody->addWidget(a2);

    QSqlQuery aq;
    int alerts = 0;
    if (aq.exec("SELECT name, quantity, minimum_stock_level FROM products WHERE quantity <= minimum_stock_level "
                "ORDER BY quantity ASC, name LIMIT 7")) {
        while (aq.next()) {
            ++alerts;
            const int qty = aq.value(1).toInt();
            auto *w = new QWidget;
            auto *h = new QHBoxLayout(w);
            h->setContentsMargins(0, 3, 0, 3);
            h->setSpacing(10);
            auto *name = new QLabel(aq.value(0).toString());
            auto *lvl = new QLabel(QString("%1 / %2").arg(qty).arg(aq.value(2).toInt()));
            lvl->setProperty("role", "muted");
            auto *chip = new QLabel(qty <= 0 ? tr("Out") : tr("Low"));
            chip->setProperty("role", qty <= 0 ? "chip-bad" : "chip-warn");
            h->addWidget(name, 1);
            h->addWidget(lvl);
            h->addWidget(chip);
            m_alertBody->addWidget(w);
        }
    }
    if (alerts == 0) {
        auto *ok = new QLabel(tr("All products are above their minimum level."));
        ok->setProperty("role", "muted");
        m_alertBody->addWidget(ok);
    }
    auto *viewAll = new QPushButton(tr("Open Inventory Explorer"));
    viewAll->setProperty("role", "ghost");
    viewAll->setCursor(Qt::PointingHandCursor);
    connect(viewAll, &QPushButton::clicked, this, [this]() { emit actionRequested("inventory"); });
    m_alertBody->addWidget(viewAll, 0, Qt::AlignLeft);
    m_alertBody->addStretch(1);

    // recent activity
    clear(m_activityBody);
    auto *r1 = new QLabel(tr("Recent activity"));
    r1->setStyleSheet("QLabel { font-weight: 600; }");
    auto *r2 = new QLabel(tr("Latest audit-log entries"));
    r2->setProperty("role", "muted");
    m_activityBody->addWidget(r1);
    m_activityBody->addWidget(r2);

    QSqlQuery rq;
    int events = 0;
    if (rq.exec("SELECT timestamp, action, COALESCE(details,'') FROM audit_logs ORDER BY timestamp DESC LIMIT 8")) {
        while (rq.next()) {
            ++events;
            auto *w = new QWidget;
            auto *h = new QHBoxLayout(w);
            h->setContentsMargins(0, 3, 0, 3);
            h->setSpacing(10);
            auto *when = new QLabel(rq.value(0).toString().mid(5, 11));
            when->setProperty("role", "muted");
            when->setMinimumWidth(90);
            auto *what = new QLabel(rq.value(1).toString());
            what->setStyleSheet("QLabel { font-weight: 600; }");
            auto *det = new QLabel(rq.value(2).toString());
            det->setProperty("role", "muted");
            h->addWidget(when);
            h->addWidget(what);
            h->addWidget(det, 1);
            m_activityBody->addWidget(w);
        }
    }
    if (events == 0) {
        auto *none = new QLabel(tr("No activity recorded yet."));
        none->setProperty("role", "muted");
        m_activityBody->addWidget(none);
    }
    m_activityBody->addStretch(1);
}
