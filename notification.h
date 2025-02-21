#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QWidget>

namespace Ui {
class Notification;
}

class Notification : public QWidget
{
    Q_OBJECT

public:
    explicit Notification(const QString &title, const QString &message, const QString &noti_type,QWidget *parent = nullptr);
    ~Notification();

private slots:
    void on_dismiss_btn_clicked();

private:
    Ui::Notification *ui;
};

#endif // NOTIFICATION_H
