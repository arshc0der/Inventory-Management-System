#ifndef DELETESALES_H
#define DELETESALES_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>


namespace Ui {
class DeleteSales;
}

class DeleteSales : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeleteSales(QWidget *parent = nullptr);
    ~DeleteSales();

private slots:
    void on_Delete_Product_btn_clicked();

private:
    Ui::DeleteSales *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // DELETESALES_H
