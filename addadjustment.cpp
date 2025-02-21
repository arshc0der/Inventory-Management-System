#include "addadjustment.h"
#include "ui_addadjustment.h"
#include <QDate>
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

AddAdjustment::AddAdjustment(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AddAdjustment)
{
    ui->setupUi(this);
    load_products();
}

AddAdjustment::~AddAdjustment()
{
    delete ui;
}
void AddAdjustment::load_products(){
    // Clear existing items in the combo box
    ui->product_comboBox->clear();

    // Query to fetch product IDs
    QSqlQuery query("SELECT product_id FROM products");

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to load product IDs: " + query.lastError().text());
        return;
    }

    // Populate the combo box with product IDs
    while (query.next()) {
        QString productID = query.value(0).toString();
        ui->product_comboBox->addItem(productID);
    }

    // Check if there are no products in the table
    if (ui->product_comboBox->count() == 0) {
        QMessageBox::warning(this, "No Products", "No products found in the database. Please add products first.");
    }
}

void AddAdjustment::on_Adjustments_btn_clicked()
{
    QString product_id = ui->product_comboBox->currentText();
    QString adjustment_type = ui->type_comboBox->currentText(); // 'Add' or 'Remove'
    double quantity = ui->qty_doubleSpinBox->value();
    QDate adjustment_date = ui->date_dateEdit->date();
    QString reason = ui->reason_lineEdit->text();

    // Validate inputs
    if (product_id.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please select a product.");
        return;
    }

    if (adjustment_type.isEmpty() || (adjustment_type != "Add" && adjustment_type != "Remove")) {
        QMessageBox::warning(this, "Validation Error", "Invalid adjustment type. Must be 'Add' or 'Remove'.");
        return;
    }

    if (quantity <= 0) {
        QMessageBox::warning(this, "Validation Error", "Quantity must be greater than zero.");
        return;
    }

    if (reason.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please provide a reason for the adjustment.");
        return;
    }

    // Start database insertion
    QSqlQuery query;

    // Insert into inventory_adjustments table
    query.prepare("INSERT INTO inventory_adjustments (product_id, adjustment_type, quantity, adjustment_date, reason) "
                  "VALUES (:product_id, :adjustment_type, :quantity, :adjustment_date, :reason)");
    query.bindValue(":product_id", product_id);
    query.bindValue(":adjustment_type", adjustment_type);
    query.bindValue(":quantity", quantity);
    query.bindValue(":adjustment_date", adjustment_date);
    query.bindValue(":reason", reason);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to insert adjustment: " + query.lastError().text());
        return;
    }

    // Update the product quantity in the products table
    QString updateQuery = (adjustment_type == "Add") ?
                              "UPDATE products SET quantity = quantity + :quantity WHERE product_id = :product_id" :
                              "UPDATE products SET quantity = quantity - :quantity WHERE product_id = :product_id";

    query.prepare(updateQuery);
    query.bindValue(":quantity", quantity);
    query.bindValue(":product_id", product_id);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to update product quantity: " + query.lastError().text());
        return;
    }

    // Success message
    QMessageBox::information(this, "Success", "Adjustment added successfully!");

    // Optionally, clear the input fields
    ui->qty_doubleSpinBox->setValue(0);
    ui->reason_lineEdit->clear();
}


