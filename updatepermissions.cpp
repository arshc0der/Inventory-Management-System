#include "updatepermissions.h"
#include "ui_updatepermissions.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDate>
#include <QDebug>

UpdatePermissions::UpdatePermissions(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UpdatePermissions)
    , tableView(new QTableView(this)),
    model(new QSqlTableModel(this))
{
    ui->setupUi(this);
    // Set up the QTableView
    model->setTable("permissions");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit); // Manual commit
    model->select(); // Load data lazily

    tableView->setModel(model);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->table_horizontalLayout->addWidget(tableView);

    // Adjust column sizes based on the content
    tableView->resizeColumnsToContents();

    // Set a minimum width for each column to ensure the table looks good
    for (int column = 0; column < model->columnCount(); ++column) {
        // Set a minimum width for the column, you can adjust this value as needed
        tableView->setColumnWidth(column, qMax(tableView->columnWidth(column), 100));  // Minimum width is 100px
    }
}

UpdatePermissions::~UpdatePermissions()
{
    delete ui;
}

void UpdatePermissions::on_Update_btn_clicked()
{
    // Commit the changes made in the table back to the database
    if (model->submitAll()) {
        QMessageBox::information(this, "Success", "Product(s) updated successfully.");
        refreshTable();  // Refresh the table view to reflect the changes
    } else {
        QMessageBox::critical(this, "Error", "Failed to update product(s).");
    }
}

void UpdatePermissions::refreshTable(){
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
