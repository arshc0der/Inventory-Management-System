#include "manageactivity.h"
#include "ui_manageactivity.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDate>
#include <QDebug>

ManageActivity::ManageActivity(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ManageActivity)
    , tableView(new QTableView(this))
    , model(new QSqlTableModel(this))
{
    ui->setupUi(this);

    // Set up the SQL Table Model
    model->setTable("audit_logs");  // Specify the table to display data from
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

ManageActivity::~ManageActivity()
{
    delete ui;  // Clean up the user interface
}

void ManageActivity::on_del_btn_clicked()
{
    // Get the list of selected rows from the table view
    QModelIndexList selectedIndexes = tableView->selectionModel()->selectedRows();

    // Check if at least one row is selected
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a record to delete.");
        return;
    }

    // Loop through selected rows and remove each selected one
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();  // Get the row index of the selected item

        // Remove the selected row from the model
        if (!model->removeRow(row)) {
            QMessageBox::critical(this, "Error", "Failed to delete the record.");
            return;
        }
    }

    // Submit changes to the database
    if (model->submitAll()) {
        QMessageBox::information(this, "Success", "Record(s) deleted successfully.");
        refreshTable();  // Refresh the table view after deletion
    } else {
        QMessageBox::critical(this, "Error", "Failed to delete record(s).");
    }
}

void ManageActivity::refreshTable()
{
    // Reload data from the database into the model
    model->select();

    // Adjust column sizes based on the content
    tableView->resizeColumnsToContents();

    // Set a minimum width for each column to ensure the table looks good
    for (int column = 0; column < model->columnCount(); ++column) {
        // Set a minimum width for the column, you can adjust this value as needed
        tableView->setColumnWidth(column, qMax(tableView->columnWidth(column), 100));  // Minimum width is 100px
    }
}

void ManageActivity::on_delete_all_btn_clicked()
{
    // Show a Yes/No confirmation dialog
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Deletion", "Are you sure you want to delete all records?",
                                  QMessageBox::Yes | QMessageBox::No);

    // If the user clicked Yes, proceed with deletion
    if (reply == QMessageBox::Yes) {
        // Start a transaction for safe deletion
        QSqlDatabase::database().transaction();
        qDebug() << "Transaction started";

        // Create a QSqlQuery to execute the SQL DELETE statement
        QSqlQuery query;

        // Directly execute the DELETE query on the table
        query.prepare("DELETE FROM audit_logs");  // Replace with your table name

        if (!query.exec()) {
            // If the query fails, rollback the transaction and show an error
            qDebug() << "Error during deletion: " << query.lastError().text();
            QMessageBox::critical(this, "Error", "Failed to delete all records.\n" + query.lastError().text());
            QSqlDatabase::database().rollback();
            return;
        }

        // Commit the transaction if the query was successful
        if (QSqlDatabase::database().commit()) {
            qDebug() << "Deletion successful";
            refreshTable();  // Refresh the table view after deletion
            QMessageBox::information(this, "Success", "All records have been deleted successfully.");
        } else {
            qDebug() << "Transaction commit failed: " << QSqlDatabase::database().lastError().text();
            QMessageBox::critical(this, "Error", "Failed to commit the deletion.\n" + QSqlDatabase::database().lastError().text());
            QSqlDatabase::database().rollback();  // Rollback transaction if commit fails
        }
    } else {
        // If the user clicked No, do nothing and exit the function
        qDebug() << "User canceled the deletion.";
        return;
    }
}


