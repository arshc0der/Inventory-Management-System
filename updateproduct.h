#ifndef UPDATEPRODUCT_H
#define UPDATEPRODUCT_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class UpdateProduct;
}

class UpdateProduct : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdateProduct(QWidget *parent = nullptr);
    ~UpdateProduct();

private slots:
    void on_Update_btn_clicked();

private:
    Ui::UpdateProduct *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // UPDATEPRODUCT_H
