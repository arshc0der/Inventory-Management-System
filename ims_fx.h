#ifndef IMS_FX_H
#define IMS_FX_H

class QWidget;
class QLabel;
class QPushButton;
class QStackedWidget;

/*
 * Fx - small, dependency-free animation helpers used across IMS.
 * Everything here is plain Qt (QPropertyAnimation / QVariantAnimation).
 */
namespace Fx {

// Fade a widget in (opacity 0 -> 1). The temporary effect is removed afterwards.
void fadeIn(QWidget *w, int ms = 240);

// Fade a top-level window in using windowOpacity.
void fadeInWindow(QWidget *w, int ms = 320);

// Fade + small upward slide whenever the stacked widget changes page,
// and count-up animation for the KPI numbers on the page being shown.
void installPageTransitions(QStackedWidget *sw);

// Animate "Total Products : 1,234" style labels from 0 to their value.
void countUp(QLabel *label, int ms = 900);
void countUpAll(QWidget *root, int ms = 900);

// Soft blue shadow that grows when the mouse enters the card.
void installHoverLift(QWidget *card);

// Swap a sidebar button to its blue icon on hover.
void installNavHover(QPushButton *button);

// Quick horizontal shake, e.g. for a rejected login.
void shake(QWidget *w);

} // namespace Fx

#endif // IMS_FX_H
