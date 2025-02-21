#ifndef MANAGEACTIVITY_H
#define MANAGEACTIVITY_H
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QWidget>

namespace Ui {
class ManageActivity;
}

class ManageActivity : public QWidget
{
    Q_OBJECT

public:
    explicit ManageActivity(QWidget *parent = nullptr);
    ~ManageActivity();

private slots:
    void on_del_btn_clicked();

    void on_delete_all_btn_clicked();

private:
    Ui::ManageActivity *ui;
    QTableView *tableView;
    QSqlTableModel *model;
    void refreshTable();
};

#endif // MANAGEACTIVITY_H
