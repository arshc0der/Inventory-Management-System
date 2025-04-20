#include "reset_password_dialog.h"
#include "ui_reset_password_dialog.h"

Reset_Password_Dialog::Reset_Password_Dialog(const QString role,const QString id,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Reset_Password_Dialog)
    , userRole(role)
    , userID(id)
{
    ui->setupUi(this);

    qDebug() << "role : "+role;
    qDebug() << "id : "+id;

    if (userRole == "admin") {
        // Setup admin reset UI
        ui->status_label->setText("Admin");
    } else if (userRole == "employee") {
        // Setup employee reset UI
        ui->status_label->setText("Employee");
    }

}

Reset_Password_Dialog::~Reset_Password_Dialog()
{
    delete ui;
}
