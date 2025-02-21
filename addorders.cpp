#include "addorders.h"
#include "ui_addorders.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QString>
#include <QDate>
#include <globalfunctions.h>

double total_orders = 0;

AddOrders::AddOrders(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AddOrders)
    ,tableView(new QTableView(this)),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Set up the model with some sample data (e.g., "Product", "Category", "Price")
    model->setColumnCount(6);  // Define three columns
    model->setHorizontalHeaderLabels({"Customer ID", "Order Date","Status", "Product ID","Qunatity","Price","Total Price"});

    // Set the model for the table view
    tableView->setModel(model);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    ui->table_horizontalLayout->addWidget(tableView);
    tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->table_horizontalLayout->setStretch(0, 1);
}

AddOrders::~AddOrders()
{
    delete ui;
}

void AddOrders::on_check_discount_btn_clicked()
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


void AddOrders::on_ADD_Row_clicked()
{
    // Retrieve user input values
    QString Customer_ID = ui->customer_id->text();
    QDate Order_Date = ui->order_date->date();
    QString Status = ui->status->currentText();
    QString Product_ID = ui->product_id->text();
    int Quantity = ui->quantity->value();
    double Price = ui->price->value();
    double Total_Price = Price*Quantity;


    // Check for empty fields or invalid values
    if (Customer_ID.isEmpty() || Status.isEmpty() || Product_ID.isEmpty() || Quantity == 0 || Price == 0 || Order_Date.isNull() || Total_Price==0) {
        QMessageBox::warning(this, "Input Error", "Please fill all necessary fields.");
    } else {
        // Create a new row with the appropriate data
        QList<QStandardItem*> newRow;

        newRow.append(new QStandardItem(Customer_ID));  // Customer_ID
        newRow.append(new QStandardItem(Order_Date.toString()));        // Order_Date
        newRow.append(new QStandardItem(Status));    // Status
        newRow.append(new QStandardItem(Product_ID));    // Product_ID
        newRow.append(new QStandardItem(QString::number(Quantity)));      // Quantity
        newRow.append(new QStandardItem(QString::number(Price, 'f', 2)));  //Price
        newRow.append(new QStandardItem(QString::number(Total_Price, 'f', 2)));  // Total_Price

        // Append the new row to the model
        model->appendRow(newRow);

        // Optionally, select the newly added row for user convenience
        int newRowIndex = model->rowCount() - 1;  // The newly added row is the last one
        tableView->selectRow(newRowIndex);

        // Update the total by adding the price of the newly added product
        total_orders += Total_Price;

        // Update the total label in the UI, ensuring it's formatted correctly
        ui->Total_label->setText(QString::number(total_orders, 'f', 2));
    }
}



void AddOrders::on_Del_Row_clicked()
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
        QModelIndex priceIndex = model->index(row, 6);  // Assuming price is in the 7th column (index 6)
        double price = priceIndex.data().toString().toDouble();
        totalToSubtract += price;
    }

    // Remove selected rows from the model
    for (int i = selectedIndexes.size() - 1; i >= 0; --i) {
        model->removeRow(selectedIndexes[i].row());
    }

    // Update the total by subtracting the price of the deleted items
    total_orders -= totalToSubtract;
    ui->Total_label->setText(QString::number(total_orders, 'f', 2));

    QMessageBox::information(this, "Success", "Selected product(s) deleted successfully.");
}


