#ifndef REGISTER_H
#define REGISTER_H

#include <QMainWindow>

namespace Ui {
class Register;
}

class Register : public QMainWindow
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

private slots:
    void on_show_pass_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_confirmPassword_lineEdit_textChanged(const QString &arg1);

    void on_email_lineEdit_textChanged(const QString &arg1);

    void on_register_btn_clicked();

    void on_BusinessName_lineEdit_textChanged(const QString &arg1);

    void on_forgotPassword_pin_lineEdit_textChanged(const QString &arg1);

    void on_username_lineEdit_textChanged(const QString &arg1);

    void on_role_lineEdit_textChanged(const QString &arg1);

    void on_already_signin_btn_clicked();

    void on_select_btn_clicked();

private:
    Ui::Register *ui;
};

#endif // REGISTER_H
