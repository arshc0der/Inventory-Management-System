#include "registercustomer.h"
#include "ui_registercustomer.h"
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

RegisterCustomer::RegisterCustomer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterCustomer)
{
    ui->setupUi(this);
}

RegisterCustomer::~RegisterCustomer()
{
    delete ui;
}

void RegisterCustomer::on_register_customer_btn_clicked()
{
    int customer_id = ui->id_mobile_no_spinBox->value();
    QString name = ui->name_lineEdit->text();
    QString email = ui->conatct_email_lineEdit->text();
    QString address = ui->address_lineEdit->text();

    // Input validation
    if (name.isEmpty() || email.isEmpty() || address.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields must be filled out.");
        return;
    }

    // Prepare SQL query
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO customers (customer_id, name, contact_info, address)
        VALUES (:customer_id, :name, :contact_info, :address)
    )");

    // Bind values
    query.bindValue(":customer_id", customer_id);
    query.bindValue(":name", name);
    query.bindValue(":contact_info", email);
    query.bindValue(":address", address);

    // Execute the query
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to register customer:\n" + query.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "Customer registered successfully.");
        // Optionally clear the form after successful insertion
        ui->id_mobile_no_spinBox->setValue(0);
        ui->name_lineEdit->clear();
        ui->conatct_email_lineEdit->clear();
        ui->address_lineEdit->clear();
    }
}
