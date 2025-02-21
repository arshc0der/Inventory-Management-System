// checkdiscount.cpp
#include "checkdiscount.h"
#include "ui_checkdiscount.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QItemSelectionModel>
#include <QTableWidgetItem>

CheckDiscount::CheckDiscount(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CheckDiscount)
{
    ui->setupUi(this);

    // Prepare the SQL query to get current promotions
    QSqlQuery query;
    query.prepare("SELECT name, discount_percentage FROM promotions "
                  "WHERE '2024-12-26' BETWEEN start_date AND end_date;");

    // Execute the query
    if (query.exec()) {

        // Clear the table widget before adding new items
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);  // Ensure that rows are reset

        // Set the column headers (optional)
        ui->tableWidget->setColumnCount(2);
        ui->tableWidget->setHorizontalHeaderLabels({"Promotion Name", "Discount (%)"});

        // Iterate through the query results and add each promotion's name and discount
        int row = 0;  // Row counter for table
        while (query.next()) {
            QString promotionName = query.value(0).toString();  // Get promotion name
            QString discount = query.value(1).toString();  // Get discount percentage

            // Add the promotion name and discount to the table
            ui->tableWidget->insertRow(row);  // Insert a new row
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(promotionName));  // Column 0 - Promotion Name
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(discount));  // Column 1 - Discount (%)

            row++;  // Move to the next row
        }

        // Optionally, you can adjust the column widths for better readability
        ui->tableWidget->resizeColumnsToContents();
    } else {
        qDebug() << "Error executing query: " << query.lastError().text();
    }
}

CheckDiscount::~CheckDiscount()
{
    delete ui;
}

void CheckDiscount::on_apply_discount_btn_clicked()
{
    // Get the selected row index
    QItemSelectionModel *selectModel = ui->tableWidget->selectionModel();

    // Check if a row is selected
    if (selectModel->selectedRows().isEmpty()) {
        // If no row is selected, show an error message
        QMessageBox::warning(this, "No Row Selected", "Please select a promotion to apply the discount.");
        return;
    }

    // Get the index of the selected row
    int selectedRow = selectModel->selectedRows().first().row();

    // Get the discount value from the second column (Discount %)
    selectedDiscount = ui->tableWidget->item(selectedRow, 1)->text();

    // Optionally, you can close the dialog after emitting the signal
    close();
}

QString CheckDiscount::getSelectedDiscount() const
{
    // Return the selected discount value
    return selectedDiscount;
}
