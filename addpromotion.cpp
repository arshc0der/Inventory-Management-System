#include "addpromotion.h"
#include "ui_addpromotion.h"
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

AddPromotion::AddPromotion(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AddPromotion)
{
    ui->setupUi(this);
}

AddPromotion::~AddPromotion()
{
    delete ui;
}

void AddPromotion::on_select_product_id_btn_clicked()
{
    Products_Loader *productsLoader = new Products_Loader(this);
    productsLoader->exec();

    // After the dialog is closed, get the selected product IDs
    QStringList selectedProductIDs = productsLoader->getSelectedProductIDs();

    // Print the selected product IDs
    qDebug() << "Selected Product IDs:" << selectedProductIDs;

    // Join the selected product IDs into a single string, separated by commas
    QString selectedProductText = selectedProductIDs.join(",");

    // Set the joined string as the label's text
    ui->product_selected_label->setText(selectedProductText);

}


void AddPromotion::on_add_promotion_btn_clicked()
{
    // Step 1: Retrieve the values from the UI elements
    QString name = ui->discount_name_lineEdit->text();
    QString description = ui->discount_description_lineEdit->text();
    QDate start_date = ui->startDate_dateEdit->date();
    QDate end_date = ui->end_Date_dateEdit->date();
    double discount = ui->discount_doubleSpinBox->value();
    QString selected_products_id = ui->product_selected_label->text();  // Comma-separated product IDs

    // Step 2: Validate input fields
    if (name.isEmpty() || description.isEmpty() || selected_products_id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all required fields.");
        return;
    }

    // Step 3: Prepare the SQL query to insert the promotion
    QSqlQuery query;
    query.prepare("INSERT INTO promotions (name, description, start_date, end_date, discount_percentage, applicable_product_ids) "
                  "VALUES (:name, :description, :start_date, :end_date, :discount, :applicable_product_ids)");

    // Step 4: Bind the values to the query
    query.bindValue(":name", name);
    query.bindValue(":description", description);
    query.bindValue(":start_date", start_date.toString("yyyy-MM-dd"));  // Convert QDate to string (yyyy-MM-dd)
    query.bindValue(":end_date", end_date.toString("yyyy-MM-dd"));  // Convert QDate to string (yyyy-MM-dd)
    query.bindValue(":discount", discount);
    query.bindValue(":applicable_product_ids", selected_products_id);  // Comma-separated product IDs

    // Step 5: Execute the query
    if (!query.exec()) {
        // If the query execution fails, show an error message
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 6: Show success message
    QMessageBox::information(this, "Success", "Promotion added successfully!");

    // Step 7: Clear the input fields after successful insertion
    ui->discount_name_lineEdit->clear();
    ui->discount_description_lineEdit->clear();
    ui->startDate_dateEdit->clear();
    ui->end_Date_dateEdit->clear();
    ui->discount_doubleSpinBox->setValue(0.0);
    ui->product_selected_label->clear();  // Clear selected products label

    // Optionally, you can close the dialog after successful insertion
    // this->close();  // If you want to close the dialog after adding
}




