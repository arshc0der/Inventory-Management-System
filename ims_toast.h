#ifndef IMS_TOAST_H
#define IMS_TOAST_H

#include <QFrame>
#include <QString>

/*
 * Toast - non-blocking, auto-dismissing notification in the bottom-right
 * corner of the window. Use it instead of a modal QMessageBox for
 * confirmations that do not need an answer.
 *
 *     Toast::notify(this, "Product saved", Toast::Success);
 */
class Toast : public QFrame
{
public:
    enum Type { Info, Success, Warning, Error };

    static void notify(QWidget *anchor, const QString &message,
                       Type type = Info, int durationMs = 3200);

private:
    Toast(QWidget *window, const QString &message, Type type);
    void dismiss();
    static void relayout(QWidget *window, Toast *fresh);
};

#endif // IMS_TOAST_H
