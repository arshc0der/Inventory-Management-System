#include "addsales.h"
#include "ui_addsales.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QDate>
#include <globalfunctions.h>

double total = 0;

AddSales::AddSales(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::AddSales),
    tableView(new QTableView(this)),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Set up the model with the appropriate column count
    model->setColumnCount(5); // Adjusted the column count to 5 to match the actual data being used
    model->setHorizontalHeaderLabels({"Product ID", "Product", "Quantity", "Sale Date", "Price","Discount","Customer ID","Total Price"});

    // Set the model for the table view
    tableView->setModel(model);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    ui->table_horizontalLayout->addWidget(tableView);
    tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->table_horizontalLayout->setStretch(0, 1);

    ui->Total_label->setText(QString::number(total, 'f', 2));  // Format total to two decimals

    // Adjust column sizes based on the content
    tableView->resizeColumnsToContents();

    // Set a minimum width for each column to ensure the table looks good
    for (int column = 0; column < model->columnCount(); ++column) {
        // Set a minimum width for the column, you can adjust this value as needed
        tableView->setColumnWidth(column, qMax(tableView->columnWidth(column), 100));  // Minimum width is 100px
    }
}

AddSales::~AddSales()
{
    delete ui;
}

void AddSales::on_ADD_Row_btn_clicked()
{
    // Retrieve user input values
    QString productId = ui->Product_id->text();
    QString productName = ui->Product_name->text();
    int quantity = ui->quantity_spinBox->value();
    double price = ui->price_doubleSpinBox->value();
    QDate saleDate = ui->purchase_Date_dateEdit->date();
    double discount = ui->discount_doubleSpinBox->value();
    QString customer_Id = ui->customer_id->text();
    double total_price_payable = price - ((price*discount)/100);
    if(customer_Id.isEmpty()){
        QMessageBox::warning(this, "Input Error", "Please fill customer");
        return;
    }

    // Check for empty fields or invalid values
    if (productId.isEmpty() || productName.isEmpty() || quantity <= 0 || price <= 0 || saleDate.isNull() || customer_Id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill all necessary fields.");
        return;
    }

    // Create a new row with the appropriate data
    QList<QStandardItem*> newRow;
    newRow.append(new QStandardItem(productId));         // Product ID
    newRow.append(new QStandardItem(productName));       // Product Name
    newRow.append(new QStandardItem(QString::number(quantity))); // Quantity
    newRow.append(new QStandardItem(saleDate.toString("yyyy-MM-dd"))); // Sale Date
    newRow.append(new QStandardItem(QString::number(price, 'f', 2))); // Price (formatted to 2 decimal places)
    newRow.append(new QStandardItem(QString::number(discount, 'f', 2))); // discount (formatted to 2 decimal places)
    newRow.append(new QStandardItem(customer_Id)); // Price (formatted to 2 decimal places)
    newRow.append(new QStandardItem(QString::number(total_price_payable, 'f', 2))); // total_price_payable (formatted to 2 decimal places)

    // Append the new row to the model
    model->appendRow(newRow);

    // Update the total by adding the price of the newly added product
    total += total_price_payable;

    // Update the total label in the UI, ensuring it's formatted correctly
    ui->Total_label->setText(QString::number(total, 'f', 2));
}

void AddSales::on_Del_Row_btn_clicked()
{
    // Get the selected indexes from the table view
    QModelIndexList selectedIndexes = tableView->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a product to delete.");
        return;
    }

    // Calculate the total price of the selected products to delete
    double totalToSubtract = 0;
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();
        QModelIndex priceIndex = model->index(row, 7);  // Assuming price is in the 5th column (index 4)
        double price = priceIndex.data().toString().toDouble();
        totalToSubtract += price;
    }

    // Remove selected rows from the model
    for (int i = selectedIndexes.size() - 1; i >= 0; --i) {
        model->removeRow(selectedIndexes[i].row());
    }

    // Update the total by subtracting the price of the deleted items
    total -= totalToSubtract;
    ui->Total_label->setText(QString::number(total, 'f', 2));

    QMessageBox::information(this, "Success", "Selected product(s) deleted successfully.");
}

