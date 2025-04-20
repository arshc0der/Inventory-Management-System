#ifndef FORGOT_PASSWORD_DIALOG_H
#define FORGOT_PASSWORD_DIALOG_H

#include <QDialog>

namespace Ui {
class Forgot_Password_Dialog;
}

class Forgot_Password_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Forgot_Password_Dialog(const QString role,QWidget *parent = nullptr);
    ~Forgot_Password_Dialog();

private:
    Ui::Forgot_Password_Dialog *ui;
    QString userRole; // Store the role internally
};

#endif // FORGOT_PASSWORD_DIALOG_H
