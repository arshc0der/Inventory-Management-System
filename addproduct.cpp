#include "addproduct.h"
#include "ui_addproduct.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QString>
#include <QDate>
#include <globalfunctions.h>

AddProduct::AddProduct(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AddProduct)
    ,tableView(new QTableView(this)),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Set up the model with some sample data (e.g., "Product", "Category", "Price")
    model->setColumnCount(6);  // Define three columns
    model->setHorizontalHeaderLabels({"Product ID", "Name","Category", "Quantity","Price","Supplier ID","Purcahse Date","Minimum Stock Level","Description "});

    // Set the model for the table view
    tableView->setModel(model);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    ui->table_horizontalLayout->addWidget(tableView);
    tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->table_horizontalLayout->setStretch(0, 1);

    // Adjust column sizes based on the content
    tableView->resizeColumnsToContents();

    // Set a minimum width for each column to ensure the table looks good
    for (int column = 0; column < model->columnCount(); ++column) {
        // Set a minimum width for the column, you can adjust this value as needed
        tableView->setColumnWidth(column, qMax(tableView->columnWidth(column), 100));  // Minimum width is 100px
    }

    //Load Category
    loadCategories();
    //load supplier
    loadSupplier_id();
}

AddProduct::~AddProduct()
{
    delete ui;
}


// Function to load unique categories into the combo box
void AddProduct::loadCategories()
{
    // Prepare the SQL query to get distinct categories
    QSqlQuery query;
    query.prepare("SELECT DISTINCT category FROM products");

    // Execute the query
    if (query.exec()) {
        // Clear the existing items in the combo box (if any)
        ui->category_comboBox->clear();

        // Iterate through the query results and add each category to the combo box
        while (query.next()) {
            QString category = query.value(0).toString();  // Get the category value
            ui->category_comboBox->addItem(category);  // Add the category to the combo box
        }
    } else {
        qDebug() << "Error executing query: " << query.lastError().text();
    }
}

void AddProduct::loadSupplier_id()
{
    // Prepare the SQL query to get distinct categories
    QSqlQuery query;
    query.prepare("SELECT DISTINCT supplier_id FROM suppliers");

    // Execute the query
    if (query.exec()) {
        // Clear the existing items in the combo box (if any)
        ui->supplier_comboBox->clear();

        // Iterate through the query results and add each category to the combo box
        while (query.next()) {
            QString category = query.value(0).toString();  // Get the category value
            ui->supplier_comboBox->addItem(category);  // Add the category to the combo box
        }
    } else {
        qDebug() << "Error executing query: " << query.lastError().text();
    }
}


void AddProduct::on_ADD_Row_btn_clicked()
{
    // Retrieve user input values
    QString product_id = ui->produt_edit_lineEdit->text();
    QString name = ui->name_lineEdit->text();
    QString Custom_category = ui->lineEdit_category->text();
    QString category = (!Custom_category.isEmpty()) ? ui->lineEdit_category->text() : ui->category_comboBox->currentText();
    int quantity = ui->quantity_spinBox->value();
    double price = ui->price_doubleSpinBox->value();
    QString supplier_id = ui->supplier_comboBox->currentText();
    QDate purchase_date = ui->purchase_Date_dateEdit->date();
    int min_Stock_level = ui->min_stock_level_spinBox->value();
    QString Description = ui->description_plainTextEdit->toPlainText();

    // Check for empty fields or invalid values
    if (product_id.isEmpty() || name.isEmpty() || category.isEmpty() || quantity == 0 || price == 0 || supplier_id.isEmpty() || purchase_date.isNull()) {
        QMessageBox::warning(this, "Input Error", "Please fill all necessary fields.");
    } else {
        // Create a new row with the appropriate data
        QList<QStandardItem*> newRow;

        newRow.append(new QStandardItem(product_id));  // Product ID
        newRow.append(new QStandardItem(name));        // Product Name
        newRow.append(new QStandardItem(category));    // Category
        newRow.append(new QStandardItem(QString::number(quantity)));      // Quantity
        newRow.append(new QStandardItem(QString::number(price, 'f', 2)));  // Price (formatted to 2 decimal places)
        newRow.append(new QStandardItem(supplier_id)); // Supplier ID
        newRow.append(new QStandardItem(purchase_date.toString("yyyy-MM-dd"))); // Purchase Date
        newRow.append(new QStandardItem(QString::number(min_Stock_level))); // minimum stock level Date
        newRow.append(new QStandardItem(Description)); // description Date

        // Append the new row to the model
        model->appendRow(newRow);

        // Optionally, select the newly added row for user convenience
        int newRowIndex = model->rowCount() - 1;  // The newly added row is the last one
        tableView->selectRow(newRowIndex);
    }
}