void AddSales::on_Add_sale_btn_clicked()
{
    int rowCount = model->rowCount();
    if (rowCount == 0) {
        QMessageBox::warning(this, "Warning", "No products to insert.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Error", "Database is not open.");
        return;
    }

    QSqlQuery query(db);
    if (!query.exec("BEGIN TRANSACTION")) {
        QMessageBox::critical(this, "Error", "Failed to start database transaction.");
        return;
    }

    for (int row = 0; row < rowCount; ++row) {
        QString productId = model->item(row, 0)->text();
        int quantity = model->item(row, 2)->text().toInt();
        QDate saleDate = QDate::fromString(model->item(row, 3)->text(), "yyyy-MM-dd");
        double price = model->item(row, 4)->text().toDouble();
        double discount = model->item(row, 5)->text().toDouble();
        QString customer_id = model->item(row, 6)->text();

        query.prepare("INSERT INTO sales (product_id, quantity_sold, sale_date, total_price, discount, customer_id) "
                      "VALUES (:product_id, :quantity, :sale_date, :price, :discount, :customer_id)");

        query.bindValue(":product_id", productId);
        query.bindValue(":quantity", quantity);
        query.bindValue(":sale_date", saleDate.toString("yyyy-MM-dd"));
        query.bindValue(":price", price * quantity);  // Calculate total price
        query.bindValue(":discount", discount);
        query.bindValue(":customer_id", customer_id);

        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to insert product: " + query.lastError().text());
            query.exec("ROLLBACK");
            return;
        }
    }

    if (!query.exec("COMMIT")) {
        QMessageBox::critical(this, "Error", "Failed to commit transaction: " + query.lastError().text());
        return;
    }
    total=0;
    ui->Total_label->setText(QString::number(total, 'f', 2));
    QMessageBox::information(this, "Success", "Products Sale successfully");
    // Remove all rows from the model after successful insertion
    model->removeRows(0, rowCount);

    //logs
    GlobalFunctions::log_add_sale();
}

void AddSales::on_Product_id_textChanged(const QString &arg1)
{
    bool ok;
    int productId = arg1.toInt(&ok);

    if (!ok || productId <= 0) {
        ui->Product_id->clear();
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM products WHERE product_id = :product_id");
    query.bindValue(":product_id", productId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    if (query.next()) {
        QString productName = query.value("name").toString();
        int quantity = query.value("quantity").toInt();
        float price = query.value("price").toFloat();

        if (quantity == 0) {
            ui->statusbar->showMessage("The product is out of stock.", 3000);
            ui->statusbar->setStyleSheet("QStatusBar { background-color: yellow; color: white; }");
            return;
        }

        ui->Product_name->setText(productName);
        ui->quantity_spinBox->setValue(1);  // Default quantity
        ui->price_doubleSpinBox->setValue(price);

        ui->statusbar->showMessage("Product found customer with the given ID.", 3000);
        ui->statusbar->setStyleSheet("QStatusBar { background-color: green; color: white; }");
    } else {
        ui->Product_name->clear();
        ui->quantity_spinBox->setValue(0);  // Default quantity
        ui->price_doubleSpinBox->setValue(0);

        ui->statusbar->showMessage("No product found with the given ID.", 3000);
        ui->statusbar->setStyleSheet("QStatusBar { background-color: red; color: white; }");

    }
}

void AddSales::on_quantity_spinBox_valueChanged(int arg1)
{
    double price = ui->price_doubleSpinBox->value();
    ui->price_doubleSpinBox->setValue(price * arg1);
}

void AddSales::on_actionDel_Row_triggered()
{
    on_Del_Row_btn_clicked();
}

void AddSales::on_customer_id_textChanged(const QString &arg1)
{
    bool ok;
    int customerId = arg1.toInt(&ok);

    if (!ok || customerId <= 0) {
        ui->customer_id->clear();
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM customers WHERE customer_id = :customer_id");
    query.bindValue(":customer_id", customerId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    if (query.next()) {
        QString CustomerName = query.value("name").toString();
        QString Address = query.value("address").toString();
        QString Contact_info = query.value("contact_info").toString();
        int loyalty_points = query.value("loyalty_points").toInt();

        ui->customer_name->setText(CustomerName);
        ui->customer_addresss->setText(Address);
        ui->customer_label->setText(Contact_info);
        ui->customer_loyalty_points->setText(QString::number(loyalty_points));

        ui->statusbar->showMessage("Customer found with the given ID.", 3000);
        ui->statusbar->setStyleSheet("QStatusBar { background-color: green; color: white; }");
    } else {
        ui->customer_name->clear();
        ui->customer_addresss->clear();
        ui->customer_label->clear();
        ui->customer_loyalty_points->clear();
        ui->statusbar->showMessage("No customer found with the given ID.", 3000);
        ui->statusbar->setStyleSheet("QStatusBar { background-color: red; color: white; }");
    }
}


void AddSales::on_register_customer_btn_clicked()
{
    Register_Customer = new RegisterCustomer(this);
    Register_Customer->show();
}


void AddSales::on_check_discount_pushButton_clicked()
{
    checkdiscount = new CheckDiscount;
    //checkdiscount->show();
    checkdiscount->exec();
    // After the dialog is closed, retrieve the discount value
    QString discount = checkdiscount->getSelectedDiscount();

    // Handle the discount (e.g., display it or apply it to a price)
    if (!discount.isEmpty()) {
        QMessageBox::information(this, "Discount Selected", "You selected a discount of " + discount + "%.");
    } else {
        QMessageBox::warning(this, "No Discount", "No discount was selected.");
    }

    // Clean up the dialog
    delete checkdiscount;

    //setting discount in ui
    ui->discount_doubleSpinBox->setValue( discount.toDouble());
}

