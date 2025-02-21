#ifndef DELETEPROMOTION_H
#define DELETEPROMOTION_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class DeletePromotion;
}

class DeletePromotion : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeletePromotion(QWidget *parent = nullptr);
    ~DeletePromotion();

private slots:
    void on_Delete_btn_clicked();

private:
    Ui::DeletePromotion *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // DELETEPROMOTION_H
