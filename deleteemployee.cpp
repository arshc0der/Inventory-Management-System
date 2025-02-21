#include "deleteemployee.h"
#include "ui_deleteemployee.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDate>
#include <QDebug>

DeleteEmployee::DeleteEmployee(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DeleteEmployee)
    , tableView(new QTableView(this)),
    model(new QSqlTableModel(this))
{
    ui->setupUi(this);
    // Set up the SQL Table Model
    model->setTable("employees");  // Specify the table to display data from
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

DeleteEmployee::~DeleteEmployee()
{
    delete ui;
}

void DeleteEmployee::on_Delete_btn_clicked()
{
    // Get the selected index from the table view
    QModelIndexList selectedIndexes = tableView->selectionModel()->selectedRows();

    // Check if at least one row is selected
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select an employee to delete.");
        return;
    }

    // Loop through selected rows (in case of multiple row selection)
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();  // Get the row index of the selected item

        // Get the employee ID (assuming employee_id is the first column)
        int employeeId = model->data(model->index(row, 0)).toInt();  // Adjust column index as necessary

        // Start a transaction to ensure both delete actions (employee and permissions) succeed or fail together
        QSqlDatabase::database().transaction();

        // Remove the selected employee record from the model
        if (!model->removeRow(row)) {
            QMessageBox::critical(this, "Error", "Failed to delete the employee.");
            return;
        }

        // Remove the associated permissions from the permissions table using employee_id
        QSqlQuery deletePermissionsQuery;
        deletePermissionsQuery.prepare("DELETE FROM permissions WHERE user_id = :user_id");
        deletePermissionsQuery.bindValue(":user_id", employeeId);  // Use employeeId as the user_id in the permissions table

        if (!deletePermissionsQuery.exec()) {
            QMessageBox::critical(this, "Error", "Failed to delete associated permissions.");
            QSqlDatabase::database().rollback();  // Rollback the transaction if deleting permissions fails
            return;
        }

        // Commit the transaction after both delete operations are successful
        if (model->submitAll()) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, "Success", "Employee and associated permissions deleted successfully.");
            refreshTable();  // Refresh the table view after deletion
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete employee(s).");
            QSqlDatabase::database().rollback();  // Rollback if model submit fails
        }
    }
}

void DeleteEmployee::refreshTable(){
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
