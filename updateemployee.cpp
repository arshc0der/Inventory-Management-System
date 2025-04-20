#include "updateemployee.h"
#include "ui_updateemployee.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDate>
#include <QDebug>

//hashing
#include <QUuid>
#include <QCryptographicHash>

UpdateEmployee::UpdateEmployee(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UpdateEmployee)
    , tableView(new QTableView(this)),
    model(new QSqlTableModel(this))
{
    ui->setupUi(this);
    // Set up the QTableView
    model->setTable("employees");
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

UpdateEmployee::~UpdateEmployee()
{
    delete ui;
}

void UpdateEmployee::on_Update_btn_clicked()
{
    // Commit the changes made in the table back to the database
    if (model->submitAll()) {
        QMessageBox::information(this, "Success", "Product(s) updated successfully.");
        refreshTable();  // Refresh the table view to reflect the changes
    } else {
        QMessageBox::critical(this, "Error", "Failed to update product(s).");
    }
}

void UpdateEmployee::refreshTable(){
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

void UpdateEmployee::on_reset_btn_clicked()
{
    QModelIndexList selectedRows = tableView->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "No Selection", "Please select an employee.");
        return;
    }

    int row = selectedRows.first().row();
    QString employeeId = model->data(model->index(row, model->fieldIndex("employee_id"))).toString();
    QString email = model->data(model->index(row, model->fieldIndex("email"))).toString();

    // Generate temporary password (e.g., using employee ID)
    QString tempPassword = employeeId;  // You can customize this pattern
    QString salt = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    QByteArray salted = (tempPassword + salt).toUtf8();
    QString hash = QString(QCryptographicHash::hash(salted, QCryptographicHash::Sha256).toHex());

    // Update password_hash, password_salt, and force_password_change
    QSqlQuery query;
    query.prepare("UPDATE employees SET password_hash = :hash, password_salt = :salt, force_password_change = 1 WHERE employee_id = :id");
    query.bindValue(":hash", hash);
    query.bindValue(":salt", salt);
    query.bindValue(":id", employeeId);

    if (query.exec()) {
        QMessageBox::information(this, "Password Reset",
                                 QString("Password has been reset to: %1\nPlease ask the employee to log in and change it.")
                                     .arg(tempPassword));
    } else {
        QMessageBox::critical(this, "Error", "Failed to reset password:\n" + query.lastError().text());
    }
}


