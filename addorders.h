#ifndef ADDORDERS_H
#define ADDORDERS_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <checkdiscount.h>
#include <registercustomer.h>

namespace Ui {
class AddOrders;
}

class AddOrders : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddOrders(QWidget *parent = nullptr);
    ~AddOrders();

private slots:
    void on_check_discount_btn_clicked();

    void on_ADD_Row_clicked();

    void on_Del_Row_clicked();

    void on_customer_id_textChanged(const QString &arg1);

    void on_product_id_textChanged(const QString &arg1);

    void on_add_orders_btn_clicked();

    void on_addCustomer_btn_clicked();

private:
    Ui::AddOrders *ui;
    QTableView *tableView;
    QStandardItemModel *model;
    CheckDiscount *checkdiscount;
    RegisterCustomer * Register_Customer;
};

#endif // ADDORDERS_H
