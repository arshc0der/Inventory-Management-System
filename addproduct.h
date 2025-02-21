#ifndef ADDPRODUCT_H
#define ADDPRODUCT_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <add_supplier.h>

namespace Ui {
class AddProduct;
}

class AddProduct : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddProduct(QWidget *parent = nullptr);
    ~AddProduct();

private slots:
    void on_Add_Products_btn_clicked();

    void on_ADD_Row_btn_clicked();

    void on_Del_Row_btn_clicked();

    void on_actionDel_Row_triggered();

    void on_register_supplier_btn_clicked();

private:
    Ui::AddProduct *ui;
    void loadCategories();
    void loadSupplier_id();
    QTableView *tableView;
    QStandardItemModel *model;
    Add_Supplier *AddSupplier;
};

#endif // ADDPRODUCT_H
