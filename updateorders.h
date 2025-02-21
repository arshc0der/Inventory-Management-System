#ifndef UPDATEORDERS_H
#define UPDATEORDERS_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class UpdateOrders;
}

class UpdateOrders : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdateOrders(QWidget *parent = nullptr);
    ~UpdateOrders();

private slots:
    void on_Update_btn_clicked();

private:
    Ui::UpdateOrders *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // UPDATEORDERS_H