void AddOrders::on_customer_id_textChanged(const QString &arg1)
{
    bool ok;
    int customerID = arg1.toInt(&ok);

    if (!ok || customerID <= 0) {
        ui->customer_id->clear();
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM customers WHERE customer_id = :customer_id");
    query.bindValue(":customer_id", customerID);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    if (query.next()) {
        QString name = query.value("name").toString();
        ui->customer_name->setText(name);

        ui->statusbar->showMessage("Found customer with the given ID.", 3000);
        ui->statusbar->setStyleSheet("QStatusBar { background-color: green; color: white; }");
    } else {
        ui->customer_name->clear();
        // Set custom CSS
        ui->statusbar->showMessage("No customer found with the given ID.", 3000);
        ui->statusbar->setStyleSheet("QStatusBar { background-color: red; color: white; }");
    }
}


void AddOrders::on_product_id_textChanged(const QString &arg1)
{
    bool ok;
    int productId = arg1.toInt(&ok);

    if (!ok || productId <= 0) {
        ui->product_id->clear();
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
            QMessageBox::critical(this, "Out of Stock", "The product is out of stock.");
            return;
        }

        ui->product_name->setText(productName);
        ui->quantity->setValue(1);  // Default quantity
        ui->price->setValue(price);

        ui->statusbar->showMessage("Found product with the given ID.", 3000);
        ui->statusbar->setStyleSheet("QStatusBar { background-color: green; color: white; }");
    } else {
        ui->product_name->clear();
        ui->quantity->setValue(0);  // 0 quantity
        ui->price->setValue(0);
        // Set custom CSS
        ui->statusbar->setStyleSheet("QStatusBar { background-color: red; color: white; }");
        ui->statusbar->showMessage("No product found with the given ID.", 3000);
    }
}


void AddOrders::on_add_orders_btn_clicked()
{
    // Get the model from the tableView (assuming it is a QStandardItemModel)
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(tableView->model());
    if (!model) {
        QMessageBox::critical(this, "Error", "Failed to get model from table view.");
        return;
    }

    // Check if there are any rows to insert
    int rowCount = model->rowCount();
    if (rowCount == 0) {
        QMessageBox::warning(this, "Warning", "No products to insert.");
        return;
    }

    // Get the customer ID, order date, status, and total price from the model or UI
    QString customer_id = model->item(0, 0)->text(); // Assuming the first row holds the customer ID
    QString order_date = model->item(0, 1)->text(); // Order date from the first row
    QString status = model->item(0, 2)->text();     // Status from the first row
    double total_price = 0;                          // Total price will be calculated

    // Loop through all rows to calculate the total price
    for (int row = 0; row < rowCount; ++row) {
        double price = model->item(row, 5)->text().toDouble(); // Price from the model
        int quantity = model->item(row, 4)->text().toInt();   // Quantity from the model
        total_price += price * quantity; // Calculate the total price
    }

    // Start a database transaction to make the insertions more efficient
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

    // Insert into the orders table first
    query.prepare("INSERT INTO orders (customer_id, order_date, status, total_price) "
                  "VALUES (:customer_id, :order_date, :status, :total_price)");

    query.bindValue(":customer_id", customer_id);
    query.bindValue(":order_date", order_date);
    query.bindValue(":status", status);
    query.bindValue(":total_price", total_price);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to insert into orders: " + query.lastError().text());
        query.exec("ROLLBACK");
        return;
    }

    // Get the last inserted order_id (auto-generated)
    int order_id = query.lastInsertId().toInt();

    // Insert into the order_details table for each row
    for (int row = 0; row < rowCount; ++row) {
        QString product_id = model->item(row, 3)->text(); // Product ID from the model
        int quantity = model->item(row, 4)->text().toInt(); // Quantity from the model
        double price = model->item(row, 5)->text().toDouble(); // Price from the model

        // Prepare the SQL insert query for order_details
        query.prepare("INSERT INTO order_details (order_id, product_id, quantity, price) "
                      "VALUES (:order_id, :product_id, :quantity, :price)");

        query.bindValue(":order_id", order_id); // Use the generated order_id
        query.bindValue(":product_id", product_id);
        query.bindValue(":quantity", quantity);
        query.bindValue(":price", price);

        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to insert into order_details: " + query.lastError().text());
            query.exec("ROLLBACK");
            return;
        }
    }

    // Commit the transaction if all inserts were successful
    if (!query.exec("COMMIT")) {
        QMessageBox::critical(this, "Error", "Failed to commit transaction: " + query.lastError().text());
        return;
    }

    // Inform the user that the order has been added successfully
    QMessageBox::information(this, "Success", "Order added successfully to the database.");

    // Remove all rows from the model
    model->removeRows(0, model->rowCount());
}



void AddOrders::on_addCustomer_btn_clicked()
{
    Register_Customer = new RegisterCustomer(this);
    Register_Customer->show();
}

