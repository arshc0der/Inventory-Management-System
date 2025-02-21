#ifndef DELETEORDERS_H
#define DELETEORDERS_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class DeleteOrders;
}

class DeleteOrders : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeleteOrders(QWidget *parent = nullptr);
    ~DeleteOrders();

private slots:
    void on_Delete_orders_btn_clicked();

private:
    Ui::DeleteOrders *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // DELETEORDERS_H
