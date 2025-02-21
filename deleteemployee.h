#ifndef DELETEEMPLOYEE_H
#define DELETEEMPLOYEE_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class DeleteEmployee;
}

class DeleteEmployee : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeleteEmployee(QWidget *parent = nullptr);
    ~DeleteEmployee();

private slots:
    void on_Delete_btn_clicked();

private:
    Ui::DeleteEmployee *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // DELETEEMPLOYEE_H

