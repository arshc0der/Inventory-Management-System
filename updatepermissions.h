#ifndef UPDATEPERMISSIONS_H
#define UPDATEPERMISSIONS_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class UpdatePermissions;
}

class UpdatePermissions : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdatePermissions(QWidget *parent = nullptr);
    ~UpdatePermissions();

private slots:
    void on_Update_btn_clicked();

private:
    Ui::UpdatePermissions *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // UPDATEPERMISSIONS_H
