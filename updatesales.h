#ifndef UPDATESALES_H
#define UPDATESALES_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class UpdateSales;
}

class UpdateSales : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdateSales(QWidget *parent = nullptr);
    ~UpdateSales();

private slots:
    void on_Update_btn_clicked();

private:
    Ui::UpdateSales *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // UPDATESALES_H


