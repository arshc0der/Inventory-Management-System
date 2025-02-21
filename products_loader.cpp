#include "products_loader.h"
#include "ui_products_loader.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCheckBox>

Products_Loader::Products_Loader(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Products_Loader)
{
    ui->setupUi(this);

    // Prepare the SQL query to get product details
    QSqlQuery query;
    query.prepare("SELECT product_id, name FROM products;");

    // Execute the query
    if (query.exec()) {
        // Iterate over the query results
        while (query.next()) {
            // Retrieve the product ID and name
            QString productId = query.value("product_id").toString();
            QString productName = query.value("name").toString();

            // Store the product details in separate arrays
            productIDs.append(productId);
            productNames.append(productName);
        }

        // Debug: Print all products in the arrays
        qDebug() << "Loaded Product IDs:" << productIDs;
        qDebug() << "Loaded Product Names:" << productNames;

        // Add checkboxes for each product
        for (int i = 0; i < productIDs.length(); ++i) {
            QCheckBox *checkBox = new QCheckBox(productIDs[i] + " - " + productNames[i], this);
            checkBoxes.append(checkBox);
            ui->scrollArea_vertical_layout->addWidget(checkBox); // Ensure you have a vertical layout named `verticalLayout` in your UI file
        }

    } else {
        // Show an error message if the query fails
        qDebug() << "Error executing query: " << query.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to load products: " + query.lastError().text());
    }
}

Products_Loader::~Products_Loader()
{
    delete ui;
}


void Products_Loader::on_applyFilter_linedit_textChanged(const QString &arg1)
{
    for (int i = 0; i < checkBoxes.size(); ++i) {
        QCheckBox *checkBox = checkBoxes[i];
        QString productDetails = productIDs[i] + " - " + productNames[i];

        // Show or hide the checkbox based on the filter
        if (productDetails.contains(arg1, Qt::CaseInsensitive)) {
            checkBox->show();
        } else {
            checkBox->hide();
        }
    }
}


void Products_Loader::on_add_btn_clicked()
{
    // Array to hold the selected product IDs
    QStringList selectedProductIDs;

    // Iterate through the checkboxes to find the checked ones
    for (int i = 0; i < checkBoxes.size(); ++i) {
        QCheckBox *checkBox = checkBoxes[i];

        // If the checkbox is checked, add the product's ID to the array
        if (checkBox->isChecked()) {
            selectedProductIDs.append(productIDs[i]);
        }
    }

    // Return the selected product IDs directly (simplified)
    accept();  // Close the dialog after selection is made

    // Return the selected IDs to the main window (via the constructor or other method)
    QDialog::done(0);  // Close dialog after "Add" is clicked
}

QStringList Products_Loader::getSelectedProductIDs()
{
    // Return the selected product IDs list
    QStringList selectedProductIDs;

    for (int i = 0; i < checkBoxes.size(); ++i) {
        QCheckBox *checkBox = checkBoxes[i];

        if (checkBox->isChecked()) {
            selectedProductIDs.append(productIDs[i]);
        }
    }

    return selectedProductIDs;
}