void AddProduct::on_Del_Row_btn_clicked()
{
    // Get the selected indexes from the table view
    QModelIndexList selectedIndexes = tableView->selectionModel()->selectedRows();

    // Check if at least one row is selected
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a product to delete.");
        return;
    }

    // Loop through selected rows in reverse order (to avoid shifting issues)
    for (int i = selectedIndexes.size() - 1; i >= 0; --i) {
        int row = selectedIndexes[i].row();  // Get the row index of the selected item

        // Remove the selected row from the model
        if (!model->removeRow(row)) {
            QMessageBox::critical(this, "Error", "Failed to delete the product.");
            return;
        }
    }

    // Show a success message after deleting
    QMessageBox::information(this, "Success", "Selected product(s) deleted successfully.");
}


void AddProduct::on_Add_Products_btn_clicked()
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

    // Loop through all the rows and insert each row's data into the database
    for (int row = 0; row < rowCount; ++row) {
        QString product_id = model->item(row, 0)->text();
        QString name = model->item(row, 1)->text();
        QString category = model->item(row, 2)->text();
        int quantity = model->item(row, 3)->text().toInt();
        double price = model->item(row, 4)->text().toDouble();
        QString supplier_id = model->item(row, 5)->text();
        QDate purchase_date = QDate::fromString(model->item(row, 6)->text(), "yyyy-MM-dd");
        int min_stock_level = model->item(row, 7)->text().toInt();
        QString Description = model->item(row, 8)->text();

        // Prepare the SQL insert query
        query.prepare("INSERT INTO products (product_id, name, category, quantity, price, supplier_id, purchase_date, minimum_stock_level, description) "
                      "VALUES (:product_id, :name, :category, :quantity, :price, :supplier_id, :purchase_date, :minimum_stock_level, :description)");

        // Bind the values to the query
        query.bindValue(":product_id", product_id);
        query.bindValue(":name", name);
        query.bindValue(":category", category);
        query.bindValue(":price", price);
        query.bindValue(":quantity", quantity);
        query.bindValue(":supplier_id", supplier_id);
        query.bindValue(":purchase_date", purchase_date.toString("yyyy-MM-dd"));
        query.bindValue(":minimum_stock_level", min_stock_level);
        query.bindValue(":description", Description);

        // Execute the query for each row
        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to insert product: " + query.lastError().text());
            query.exec("ROLLBACK");
            return;
        }
    }

    // Commit the transaction
    if (!query.exec("COMMIT")) {
        QMessageBox::critical(this, "Error", "Failed to commit transaction: " + query.lastError().text());
        return;
    }

    // Inform the user that the products have been added successfully
    QMessageBox::information(this, "Success", "Products added successfully to the database.");
    // Remove all rows from the model
    model->removeRows(0, model->rowCount());
    //Load Category
    loadCategories();
    //load supplier
    loadSupplier_id();
    //logs
    GlobalFunctions::log_add_product();
}

void AddProduct::on_actionDel_Row_triggered()
{
    on_Del_Row_btn_clicked();
}


void AddProduct::on_register_supplier_btn_clicked()
{
    Add_Supplier addSupplierDialog(this);
    addSupplierDialog.exec(); // Blocks until closed

    // Load supplier after closing the dialog
    loadSupplier_id();
}

