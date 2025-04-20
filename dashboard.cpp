#include "dashboard.h"
#include "ui_dashboard.h"
#include <QMessageBox>
#include "DatabaseManager.h"  // Singleton database connection
#include <QCloseEvent>
#include <QThread>

#include <QLocale>
#include <QSqlQuery>
#include <QSqlError>
#include <globalfunctions.h>
//manage activity widget
#include <manageactivity.h>
#include <salestrends_chart.h>
#include <products_hightlights_min_level_chart.h>
#include <orders_trends.h>
#include <QVariant>
#include <QDebug>

DashBoard::DashBoard(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DashBoard)
{
    ui->setupUi(this);

    //insert deafult notification
    insertStartupNotifications();

    //charts
    Sales_Trends_loader();
    Products_Hightlights_Min_Level_Chart_Loader();
    Orders_Trends_Loader();

    //product Dashboard
    if (GlobalFunctions::hasPermission("product_dashboard")) {
        Get_Total_No_Products();
        Get_Total_Inventory_Value();
        Get_Total_No_Products_OutOfStock();
        Get_Total_No_Products_Low_Stock();
        Get_No_Top_Selling_Products();
        Get_No_Top_Selling_Products_value();
    }

    //Sales Dashbaord
    if (GlobalFunctions::hasPermission("sales_dashboard")) {
        Get_Total_No_Sales();
        Get_Total_Sales_Value();
        Get_Total_No_Suppliers();
        Get_Total_No_Products_Low_Stock();
        Get_No_Top_Selling_Products();
        Get_No_Top_Selling_Products_value();
    }

    //Orders Dashboard
    if (GlobalFunctions::hasPermission("orders_dashboard")) {
        Get_Total_Orders();
        Get_Total_Orders_Cost();
        Get_Total_Completed_Orders();
        Get_Total_Pending_Orders();
        Get_Total_Cancelled_Orders();
        Get_Total_Cancelled_Orders_Cost();
        Load_manage_actvity();
    }

    //Activity Dashboard
    if (GlobalFunctions::hasPermission("activity_dashboard")) {
        Get_No_Login();
        Get_No_Logout();
    }

    //users Dashboard
    if (GlobalFunctions::hasPermission("user_dashboard")) {
        Get_Total_No_Employee();
        Get_Total_Active_Employee();
        Get_Total_Inactive_Employee();
        Get_Average_salary_employee();
    }

    //promotion Dashboard
    if (GlobalFunctions::hasPermission("promotion_dashboard")) {
        Get_Active_Promotions();
        Get_Upcoming_Promotions();
        Get_Expired_Promotions();
        Get_Discount_Highlights();
    }

    //Seetings DashBoard
    if (GlobalFunctions::hasPermission("settings_dashboard")) {
        fetchEmployeeInfo(GlobalFunctions::get_user_id());
    }

    //showing the info of multiple admin accounts deleted
    if(GlobalFunctions::is_admin()){
        QSqlQuery checkAdmin;
        checkAdmin.prepare("SELECT COUNT(*) FROM users");

        if (checkAdmin.exec() && checkAdmin.next()) {
            int adminCount = checkAdmin.value(0).toInt();

            if (adminCount > 1) {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::warning(this, "Multiple Admins Detected",
                                             QString("There are %1 admin accounts in the system!\n"
                                                     "Only 1 admin is allowed.\n\n"
                                                     "Do you want to delete the extra admin accounts?")
                                                 .arg(adminCount),
                                             QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::Yes) {
                    check_Admin_Dialog = new check_Admin(nullptr);
                    check_Admin_Dialog->show();
                }
            }
        }
    }

    if(!GlobalFunctions::is_admin()){
        int employee_id = GlobalFunctions::get_user_id();

        // Create a query object
        QSqlQuery query;

        // Prepare the SQL query
        query.prepare("SELECT force_password_change FROM employees WHERE employee_id = :employee_id AND force_password_change = 1");

        // Bind the employee_id value to the query
        query.bindValue(":employee_id", employee_id);

        // Execute the query
        if (!query.exec()) {
            // Handle error if query execution fails
            qDebug() << "Query failed: " << query.lastError().text();
        }

        // Check if a record was returned
        if (query.next()) {
            // If the query returns a result, it means force_password_change is 1
            Reset_Dialog = new Reset_Password_Dialog("employee",QString::number(employee_id),"",this);
            Reset_Dialog->exec();

        } else {
            // No record found with force_password_change = 1 for the given employee_id
        }

    }
}

DashBoard::~DashBoard()
{
    delete ui;
}

void DashBoard::closeEvent(QCloseEvent *event)
{
    // Confirm with the user before closing the application (optional)
    if (QMessageBox::question(this, "Close IMS", "Are you sure you want to exit and end all processes?") == QMessageBox::Yes) {
        //logs
        GlobalFunctions::log_user_logout();

        // Perform any cleanup steps before quitting the application

        // 1. Close the database connection
        DatabaseManager &dbManager = DatabaseManager::instance();
        if (dbManager.getDatabase().isOpen()) {
            dbManager.getDatabase().close();
            qDebug() << "Database connection closed.";
        }

        // 2. Terminate any running background threads (if you have any)
        // If you have any threads or processes running, terminate them here.
        // For example, if you have a worker thread:
        //workerThread->terminate();  // Ensure to safely terminate background threads

        // 3. Ensure all resources are cleaned up
        // If you have any additional cleanup tasks (files, network connections, etc.), do them here.

        // 4. Exit the application
        QApplication::quit();  // This will terminate the application
        event->accept();  // Accept the close event and proceed with application shutdown
    } else {
        event->ignore();  // If the user cancels, keep the window open
    }
}

