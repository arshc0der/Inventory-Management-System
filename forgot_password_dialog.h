#ifndef FORGOT_PASSWORD_DIALOG_H
#define FORGOT_PASSWORD_DIALOG_H

#include <QDialog>
#include <reset_password_dialog.h>

namespace Ui {
class Forgot_Password_Dialog;
}

class Forgot_Password_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Forgot_Password_Dialog(const QString role,QWidget *parent = nullptr);
    ~Forgot_Password_Dialog();

private slots:
    void on_submit_btn_clicked();

private:
    Ui::Forgot_Password_Dialog *ui;
    QString userRole; // Store the role internally
    Reset_Password_Dialog *Reset_Dialog;
};

#endif // FORGOT_PASSWORD_DIALOG_H
