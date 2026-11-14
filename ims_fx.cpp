#include "ims_fx.h"
#include "ims_theme.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <QEvent>
#include <QTimer>
#include <QLocale>
#include <QPointer>
#include <QRegularExpression>
#include <QIcon>
#include <QVariant>
#include <QPoint>
#include <QColor>

namespace {

// ---------------------------------------------------------------------------
// Hover lift for cards
// ---------------------------------------------------------------------------
class HoverLift : public QObject
{
public:
    explicit HoverLift(QWidget *w) : QObject(w), m_w(w) { w->installEventFilter(this); }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (o == m_w) {
            if (e->type() == QEvent::Enter)
                animate(true);
            else if (e->type() == QEvent::Leave)
                animate(false);
        }
        return false;
    }

private:
    void animate(bool up)
    {
    if (!ThemeManager::instance().animationsEnabled())
        return;
        if (!m_fx || m_w->graphicsEffect() != m_fx) {
            m_fx = new QGraphicsDropShadowEffect(m_w);
            m_fx->setBlurRadius(0);
            m_fx->setOffset(0, 0);
            m_fx->setColor(QColor(15, 98, 254, 70));
            m_w->setGraphicsEffect(m_fx);
        }
        auto *blur = new QPropertyAnimation(m_fx, "blurRadius", this);
        blur->setDuration(180);
        blur->setEndValue(up ? 26.0 : 0.0);
        blur->setEasingCurve(QEasingCurve::OutCubic);
        blur->start(QAbstractAnimation::DeleteWhenStopped);

        auto *lift = new QPropertyAnimation(m_fx, "yOffset", this);
        lift->setDuration(180);
        lift->setEndValue(up ? 6.0 : 0.0);
        lift->setEasingCurve(QEasingCurve::OutCubic);
        lift->start(QAbstractAnimation::DeleteWhenStopped);
    }

    QWidget *m_w;
    QPointer<QGraphicsDropShadowEffect> m_fx;
};

// ---------------------------------------------------------------------------
// Sidebar icon swap on hover
// ---------------------------------------------------------------------------
class NavHover : public QObject
{
public:
    explicit NavHover(QPushButton *b) : QObject(b), m_b(b) { b->installEventFilter(this); }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (o != m_b)
            return false;
        if (e->type() == QEvent::Enter) {
            const QIcon hover = m_b->property("ims_icon_active").value<QIcon>();
            if (!hover.isNull())
                m_b->setIcon(hover);
        } else if (e->type() == QEvent::Leave) {
            if (!m_b->property("active").toBool()) {
                const QIcon base = m_b->property("ims_icon_base").value<QIcon>();
                if (!base.isNull())
                    m_b->setIcon(base);
            }
        }
        return false;
    }

private:
    QPushButton *m_b;
};

void pageTransition(QStackedWidget *sw, QWidget *page)
{
    if (!ThemeManager::instance().animationsEnabled())
        return;
    Fx::fadeIn(page, 260);

    // Slide up slightly. Deferred so the stacked layout has placed the page.
    QTimer::singleShot(0, page, [sw, page]() {
        const QPoint home = sw->contentsRect().topLeft();
        auto *a = new QPropertyAnimation(page, "pos", page);
        a->setDuration(300);
        a->setStartValue(home + QPoint(0, 14));
        a->setEndValue(home);
        a->setEasingCurve(QEasingCurve::OutCubic);
        a->start(QAbstractAnimation::DeleteWhenStopped);
    });

    Fx::countUpAll(page);
}

} // namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
namespace Fx {

void fadeIn(QWidget *w, int ms)
{
    if (!ThemeManager::instance().animationsEnabled())
        return;
    if (!w)
        return;
    auto *eff = new QGraphicsOpacityEffect(w);
    eff->setOpacity(0.0);
    w->setGraphicsEffect(eff);

    auto *a = new QPropertyAnimation(eff, "opacity", w);
    a->setDuration(ms);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(a, &QPropertyAnimation::finished, w, [w, eff]() {
        if (w->graphicsEffect() == eff)
            w->setGraphicsEffect(nullptr);
    });
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void fadeInWindow(QWidget *w, int ms)
{
    if (!ThemeManager::instance().animationsEnabled())
        return;
    if (!w)
        return;
    w->setWindowOpacity(0.0);
    auto *a = new QPropertyAnimation(w, "windowOpacity", w);
    a->setDuration(ms);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void installPageTransitions(QStackedWidget *sw)
{
    if (!sw)
        return;
    QObject::connect(sw, &QStackedWidget::currentChanged, sw, [sw](int index) {
        if (QWidget *page = sw->widget(index))
            pageTransition(sw, page);
    });
}

void countUp(QLabel *label, int ms)
{
    if (!ThemeManager::instance().animationsEnabled())
        return;
    if (!label || label->property("ims_cu_running").toBool())
        return;

    static const QRegularExpression rx("^(.*?)(-?\\d[\\d,]*)(\\.\\d+)?(.*)$");
    static const QRegularExpression digit("\\d");

    const QString full = label->text();
    const QRegularExpressionMatch m = rx.match(full);
    if (!m.hasMatch())
        return;

    const QString prefix = m.captured(1);
    const QString suffix = m.captured(4);
    if (digit.match(prefix).hasMatch() || digit.match(suffix).hasMatch())
        return; // several numbers (dates, ranges): leave untouched

    QString digits = m.captured(2);
    digits.remove(',');
    const QString dec = m.captured(3);
    bool ok = false;
    const double target = (digits + dec).toDouble(&ok);
    if (!ok || qFuzzyIsNull(target))
        return;
    const int decimals = dec.isEmpty() ? 0 : dec.size() - 1;

    label->setProperty("ims_cu_running", true);
    auto *a = new QVariantAnimation(label);
    a->setDuration(ms);
    a->setStartValue(0.0);
    a->setEndValue(target);
    a->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(a, &QVariantAnimation::valueChanged, label,
                     [label, prefix, suffix, decimals](const QVariant &v) {
        label->setText(prefix + QLocale().toString(v.toDouble(), 'f', decimals) + suffix);
    });
    QObject::connect(a, &QVariantAnimation::finished, label, [label, full]() {
        label->setText(full);
        label->setProperty("ims_cu_running", false);
    });
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void countUpAll(QWidget *root, int ms)
{
    if (!root)
        return;
    const QList<QLabel *> labels = root->findChildren<QLabel *>();
    for (QLabel *l : labels) {
        if (l->property("role").toString() == "kpi")
            countUp(l, ms);
    }
}

void installHoverLift(QWidget *card)
{
    if (card)
        new HoverLift(card);
}

void installNavHover(QPushButton *button)
{
    if (button)
        new NavHover(button);
}

void shake(QWidget *w)
{
    if (!ThemeManager::instance().animationsEnabled())
        return;
    if (!w)
        return;
    const QPoint p = w->pos();
    auto *a = new QPropertyAnimation(w, "pos", w);
    a->setDuration(380);
    a->setKeyValueAt(0.00, p);
    a->setKeyValueAt(0.15, p + QPoint(-8, 0));
    a->setKeyValueAt(0.30, p + QPoint(8, 0));
    a->setKeyValueAt(0.50, p + QPoint(-6, 0));
    a->setKeyValueAt(0.70, p + QPoint(4, 0));
    a->setKeyValueAt(0.85, p + QPoint(-2, 0));
    a->setKeyValueAt(1.00, p);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace Fx
