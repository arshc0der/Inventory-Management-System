#include "reset_password_dialog.h"
#include "ui_reset_password_dialog.h"
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

//Hashing
#include <QCryptographicHash>
#include <QUuid>

Reset_Password_Dialog::Reset_Password_Dialog(const QString role,const QString id,const QString token,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Reset_Password_Dialog)
    , userRole(role)
    , userID(id)
    , resetToken(token)
{
    ui->setupUi(this);

    qDebug() << "role : "+role;
    qDebug() << "id : "+id;
    qDebug() << "Token " +resetToken;

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

void Reset_Password_Dialog::on_reset_btn_clicked()
{
    QString newPassword = ui->New_Password_lineEdit->text();
    QString confirmPassword = ui->Confirm_Password_lineEdit->text();

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "Mismatch", "Passwords do not match.");
        return;
    }

    // 🔐 Step 1: Validate token before password update
    QSqlQuery validateQuery;
    if (userRole == "admin") {
        validateQuery.prepare("SELECT password_reset_expiry FROM users WHERE id = :id AND password_reset_token = :token");
    } else if (userRole == "employee") {
        //validateQuery.prepare("SELECT password_reset_expiry FROM employees WHERE id = :id AND password_reset_token = :token");
    } else {
        QMessageBox::critical(this, "Error", "Unknown user role.");
        return;
    }

    validateQuery.bindValue(":id", userID);
    validateQuery.bindValue(":token", resetToken);

    if (userRole == "admin"){
        if (!validateQuery.exec() || !validateQuery.next()) {
            QMessageBox::critical(this, "Invalid", "Invalid or expired reset token.");
            return;
        }

        QDateTime expiry = QDateTime::fromString(validateQuery.value(0).toString(), Qt::ISODate);
        if (QDateTime::currentDateTime() > expiry) {
            QMessageBox::warning(this, "Expired", "Reset token has expired.");
            return;
        }
    }


    // 🔐 Step 2: Generate salt and hash
    QString password_salt = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    QByteArray saltedPassword = (newPassword + password_salt).toUtf8();
    QString password_hash = QString(QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256).toHex());

    // 🔐 Step 3: Update password
    QSqlQuery query;
    if (userRole == "admin") {
        query.prepare("UPDATE users SET password_hash = :hash, password_salt = :salt, password_reset_token = NULL, password_reset_expiry = NULL WHERE id = :id");
    } else if (userRole == "employee") {
        query.prepare(R"(
    UPDATE employees
    SET password_hash = :hash,
        password_salt = :salt,
        password_reset_token = NULL,
        password_reset_expiry = NULL,
        force_password_change = 0
    WHERE employee_id = :id
)");
    }

    query.bindValue(":hash", password_hash);
    query.bindValue(":salt", password_salt);
    query.bindValue(":id", userID);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Password updated successfully.");
        this->close();
    } else {
        QMessageBox::critical(this, "Error", "Failed to update password: " + query.lastError().text());
    }
}



