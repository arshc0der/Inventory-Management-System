#include "add_supplier.h"
#include "ui_add_supplier.h"
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

Add_Supplier::Add_Supplier(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Add_Supplier)
{
    ui->setupUi(this);
}

Add_Supplier::~Add_Supplier()
{
    delete ui;
}

void Add_Supplier::on_register_supplier_btn_clicked()
{
    int supplier_id = ui->id_mobile_no_spinBox->value();
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
        INSERT INTO suppliers (supplier_id, name, contact_info, address)
        VALUES (:supplier_id, :name, :contact_info, :address)
    )");

    // Bind values
    query.bindValue(":supplier_id", supplier_id);
    query.bindValue(":name", name);
    query.bindValue(":contact_info", email);
    query.bindValue(":address", address);

    // Execute the query
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to register supplier:\n" + query.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "supplier registered successfully.");
        // Optionally clear the form after successful insertion
        ui->id_mobile_no_spinBox->setValue(0);
        ui->name_lineEdit->clear();
        ui->conatct_email_lineEdit->clear();
        ui->address_lineEdit->clear();
    }
}

