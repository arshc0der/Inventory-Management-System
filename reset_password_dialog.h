#ifndef RESET_PASSWORD_DIALOG_H
#define RESET_PASSWORD_DIALOG_H

#include <QDialog>

namespace Ui {
class Reset_Password_Dialog;
}

class Reset_Password_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Reset_Password_Dialog(const QString role,const QString id,const  QString token,QWidget *parent = nullptr);
    ~Reset_Password_Dialog();

private slots:
    void on_reset_btn_clicked();

private:
    Ui::Reset_Password_Dialog *ui;
    QString userRole; // Store the role internally
    QString userID;
    QString resetToken;
};

#endif // RESET_PASSWORD_DIALOG_H
