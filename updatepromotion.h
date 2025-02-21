#ifndef UPDATEPROMOTION_H
#define UPDATEPROMOTION_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class UpdatePromotion;
}

class UpdatePromotion : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdatePromotion(QWidget *parent = nullptr);
    ~UpdatePromotion();

private slots:
    void on_Update_btn_clicked();

private:
    Ui::UpdatePromotion *ui;
    void refreshTable();
    QTableView *tableView;
    QSqlTableModel *model;
};

#endif // UPDATEPROMOTION_H


