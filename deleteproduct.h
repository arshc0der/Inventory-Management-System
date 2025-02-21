#ifndef DELETEPRODUCT_H
#define DELETEPRODUCT_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class DeleteProduct;
}

class DeleteProduct : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeleteProduct(QWidget *parent = nullptr);
    ~DeleteProduct();

private slots:
    void on_Delete_Product_btn_clicked();

private:
    Ui::DeleteProduct *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // DELETEPRODUCT_H
