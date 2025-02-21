#ifndef ADDSALES_H
#define ADDSALES_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <registercustomer.h>

namespace Ui {
class AddSales;
}

class AddSales : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddSales(QWidget *parent = nullptr);
    ~AddSales();

private slots:
    void on_Del_Row_btn_clicked();

    void on_ADD_Row_btn_clicked();

    void on_Add_sale_btn_clicked();

    void on_Product_id_textChanged(const QString &arg1);

    void on_quantity_spinBox_valueChanged(int arg1);

    void on_actionDel_Row_triggered();

    void on_customer_id_textChanged(const QString &arg1);

    void on_register_customer_btn_clicked();

private:
    Ui::AddSales *ui;
    QTableView *tableView;
    QStandardItemModel *model;
    RegisterCustomer *Register_Customer;
};

#endif // ADDSALES_H
