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
        ui->status_label->setText("Admin");
    } else if (userRole == "employee") {
        // Setup employee reset UI
        ui->status_label->setText("Employee");
    }
}

Forgot_Password_Dialog::~Forgot_Password_Dialog()
{
    delete ui;
}

void Forgot_Password_Dialog::on_submit_btn_clicked()
{
    if (userRole == "admin") {
        // Setup admin reset UI
        ui->status_label->setText("Admin");
    } else if (userRole == "employee") {
        // Setup employee reset UI
        ui->status_label->setText("Employee");
    }
}

