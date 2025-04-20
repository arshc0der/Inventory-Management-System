#ifndef ADDEMPLOYEE_H
#define ADDEMPLOYEE_H

#include <QMainWindow>

namespace Ui {
class AddEmployee;
}

class AddEmployee : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddEmployee(QWidget *parent = nullptr);
    ~AddEmployee();

private slots:
    void on_open_btn_clicked();

    void on_sales_menu_btn_clicked();

    void on_orders_menu_btn_clicked();

    void on_users_menu_btn_clicked();

    void on_main_dashboard_menu_btn_clicked();

    void on_add_employee_btn_clicked();

    void on_product_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_sales_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_orders_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_user_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_promotion_dashboard_menu_btn_clicked();

    void on_promotion_dashboard_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_password_view_hide_btn_clicked();

    void on_select_profile_image_clicked();

    void on_password_comboBox_activated(int index);

private:
    Ui::AddEmployee *ui;

    void reset_permission();
};

#endif // ADDEMPLOYEE_H
