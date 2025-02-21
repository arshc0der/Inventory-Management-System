#include "deleteorders.h"
#include "ui_deleteorders.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDate>
#include <QDebug>

DeleteOrders::DeleteOrders(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DeleteOrders)
    , tableView(new QTableView(this)),
    model(new QSqlTableModel(this))
{
    ui->setupUi(this);
    // Set up the SQL Table Model
    model->setTable("orders");  // Specify the table to display data from
    model->select();  // Load data lazily from the database

    // Set up the QTableView
    tableView->setModel(model);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);  // Allow single row selection
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // Select entire rows
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Disable cell editing

    // Add the table view to the layout
    ui->table_horizontalLayout->addWidget(tableView);

    // Adjust column sizes based on the content
    tableView->resizeColumnsToContents();

    // Set a minimum width for each column to ensure the table looks good
    for (int column = 0; column < model->columnCount(); ++column) {
        // Set a minimum width for the column, you can adjust this value as needed
        tableView->setColumnWidth(column, qMax(tableView->columnWidth(column), 100));  // Minimum width is 100px
    }
}

DeleteOrders::~DeleteOrders()
{
    delete ui;
}

void DeleteOrders::on_Delete_orders_btn_clicked()
{
    // Get the selected index from the table view
    QModelIndexList selectedIndexes = tableView->selectionModel()->selectedRows();

    // Check if at least one row is selected
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a product to delete.");
        return;
    }

    // Loop through selected rows (in case of multiple row selection)
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();  // Get the row index of the selected item

        // Remove the selected row from the model
        if (!model->removeRow(row)) {
            QMessageBox::critical(this, "Error", "Failed to delete the product.");
            return;
        }
    }

    // Submit changes to the database
    if (model->submitAll()) {
        QMessageBox::information(this, "Success", "Product(s) deleted successfully.");
        refreshTable();  // Refresh the table view after deletion
    } else {
        QMessageBox::critical(this, "Error", "Failed to delete product(s).");
    }
}

void DeleteOrders::refreshTable(){
    model->select();  // Reload data from the database into the model
    tableView->resizeColumnsToContents();  // Adjust column sizes based on content

    // Adjust column sizes based on the content
    tableView->resizeColumnsToContents();

    // Set a minimum width for each column to ensure the table looks good
    for (int column = 0; column < model->columnCount(); ++column) {
        // Set a minimum width for the column, you can adjust this value as needed
        tableView->setColumnWidth(column, qMax(tableView->columnWidth(column), 100));  // Minimum width is 100px
    }
}
