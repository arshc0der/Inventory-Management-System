#ifndef UPDATEEMPLOYEE_H
#define UPDATEEMPLOYEE_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class UpdateEmployee;
}

class UpdateEmployee : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdateEmployee(QWidget *parent = nullptr);
    ~UpdateEmployee();

private slots:
    void on_Update_btn_clicked();

private:
    Ui::UpdateEmployee *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // UPDATEEMPLOYEE_H
