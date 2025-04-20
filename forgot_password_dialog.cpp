#include "forgot_password_dialog.h"
#include "ui_forgot_password_dialog.h"
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QUuid>

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
    QString email = ui->user_email_lineEdit->text().trimmed();
    QString pin = ui->pin_lineEdit->text().trimmed();

    if (email.isEmpty() || pin.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter both email and PIN.");
        return;
    }

    if (userRole == "admin") {
        QSqlQuery query;
        query.prepare("SELECT id, is_active, account_locked FROM users WHERE email = :email AND pin = :pin");
        query.bindValue(":email", email);
        query.bindValue(":pin", pin);

        if (query.exec()) {
            QString user_id;
            if (query.next()) {
                user_id = query.value("id").toString();  // gettting id
                bool isActive = query.value("is_active").toBool();
                bool isLocked = query.value("account_locked").toBool();

                if (!isActive) {
                    QMessageBox::warning(this, "Inactive", "Your account has been deactivated.");
                    return;
                }

                if (isLocked) {
                    QMessageBox::warning(this, "Account Locked", "Your account is locked. Please wait or contact the system administrator.");
                    return;
                }

                // Generate reset token and expiry
                QString resetToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
                QDateTime expiry = QDateTime::currentDateTime().addSecs(900); // 15 mins from now

                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE users SET password_reset_token = :token, password_reset_expiry = :expiry WHERE email = :email");
                updateQuery.bindValue(":token", resetToken);
                updateQuery.bindValue(":expiry", expiry.toString(Qt::ISODate));
                updateQuery.bindValue(":email", email);

                if (updateQuery.exec()) {
                    qDebug() << "Reset token created for email:" << email << "Token:" << resetToken;
                    QMessageBox::information(this, "Verified", "Reset token generated. Proceed to password reset.");

                    // Proceed to open the Reset Password Dialog
                    Reset_Dialog = new Reset_Password_Dialog(userRole,user_id,resetToken,this);
                    //resetDialog->setUserEmail(email);  // implement setUserEmail() in ResetPasswordDialog
                    this->close();
                    Reset_Dialog->show();

                } else {
                    QMessageBox::critical(this, "Error", "Failed to generate reset token.");
                    qDebug() << "Token update error:" << updateQuery.lastError().text();
                }

            } else {
                QMessageBox::warning(this, "Invalid", "Email or PIN is incorrect.");
            }
        } else {
            QMessageBox::critical(this, "Query Error", "Failed to execute query.");
            qDebug() << "Query Error:" << query.lastError().text();
        }

    } else if (userRole == "employee") {
        QMessageBox::information(this, "Not Allowed", "Employees must request password reset through their administrator.");
    } else {
        QMessageBox::warning(this, "Unknown Role", "Unrecognized user role.");
    }
}


