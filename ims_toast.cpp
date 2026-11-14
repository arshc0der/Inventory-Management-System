#include "ims_toast.h"
#include "ims_theme.h"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QPointer>
#include <QList>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

namespace {
QList<QPointer<Toast>> &activeToasts()
{
    static QList<QPointer<Toast>> list;
    return list;
}
} // namespace

Toast::Toast(QWidget *window, const QString &message, Type type)
    : QFrame(window)
{
    setObjectName("imsToast");
    setAttribute(Qt::WA_StyledBackground, true);

    const ThemeManager &tm = ThemeManager::instance();
    QString accent, glyph;
    switch (type) {
    case Success: accent = "#42BE65"; glyph = QString::fromUtf8("\u2713"); break;
    case Warning: accent = "#F1C21B"; glyph = "!";                           break;
    case Error:   accent = "#FA4D56"; glyph = QString::fromUtf8("\u2715"); break;
    default:      accent = "#4589FF"; glyph = "i";                           break;
    }

    setStyleSheet(QString("#imsToast { background: %1; border: none; border-left: 4px solid %2; border-radius: 3px; }"
                          "#imsToast QLabel { background: transparent; color: %3; }")
                      .arg(tm.color("tipBg").name(), accent, tm.color("tipText").name()));

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(14, 12, 16, 12);
    lay->setSpacing(12);

    auto *icon = new QLabel(glyph, this);
    icon->setStyleSheet(QString("QLabel { color: %1; font-weight: 700; font-size: 15px; }").arg(accent));
    icon->setFixedWidth(16);
    lay->addWidget(icon, 0, Qt::AlignTop);

    auto *text = new QLabel(message, this);
    text->setWordWrap(true);
    lay->addWidget(text, 1);
}

void Toast::notify(QWidget *anchor, const QString &message, Type type, int durationMs)
{
    if (!anchor)
        return;
    QWidget *win = anchor->window();

    auto *t = new Toast(win, message, type);
    activeToasts().append(t);

    const int w = 340;
    t->layout()->activate();
    const int h = t->layout()->hasHeightForWidth() ? t->layout()->totalHeightForWidth(w)
                                                  : t->layout()->sizeHint().height();
    t->resize(w, qMax(h, 44));
    t->show();
    t->raise();

    relayout(win, t);

    QTimer::singleShot(durationMs, t, [t]() { t->dismiss(); });
}

void Toast::relayout(QWidget *window, Toast *fresh)
{
    int y = window->height() - 24;
    QList<QPointer<Toast>> &list = activeToasts();

    for (int i = list.size() - 1; i >= 0; --i) {
        Toast *t = list.at(i);
        if (!t || t->parentWidget() != window)
            continue;
        y -= t->height();
        const QPoint target(window->width() - t->width() - 24, y);

        if (t == fresh && ThemeManager::instance().animationsEnabled()) {
            t->move(target + QPoint(40, 0));
            auto *eff = new QGraphicsOpacityEffect(t);
            eff->setOpacity(0.0);
            t->setGraphicsEffect(eff);

            auto *slide = new QPropertyAnimation(t, "pos", t);
            slide->setDuration(260);
            slide->setStartValue(target + QPoint(40, 0));
            slide->setEndValue(target);
            slide->setEasingCurve(QEasingCurve::OutCubic);
            slide->start(QAbstractAnimation::DeleteWhenStopped);

            auto *fade = new QPropertyAnimation(eff, "opacity", t);
            fade->setDuration(260);
            fade->setStartValue(0.0);
            fade->setEndValue(1.0);
            QObject::connect(fade, &QPropertyAnimation::finished, t, [t, eff]() {
                if (t->graphicsEffect() == eff)
                    t->setGraphicsEffect(nullptr);
            });
            fade->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            t->move(target);
        }
        y -= 10;
    }
}

void Toast::dismiss()
{
    QWidget *win = parentWidget();

    if (!ThemeManager::instance().animationsEnabled()) {
        activeToasts().removeAll(QPointer<Toast>(this));
        hide();
        deleteLater();
        if (win)
            relayout(win, nullptr);
        return;
    }

    auto *eff = new QGraphicsOpacityEffect(this);
    eff->setOpacity(1.0);
    setGraphicsEffect(eff);

    auto *fade = new QPropertyAnimation(eff, "opacity", this);
    fade->setDuration(220);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);
    QObject::connect(fade, &QPropertyAnimation::finished, this, [this, win]() {
        activeToasts().removeAll(QPointer<Toast>(this));
        hide();
        deleteLater();
        if (win)
            relayout(win, nullptr);
    });
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}