void DashBoard::on_dashboard_btn_clicked()
{
    ui->dashboard_btn->setStyleSheet(
        "QPushButton {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
        "} "
        );

    ui->product_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/product-blue.svg);"
        "} "
        );

    ui->supplier_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
        "} "
        );

    ui->orders_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
        "} "
        );

    ui->notification_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
        "} "
        );

    ui->users_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/user-blue.svg);"
        "} "
        );

    ui->settings_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
        "} "
        );

    ui->activity_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
        "} "
        );

    ui->promotion_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
        "} "
        );

    ui->adjust_stock_btn->setStyleSheet(
        "QPushButton:hover {"
        "color: rgb(74, 144, 226);"
        "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
        "} "
        );

    ui->stackedWidget->setCurrentIndex(0);
}

void DashBoard::on_product_btn_clicked()
{
    if (GlobalFunctions::hasPermission("product_dashboard")) {
        // Allow the user to add a product
        ui->product_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        ui->stackedWidget->setCurrentIndex(1);
    } else {
        // Deny access
         QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Product Dashboard");
    }
}

void DashBoard::on_supplier_btn_clicked()
{
    if (GlobalFunctions::hasPermission("sales_dashboard")) {
        // Allow the user to add a product
        ui->supplier_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        ui->stackedWidget->setCurrentIndex(2);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Sales Dashboard");
    }
}

void DashBoard::on_notification_btn_clicked()
{
    if (GlobalFunctions::hasPermission("notification_dashboard")) {
        ui->notification_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        // Set the current widget to the notifications section (index 4)
        ui->stackedWidget->setCurrentIndex(4);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Notifications");
    }
}


void DashBoard::on_users_btn_clicked()
{
    if (GlobalFunctions::hasPermission("user_dashboard")) {
        ui->users_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        // Set the current widget to the uers section (index 6)
        ui->stackedWidget->setCurrentIndex(6);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Employee Dashboard");
    }
}


void DashBoard::on_settings_btn_clicked()
{
    if (GlobalFunctions::hasPermission("settings_dashboard")) {
        ui->settings_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        ui->stackedWidget->setCurrentIndex(9);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Settings");
    }
}

void DashBoard::on_orders_btn_clicked()
{
    if (GlobalFunctions::hasPermission("orders_dashboard")) {
        ui->orders_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        // Set the current widget to the orders section (index 3)
        ui->stackedWidget->setCurrentIndex(3);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Orders Dashboard");
    }
}

void DashBoard::on_activity_btn_clicked()
{
    if (GlobalFunctions::hasPermission("activity_dashboard")) {
        ui->activity_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        // Set the current widget to the activity section (index 5)
        ui->stackedWidget->setCurrentIndex(5);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Activity Dashboard");
    }
}

