#include "forgot_password_dialog.h"
#include "ui_forgot_password_dialog.h"

Forgot_Password_Dialog::Forgot_Password_Dialog(const QString role, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Forgot_Password_Dialog)
    , userRole(role)
{
    ui->setupUi(this);

    if (userRole == "admin") {
        // Setup admin reset UI
    } else if (userRole == "employee") {
        // Setup employee reset UI
    }
}

Forgot_Password_Dialog::~Forgot_Password_Dialog()
{
    delete ui;
}
