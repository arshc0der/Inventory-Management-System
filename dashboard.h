#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QMainWindow>
#include <addproduct.h>
#include <updateproduct.h>
#include <deleteproduct.h>
#include <notification.h>
#include <addsales.h>
#include <updatesales.h>
#include <deletesales.h>
#include <addorders.h>
#include <updateorders.h>
#include <deleteorders.h>
#include <addemployee.h>
#include <updateemployee.h>
#include <deleteemployee.h>
#include <addpromotion.h>
#include <updatepromotion.h>
#include <deletepromotion.h>
#include <updatepermissions.h>
#include <addadjustment.h>
//check admin
#include <check_admin.h>
#include <reset_password_dialog.h>

namespace Ui {
class DashBoard;
}

class DashBoard : public QMainWindow
{
    Q_OBJECT

public:
    explicit DashBoard(QWidget *parent = nullptr);
    ~DashBoard();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_dashboard_btn_clicked();

    void on_product_btn_clicked();

    void on_supplier_btn_clicked();

    void on_add_product_btn_clicked();

    void on_Update_product_btn_clicked();

    void on_Delete_Product_btn_clicked();

    void on_notification_btn_clicked();

    void on_users_btn_clicked();

    void on_settings_btn_clicked();

    void on_add_sales_btn_clicked();

    void on_update_sales_btn_clicked();

    void on_delete_sales_btn_clicked();

    void on_orders_btn_clicked();

    void on_activity_btn_clicked();

    void on_addorders_btn_clicked();

    void on_update_orders_btn_clicked();

    void on_delete_orders_btn_clicked();

    void on_add_emp_btn_clicked();

    void on_promotion_btn_clicked();

    void on_add_promotion_btn_clicked();

    void on_update_promotion_btn_clicked();

    void on_delete_promotion_btn_clicked();

    void on_update_employee_btn_clicked();

    void on_delete_employee_btn_clicked();

    void on_update_permission_btn_clicked();

    void on_adjust_stock_btn_clicked();

    void on_add_adjustments_btn_clicked();

    void on_logout_btn_clicked();

private:
    Ui::DashBoard *ui;
    AddProduct *add_product;
    UpdateProduct *Update_Product;
    DeleteProduct *Delete_Product;
    Notification *notification;
    AddSales *Add_Sales;
    UpdateSales *Update_Sales;
    DeleteSales *Delete_Sales;
    AddOrders  *addorders;
    UpdateOrders *Update_Orders;
    DeleteOrders *Delete_Orders;
    AddEmployee *Add_Employee;
    UpdateEmployee *Update_Employee;
    DeleteEmployee *Delete_Employee;
    AddPromotion *Add_Promotion;
    UpdatePromotion *Update_Promotion;
    DeletePromotion *Delete_Promotion;
    UpdatePermissions *Update_Permissions;
    AddAdjustment *Add_Adjustment;
    //check admin
    check_Admin *check_Admin_Dialog;
    Reset_Password_Dialog *Reset_Dialog;

    //charts
    void Sales_Trends_loader();
    void Products_Hightlights_Min_Level_Chart_Loader();
    void Orders_Trends_Loader();

    void Get_Total_No_Products();
    void Get_Total_Inventory_Value();
    void Get_Total_No_Products_OutOfStock();
    void Get_Total_No_Products_Low_Stock();
    void Get_No_Top_Selling_Products();
    void Get_No_Top_Selling_Products_value();
    void Get_Total_No_Sales();
    void Get_Total_Sales_Value();
    void Get_Total_No_Suppliers();
    void Get_Total_Orders();
    void Get_Total_Orders_Cost();
    void Get_Total_Completed_Orders();
    void Get_Total_Pending_Orders();
    void Get_Total_Cancelled_Orders();
    void Get_Total_Cancelled_Orders_Cost();
    void Load_manage_actvity();
    void Get_No_Login();
    void Get_No_Logout();
    void Get_Total_No_Employee();
    void Get_Total_Active_Employee();
    void Get_Total_Inactive_Employee();
    void Get_Average_salary_employee();
    void Get_Active_Promotions();
    void Get_Upcoming_Promotions();
    void Get_Expired_Promotions();
    void Get_Discount_Highlights();
    QList<Notification*> notifications;
    //settings uer profile
    void fetchEmployeeInfo(int employeeId);
    //insertStartupNotifications
    void insertStartupNotifications();
    //loadDefaultNotifications
    void loadDefaultNotifications();
};

#endif // DASHBOARD_H
