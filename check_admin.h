#ifndef CHECK_ADMIN_H
#define CHECK_ADMIN_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>
namespace Ui {
class check_Admin;
}

class check_Admin : public QMainWindow
{
    Q_OBJECT

public:
    explicit check_Admin(QWidget *parent = nullptr);
    ~check_Admin();

private slots:
    void on_Delete_btn_clicked();

private:
    Ui::check_Admin *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // CHECK_ADMIN_H