void DashBoard::on_promotion_btn_clicked()
{
    if (GlobalFunctions::hasPermission("promotion_dashboard")) {
        ui->promotion_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );


        // Set the current widget to the promotion section (index 7)
        ui->stackedWidget->setCurrentIndex(7);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Promotion Dashboard");
    }
}
void DashBoard::on_adjust_stock_btn_clicked()
{
    if (GlobalFunctions::hasPermission("adjustment_Stock")) {
        ui->adjust_stock_btn->setStyleSheet(
            "QPushButton {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/adjust_stock-blue.svg);"
            "} "
            );

        ui->promotion_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/promotion-blue.svg);"
            "} "
            );

        ui->activity_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/activity-blue.svg);"
            "} "
            );

        ui->orders_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/orders-blue.svg);"
            "} "
            );

        ui->settings_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/settings-blue.svg);"
            "} "
            );

        ui->supplier_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/supplier-blue.svg);"
            "} "
            );

        ui->product_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/product-blue.svg);"
            "} "
            );

        ui->dashboard_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/dashboard-blue.svg);"
            "} "
            );

        ui->notification_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/notification-blue.svg);"
            "} "
            );

        ui->users_btn->setStyleSheet(
            "QPushButton:hover {"
            "color: rgb(74, 144, 226);"
            "icon: url(:/resources/dashboard-icons/user-blue.svg);"
            "} "
            );

        // Set the current widget to the Adjust Stock section (index 8)
        ui->stackedWidget->setCurrentIndex(8);
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to access Adjust Stock Dashboard");
    }
}
//Product DashBoard
void DashBoard::Get_Total_No_Products(){
    QSqlQuery query;
    if (!query.exec("SELECT SUM(quantity) AS total_products_in_stock FROM products"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_products_in_stock").toInt(); // Get the sum as an integer
        ui->label_Total_Products->setText("Total Products : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->label_Total_Products->setText("0");
    }
}

void DashBoard::Get_Total_Inventory_Value(){
    QSqlQuery query;
    if (!query.exec("SELECT SUM(p.quantity * p.price) AS total_inventory_value FROM products p"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_inventory_value").toInt(); // Get the sum as an integer
        ui->total_inventory_value_label->setText("Total Value : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->total_inventory_value_label->setText("0");
    }
}

void DashBoard::Get_Total_No_Products_OutOfStock(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS total_items FROM products WHERE quantity = 0"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_items").toInt(); // Get the sum as an integer
        ui->out_of_stock_label->setText("Total Value : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->out_of_stock_label->setText("0");
    }
}

void DashBoard::Get_Total_No_Products_Low_Stock(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS total_items FROM products WHERE quantity < 10 AND quantity > 0"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_items").toInt(); // Get the sum as an integer
        //product page
        ui->low_in_stock_label->setText("Total Value : " + QLocale().toString(total));
        //sales page
        ui->Total_Low_Stock_sales->setText("Total Value : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->low_in_stock_label->setText("0");
        ui->Total_Low_Stock_sales->setText("0");
    }
}

void DashBoard::Get_No_Top_Selling_Products(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(DISTINCT p.product_id) AS total_products_with_sales FROM sales s JOIN products p ON s.product_id = p.product_id"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_products_with_sales").toInt(); // Get the sum as an integer
        //product page
        ui->top_selling_products_label->setText("Total Value : " + QLocale().toString(total));
        //sales page
        ui->Top_Selling_No_Products_Sales->setText("Total Value : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->top_selling_products_label->setText("0");
        ui->Top_Selling_No_Products_Sales->setText("0");
    }
}

void DashBoard::Get_No_Top_Selling_Products_value(){
    QSqlQuery query;
    if (!query.exec("SELECT SUM(s.total_price) AS total_revenue FROM sales s JOIN products p ON s.product_id = p.product_id"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_revenue").toInt(); // Get the sum as an integer
        //product
        ui->top_selling_product_value->setText("Total Value : " + QLocale().toString(total));
        //sales
        ui->Top_Selling_value_Products_Sales->setText("Total Value : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->top_selling_product_value->setText("0");
        ui->Top_Selling_value_Products_Sales->setText("0");
    }
}

void DashBoard::Get_Total_No_Sales(){
    QSqlQuery query;
    if (!query.exec("SELECT SUM(quantity_sold) AS total_sales FROM Sales"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_sales").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_sales_quantity_based->setText("Total Sales : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_sales_quantity_based->setText("0");
    }
}

void DashBoard::Get_Total_Sales_Value(){
    QSqlQuery query;
    if (!query.exec("SELECT SUM(quantity_sold * total_price) AS total_sales_value FROM Sales"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_sales_value").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_Sales_Value_sales->setText("Total Value : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_Sales_Value_sales->setText("0");
    }
}

void DashBoard::Get_Total_No_Suppliers(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS No_of_suppliers FROM Suppliers;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("No_of_suppliers").toInt(); // Get the sum as an integer

        //sales page
        ui->Tota_no_supplier_sales->setText("Total Suppliers : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Tota_no_supplier_sales->setText("0");
    }
}
//orders page
void DashBoard::Get_Total_Orders(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS No_of_orders FROM Orders;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("No_of_orders").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_orders_label->setText("Total Orders : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_orders_label->setText("0");
    }
}
void DashBoard::Get_Total_Orders_Cost(){
    QSqlQuery query;
    if (!query.exec("SELECT sum(total_price) AS total_cost FROM Orders;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_cost").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_orders_cost_label->setText("Total Orders Cost : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_orders_cost_label->setText("0");
    }
}
void DashBoard::Get_Total_Completed_Orders(){
    QSqlQuery query;
    if (!query.exec("SELECT count(*) AS total_completed_orders FROM Orders where status is 'Completed';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_completed_orders").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_completed_orders_label->setText("Total Completed Orders : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_completed_orders_label->setText("0");
    }
}
void DashBoard::Get_Total_Pending_Orders(){
    QSqlQuery query;
    if (!query.exec("SELECT count(*) AS total_pending_orders FROM Orders where status is 'Pending';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_pending_orders").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_pending_orders_label->setText("Total Pending Orders : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_pending_orders_label->setText("0");
    }
}
void DashBoard::Get_Total_Cancelled_Orders(){
    QSqlQuery query;
    if (!query.exec("SELECT count(*) AS total_cancelled_orders FROM Orders where status is 'Cancelled';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_cancelled_orders").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_cancelled_orders_label->setText("Total Cancelled Orders : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_cancelled_orders_label->setText("0");
    }
}
void DashBoard::Get_Total_Cancelled_Orders_Cost(){
    QSqlQuery query;
    if (!query.exec("SELECT sum(total_price) AS total_cancelled_cost_orders FROM Orders where status is 'Cancelled';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_cancelled_cost_orders").toInt(); // Get the sum as an integer

        //sales page
        ui->Total_cancelled_orders_cost_label->setText("Total Cancelled Orders Cost : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->Total_cancelled_orders_cost_label->setText("0");
    }
}

void DashBoard::on_add_product_btn_clicked()
{
    if (GlobalFunctions::hasPermission("add_product")) {
        add_product = new AddProduct(nullptr);
        add_product->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Add Product");
    }
}


void DashBoard::on_Update_product_btn_clicked()
{
    if (GlobalFunctions::hasPermission("update_products")) {
        Update_Product = new UpdateProduct(nullptr);
        Update_Product->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Update Product");
    }
}


void DashBoard::on_Delete_Product_btn_clicked()
{
    if (GlobalFunctions::hasPermission("delete_products")) {
        Delete_Product = new DeleteProduct(nullptr);
        Delete_Product->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Delete Product");
    }
}

void DashBoard::on_add_sales_btn_clicked()
{
    if (GlobalFunctions::hasPermission("add_sales")) {
        Add_Sales = new AddSales(nullptr);
        Add_Sales->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Add Sales");
    }
}


void DashBoard::on_update_sales_btn_clicked()
{
    if (GlobalFunctions::hasPermission("update_sales")) {
        Update_Sales = new UpdateSales(nullptr);
        Update_Sales->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Update Sales");
    }
}


void DashBoard::on_delete_sales_btn_clicked()
{
    if (GlobalFunctions::hasPermission("delete_sales")) {
        Delete_Sales = new DeleteSales(nullptr);
        Delete_Sales->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Delete Sales");
    }
}


void DashBoard::on_addorders_btn_clicked()
{
    if (GlobalFunctions::hasPermission("add_orders")) {
        addorders = new AddOrders(nullptr);
        addorders->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Add Orders");
    }
}


void DashBoard::Load_manage_actvity(){
    ManageActivity *manage_activity = new ManageActivity;
    ui->manage_activity_layout->addWidget(manage_activity);
}

void DashBoard::on_update_orders_btn_clicked()
{
    if (GlobalFunctions::hasPermission("update_orders")) {
        Update_Orders= new UpdateOrders(nullptr);
        Update_Orders->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Update Orders");
    }
}


void DashBoard::on_delete_orders_btn_clicked()
{
    if (GlobalFunctions::hasPermission("delete_orders")) {
        Delete_Orders = new DeleteOrders(nullptr);
        Delete_Orders->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Delete Orders");
    }
}


void DashBoard::on_add_emp_btn_clicked()
{
    if (GlobalFunctions::hasPermission("add_employees")) {
        Add_Employee = new AddEmployee(nullptr);
        Add_Employee->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Add Employee");
    }
}

void DashBoard::Get_No_Login(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS total_logins FROM audit_logs WHERE action = 'User Login';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_logins").toInt(); // Get the sum as an integer

        //sales page
        ui->No_Login_Label->setText("Total Login : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->No_Login_Label->setText("0");
    }
}

void DashBoard::Get_No_Logout(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS total_logout FROM audit_logs WHERE action = 'User Logout';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_logout").toInt(); // Get the sum as an integer

        //sales page
        ui->No_Logout_Label->setText("Total Logout : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->No_Logout_Label->setText("0");
    }
}

void DashBoard::Get_Total_No_Employee(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS total_employee_count FROM employees;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_employee_count").toInt(); // Get the sum as an integer

        //sales page
        ui->total_no_employee_label->setText("Total Employees : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->total_no_employee_label->setText("0");
    }
}
void DashBoard::Get_Total_Active_Employee(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS total_active_employee_count FROM employees WHERE status='active';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_active_employee_count").toInt(); // Get the sum as an integer

        //sales page
        ui->total_no_active_employee_label->setText("Total Active Employees : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->total_no_active_employee_label->setText("0");
    }
}
void DashBoard::Get_Total_Inactive_Employee(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS total_inactive_employee_count FROM employees WHERE status='inactive';"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("total_inactive_employee_count").toInt(); // Get the sum as an integer

        //sales page
        ui->total_no_inactive_employee_label->setText("Total Inactive Employees : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->total_no_inactive_employee_label->setText("0");
    }
}
void DashBoard::Get_Average_salary_employee(){
    QSqlQuery query;
    if (!query.exec("SELECT AVG(salary) AS average_salary FROM employees;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("average_salary").toInt(); // Get the sum as an integer

        //sales page
        ui->average_salary_employee_label->setText("Average Salary : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->average_salary_employee_label->setText("0");
    }
}




void DashBoard::on_add_promotion_btn_clicked()
{
    if (GlobalFunctions::hasPermission("add_promotion")) {
        Add_Promotion = new AddPromotion(nullptr);
        Add_Promotion->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Add Promotion");
    }
}

void DashBoard::Get_Active_Promotions(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) AS Active_Promotions FROM promotions WHERE start_date <= CURRENT_DATE AND end_date >= CURRENT_DATE;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("Active_Promotions").toInt(); // Get the sum as an integer

        //sales page
        ui->active_promotion_label->setText("Active Promotions : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->active_promotion_label->setText("0");
    }
}
void DashBoard::Get_Upcoming_Promotions(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) as Upcoming_Promotions FROM promotions WHERE start_date > CURRENT_DATE;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("Upcoming_Promotions").toInt(); // Get the sum as an integer

        //sales page
        ui->upcoming_promotion_label->setText("Upcoming Promotions : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->upcoming_promotion_label->setText("0");
    }
}
void DashBoard::Get_Expired_Promotions(){
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) as Expired_Promotions FROM promotions WHERE end_date < CURRENT_DATE;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        int total = query.value("Expired_Promotions").toInt(); // Get the sum as an integer

        //sales page
        ui->expired_promotion_label->setText("Expired Promotions : " + QLocale().toString(total));
    }
    else
    {
        // If no result is returned
        ui->expired_promotion_label->setText("0");
    }
}
void DashBoard::Get_Discount_Highlights(){
    QSqlQuery query;
    if (!query.exec("SELECT CONCAT(name, ' - ', discount_percentage, '%') AS discount_highlight "
                    "FROM promotions WHERE start_date <= CURRENT_DATE AND end_date >= CURRENT_DATE "
                    "ORDER BY discount_percentage DESC LIMIT 1;"))
    {
        // Step 2: If the query execution fails, show the error
        QMessageBox::critical(this, "Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Step 3: Retrieve the result
    if (query.next())  // Move to the first (and only) row in the result
    {
        QString total = query.value("discount_highlight").toString(); // Get the concatenated string

        // Set the result in the label
        ui->disocunt_highlight_promotion_label->setText("Highest Discount Promotion: " + total);
    }
    else
    {
        // If no result is returned
        ui->disocunt_highlight_promotion_label->setText("No active promotions");
    }
}

void DashBoard::on_update_promotion_btn_clicked()
{
    if (GlobalFunctions::hasPermission("update_promotion")) {
        Update_Promotion = new UpdatePromotion(nullptr);
        Update_Promotion->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Update Promotion");
    }
}


void DashBoard::on_delete_promotion_btn_clicked()
{
    if (GlobalFunctions::hasPermission("delete_promotion")) {
        Delete_Promotion = new DeletePromotion(nullptr);
        Delete_Promotion->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Delete Promotion");
    }
}


void DashBoard::on_update_employee_btn_clicked()
{
    if (GlobalFunctions::hasPermission("update_employees")) {
        Update_Employee = new UpdateEmployee(nullptr);
        Update_Employee->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Update Employee");
    }
}


void DashBoard::on_delete_employee_btn_clicked()
{
    if (GlobalFunctions::hasPermission("delete_employees")) {
        Delete_Employee = new DeleteEmployee(nullptr);
        Delete_Employee->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Delete Employees");
    }
}


void DashBoard::on_update_permission_btn_clicked()
{
    if (GlobalFunctions::hasPermission("update_employees")) {
        Update_Permissions = new UpdatePermissions(nullptr);
        Update_Permissions->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to Update Permissions");
    }
}

//charts
void DashBoard::Sales_Trends_loader(){
    SalesTrends_chart *Sales_Trends_chart = new SalesTrends_chart;
    ui->Sales_Trends_layout->addWidget(Sales_Trends_chart);
}

void DashBoard::Products_Hightlights_Min_Level_Chart_Loader(){
    Products_Hightlights_Min_Level_Chart *Products_Hight_lights_Min_Level=new Products_Hightlights_Min_Level_Chart;
    ui->product_min_level->addWidget(Products_Hight_lights_Min_Level);
}

void DashBoard::Orders_Trends_Loader(){
    Orders_Trends *Orders_Trends_chart=new Orders_Trends;
    ui->order_trends_layout->addWidget(Orders_Trends_chart);
}




void DashBoard::on_add_adjustments_btn_clicked()
{
    if (GlobalFunctions::hasPermission("adjustment_Stock")) {
        Add_Adjustment = new AddAdjustment(nullptr);
        Add_Adjustment->show();
    } else {
        // Deny access
        QMessageBox::warning(this, "Permission Denied", "You do not have permission to add Adjustment");
    }
}

void DashBoard::fetchEmployeeInfo(int employeeId){
    if(GlobalFunctions::is_admin()){
        // Prepare SQL query to fetch user details by user ID
        QString queryStr = R"(
        SELECT id, business_name, address, email, pin, name, username,
               role, last_login, failed_login_attempts, account_locked,
               lockout_time, password_reset_token, password_reset_expiry, two_factor_enabled,
               created_by, updated_by, deleted_at, language_preference,
               theme_preference, recovery_email, recovery_phone,
               last_known_ip, last_known_location, email_verified,
               phone_verified, security_questions
        FROM users
        WHERE id = :userId
    )";

        QSqlQuery query;
        query.prepare(queryStr);
        query.bindValue(":userId", employeeId);

        // Execute the query
        if (!query.exec()) {
            qDebug() << "Query execution failed:" << query.lastError().text();
            return;
        }

        // Check if the query returned any result
        if (!query.next()) {
            qDebug() << "No user found with ID:" << employeeId;
            return;
        }

        // Retrieve the data from the query result
        int id = query.value("id").toInt();
        QString businessName = query.value("business_name").toString();
        QString address = query.value("address").toString();
        QString email = query.value("email").toString();
        int pin = query.value("pin").toInt();
        QString name = query.value("name").toString();
        QString username = query.value("username").toString();
        QString role = query.value("role").toString();
        QString lastLogin = query.value("last_login").toString();
        int failedLoginAttempts = query.value("failed_login_attempts").toInt();
        bool accountLocked = query.value("account_locked").toBool();
        QString lockoutTime = query.value("lockout_time").toString();
        QString passwordResetToken = query.value("password_reset_token").toString();
        bool twoFactorEnabled = query.value("two_factor_enabled").toBool();
        QString createdBy = query.value("created_by").toString();
        QString updatedBy = query.value("updated_by").toString();
        QString deletedAt = query.value("deleted_at").toString();
        QString languagePreference = query.value("language_preference").toString();
        QString themePreference = query.value("theme_preference").toString();
        QString recoveryEmail = query.value("recovery_email").toString();
        QString recoveryPhone = query.value("recovery_phone").toString();
        QString lastKnownIp = query.value("last_known_ip").toString();
        QString lastKnownLocation = query.value("last_known_location").toString();
        bool emailVerified = query.value("email_verified").toBool();
        bool phoneVerified = query.value("phone_verified").toBool();
        QString securityQuestions = query.value("security_questions").toString();

        // Debug output (to verify the fetched data)
        qDebug() << "----- User Profile -----";
        qDebug() << "ID:" << id;
        qDebug() << "Business Name:" << businessName;
        qDebug() << "Address:" << address;
        qDebug() << "Email:" << email;
        qDebug() << "PIN:" << pin;
        qDebug() << "Name:" << name;
        qDebug() << "Username:" << username;
        qDebug() << "Role:" << role;
        qDebug() << "Last Login:" << lastLogin;
        qDebug() << "Failed Login Attempts:" << failedLoginAttempts;
        qDebug() << "Account Locked:" << (accountLocked ? "Yes" : "No");
        qDebug() << "Lockout Time:" << lockoutTime;
        qDebug() << "Password Reset Token:" << passwordResetToken;
        qDebug() << "Two Factor Enabled:" << (twoFactorEnabled ? "Yes" : "No");
        qDebug() << "Created By:" << createdBy;
        qDebug() << "Updated By:" << updatedBy;
        qDebug() << "Deleted At:" << deletedAt;
        qDebug() << "Language Preference:" << languagePreference;
        qDebug() << "Theme Preference:" << themePreference;
        qDebug() << "Recovery Email:" << recoveryEmail;
        qDebug() << "Recovery Phone:" << recoveryPhone;
        qDebug() << "Last Known IP:" << lastKnownIp;
        qDebug() << "Last Known Location:" << lastKnownLocation;
        qDebug() << "Email Verified:" << (emailVerified ? "Yes" : "No");
        qDebug() << "Phone Verified:" << (phoneVerified ? "Yes" : "No");
        qDebug() << "Security Questions:" << securityQuestions;

        // Assuming `profile_image_url` is available from elsewhere, e.g., query or set statically
        QString profileImageUrl = "path/to/profile/image";  // Replace with actual profile image URL/path
        QPixmap profileImg(profileImageUrl);

        // Update UI labels and elements
        ui->profile_image_label->setPixmap(profileImg);
        ui->name_main_profile_label->setText(name);
        ui->role_profile_label->setText(role);
        ui->email_profile_label->setText(email);

        // Basic Information
        ui->id_profile_label->setText(QString::number(id));
        ui->Name_profile_label->setText(name);
        ui->address_profile_label->setText(address);
        ui->email_profile_label->setText(email);

        // Employment Information
        ui->role2_profile_label->setText(role);
        ui->hireDate_profile_label->setText(lastLogin);  // Placeholder for actual hire date
        ui->status_profile_label->setText(accountLocked ? "Locked" : "Active");

        // Contact Info
        ui->ph_no_profile_label->setText(recoveryPhone);
        ui->ph_no_profile_label->setText(emailVerified ? "Verified" : "Not Verified");
        ui->label_gender_or_email->setText("Email ");
        ui->gender_no_profile_label->setText(phoneVerified ? "Verified" : "Not Verified");

        // Hide extra or irrelevant labels based on fetched data
        //ui->lockout_time_profile_label->setVisible(accountLocked); // Only show lockout time if account is locked
        if(failedLoginAttempts <= 0){
            ui->label_Department_or_failed_attempt->hide();
            ui->Deapprtment_profile_label->hide();
        }else{
            ui->label_Department_or_failed_attempt->setText("Failed Login Attempts ");
            ui->Deapprtment_profile_label->setVisible(failedLoginAttempts > 0);
        }

        ui->label_emp_type->hide();
        ui->emp_type_no_profile_label->hide();

        if(!GlobalFunctions::business_logo_path.isEmpty()){
            QPixmap profile_img_url(GlobalFunctions::business_logo_path);
            ui->profile_image_label->setPixmap(profile_img_url);
        }

        ui->tabWidget->setTabVisible(1,false);
        ui->tabWidget->setTabVisible(2,false);
    }else{
        QString queryStr = R"(
            SELECT employee_id, name, email, phone_number AS contact_number, gender,
                   department, role, employee_type AS employment_type,
                   hire_date AS joining_date, status, last_login,
                   updated_at AS last_updated, salary, bonus,
                   address, profile_picture
            FROM employees
            WHERE employee_id = :employeeId
        )";

        QSqlQuery query;
        query.prepare(queryStr);
        query.bindValue(":employeeId", employeeId);

        // Step 3: Execute the Query
        if (!query.exec()) {
            qDebug() << "Query execution failed:" << query.lastError().text();
            return;
        }

        // Step 4: Process the Result
        if (query.next()) {
            int id = query.value("employee_id").toInt();
            QString name = query.value("name").toString();
            QString email = query.value("email").toString();
            QString contactNumber = query.value("contact_number").toString();
            QString gender = query.value("gender").toString();
            QString department = query.value("department").toString();
            QString role = query.value("role").toString();
            QString employmentType = query.value("employment_type").toString();
            QString joiningDate = query.value("joining_date").toString();
            QString status = query.value("status").toString();
            QString lastLogin = query.value("last_login").toString();
            QString lastUpdated = query.value("last_updated").toString();
            double salary = query.value("salary").toDouble();
            double bonus = query.value("bonus").toDouble();
            QString address = query.value("address").toString();
            QString profile_image_url = query.value("profile_picture").toString();

            //updating ui
            if(!profile_image_url.isEmpty()){
                QPixmap profile_img_url(profile_image_url);
                ui->profile_image_label->setPixmap(profile_img_url);
            }

            ui->name_main_profile_label->setText(name);
            ui->role_profile_label->setText(role);
            ui->email_profile_label->setText(email);
            ui->status_profile_label->setText(status);
            //Basic Information
            ui->id_profile_label->setText(QString::number(id));
            ui->Name_profile_label->setText(name);
            ui->ph_no_profile_label->setText(contactNumber);
            ui->gender_no_profile_label->setText(gender);
            ui->Deapprtment_profile_label->setText(department);
            ui->role2_profile_label->setText(role);
            ui->hireDate_profile_label->setText(joiningDate);
            ui->emp_type_no_profile_label->setText(employmentType);
            ui->address_profile_label->setText(address);
            //Employment Details
            ui->hire_date_profile_label->setText(joiningDate);
            ui->status_profile_label_2->setText(status);
            ui->last_login_profile_label->setText(lastLogin);
            ui->last_updated_profile_label->setText(lastUpdated);
            //Financial Information
            ui->salary_profile_label->setText(QString::number(salary));
            ui->bonus_profile_label->setText(QString::number(bonus));
            // Display the Employee Profile Information
            qDebug() << "----- Employee Profile -----";
            qDebug() << "Employee ID:" << id;
            qDebug() << "Name:" << name;
            qDebug() << "Email:" << email;
            qDebug() << "Contact Number:" << contactNumber;
            qDebug() << "Gender:" << gender;
            qDebug() << "Department:" << department;
            qDebug() << "Role/Position:" << role;
            qDebug() << "Employment Type:" << employmentType;
            qDebug() << "Joining Date:" << joiningDate;
            qDebug() << "Status:" << status;
            qDebug() << "Last Login:" << lastLogin;
            qDebug() << "Last Updated:" << lastUpdated;
            qDebug() << "Salary:" << salary;
            qDebug() << "Bonus:" << bonus;
            qDebug() << "Address:" << address;
            qDebug() << "image url:" << profile_image_url;
        } else {
            qDebug() << "No employee found with ID:" << employeeId;
        }
    }

}

void DashBoard::on_logout_btn_clicked()
{
    QCoreApplication::quit();
}

void DashBoard::insertStartupNotifications()
{
    QSqlQuery query, insert;

    auto addNotification = [&](const QString &message, const QString &type, const QString &key) {
        QSqlQuery check;
        check.prepare("SELECT COUNT(*) FROM notifications WHERE unique_key = ?");
        check.addBindValue(key);
        if (check.exec() && check.next() && check.value(0).toInt() == 0) {
            insert.prepare("INSERT INTO notifications (message, type, unique_key, auto_generated) VALUES (?, ?, ?, 1)");
            insert.addBindValue(message);
            insert.addBindValue(type);
            insert.addBindValue(key);
            insert.exec();
        }
    };

    // PRODUCTS
    query.exec("SELECT COUNT(*) FROM products WHERE quantity < minimum_stock_level");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("%1 products are below minimum stock level.").arg(query.value(0).toInt()), "risk", "low-stock-warning");

    query.exec("SELECT COUNT(*) FROM products WHERE quantity = 0");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("%1 products are completely out of stock.").arg(query.value(0).toInt()), "risk", "stock-zero");

    query.exec("SELECT COUNT(*) FROM products WHERE quantity < 0");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some products have negative stock. Please check inventory!", "risk", "negative-stock");

    query.exec("SELECT COUNT(*) FROM products WHERE supplier_id IS NULL OR supplier_id = 0");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some products are not linked to any supplier.", "info", "missing-supplier");

    query.exec("SELECT COUNT(*) FROM products WHERE julianday('now') - julianday(purchase_date) > 30");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some products haven't been updated in over 30 days.", "info", "product-stale");

    // SALES
    query.exec("SELECT COUNT(*) FROM sales WHERE sale_date = date('now')");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("%1 sales recorded today.").arg(query.value(0).toInt()), "info", "sales-today");

    query.exec("SELECT COUNT(*) FROM sales WHERE sale_date >= date('now', '-5 days')");
    if (query.next() && query.value(0).toInt() == 0)
        addNotification("No sales activity in the past 5 days.", "info", "sales-inactive");

    // ORDERS
    query.exec("SELECT COUNT(*) FROM orders WHERE status = 'Pending'");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("%1 customer orders are still pending.").arg(query.value(0).toInt()), "info", "orders-pending");

    query.exec("SELECT COUNT(*) FROM orders WHERE status = 'Pending' AND total_price > 10000");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("High-value pending orders exceeding ₹10,000 exist.", "info", "high-value-orders");

    query.exec("SELECT COUNT(*) FROM orders WHERE status = 'Pending' AND julianday('now') - julianday(order_date) > 7");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some orders are pending for more than 7 days!", "risk", "orders-delayed");

    // CUSTOMERS
    query.exec("SELECT COUNT(*) FROM customers WHERE contact_info IS NULL OR contact_info = ''");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some customers are missing contact details.", "info", "missing-customer-contact");

    query.exec("SELECT COUNT(*) FROM customers WHERE date('now') = (SELECT date('now'))");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("%1 new customers added today.").arg(query.value(0).toInt()), "info", "new-customers-today");

    // SUPPLIERS
    query.exec("SELECT COUNT(*) FROM suppliers WHERE supplier_id NOT IN (SELECT DISTINCT supplier_id FROM products)");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some suppliers are not linked to any products.", "info", "suppliers-unused");

    // EMPLOYEES
    query.exec("SELECT COUNT(*) FROM employees WHERE status = 'terminated' AND is_active = 1");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some terminated employees are still marked active.", "risk", "terminated-active-employees");

    query.exec("SELECT COUNT(*) FROM employees WHERE department IS NULL OR department = ''");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some employees do not have a department assigned.", "info", "missing-department");

    query.exec("SELECT COUNT(*) FROM employees WHERE email IS NULL OR role IS NULL");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some employees have missing essential details.", "risk", "employee-missing-fields");

    // USERS
    query.exec("SELECT COUNT(*) FROM users WHERE failed_login_attempts >= 5");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some user accounts have repeated failed login attempts.", "risk", "user-lock-warnings");

    query.exec("SELECT COUNT(*) FROM users WHERE role IS NULL OR role = ''");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some users have no role assigned.", "risk", "user-missing-role");

    // PERMISSIONS
    query.exec("SELECT COUNT(*) FROM permissions WHERE product_dashboard = 1 AND delete_products = 1 AND advance_view_products = 0");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Users with delete rights but no view rights detected. Check permissions.", "risk", "bad-permission-combo");

    // NOTIFICATIONS
    query.exec("SELECT COUNT(*) FROM notifications WHERE is_read = 0");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("You have %1 unread notifications.").arg(query.value(0).toInt()), "info", "unread-notification-count");

    // PROMOTIONS
    query.exec("SELECT COUNT(*) FROM promotions WHERE start_date = date('now')");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some promotions start today.", "info", "promotions-starting");

    query.exec("SELECT COUNT(*) FROM promotions WHERE end_date = date('now')");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some promotions are ending today.", "info", "promotions-ending");

    query.exec("SELECT COUNT(*) FROM promotions");
    if (query.next() && query.value(0).toInt() == 0)
        addNotification("No promotions are currently running.", "info", "no-promotions");

    // INVENTORY ADJUSTMENTS
    query.exec("SELECT COUNT(*) FROM inventory_adjustments WHERE adjustment_date = date('now')");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("Inventory adjusted today: %1 changes").arg(query.value(0).toInt()), "info", "adjustments-today");

    query.exec("SELECT COUNT(*) FROM inventory_adjustments WHERE quantity > 100");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification("Some inventory adjustments exceed 100 units.", "risk", "adjustments-large");

    // AUDIT LOGS
    query.exec("SELECT COUNT(*) FROM audit_logs WHERE timestamp >= datetime('now', '-1 day')");
    if (query.next() && query.value(0).toInt() > 0)
        addNotification(QString("%1 user actions logged in last 24 hours.").arg(query.value(0).toInt()), "info", "audit-activity");

    // Load all notifications into UI
    loadDefaultNotifications();
}

void DashBoard::loadDefaultNotifications()
{
    QSqlQuery query;
    query.prepare("SELECT message, created_at, type FROM notifications ORDER BY created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            QString message = query.value("message").toString();
            QString createdAt = query.value("created_at").toString();
            QString type = query.value("type").toString();

            // Create notification UI object
            Notification *notification = new Notification("Notification", message, type, this);
            ui->scrollArea_notification_store->addWidget(notification);
            notifications.append(notification);
        }
    } else {
        qDebug() << "Error loading notifications:" << query.lastError().text();
    }
}

