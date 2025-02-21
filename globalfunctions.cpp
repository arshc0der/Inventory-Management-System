#include "globalfunctions.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QByteArray>
#include <QCryptographicHash>

QString GlobalFunctions::username = ""; // Initialize static member
int GlobalFunctions::user_ID = 0; // Initialize static member
bool GlobalFunctions::admin=false;

QString GlobalFunctions::business_name="";
QString GlobalFunctions::business_logo_path="";

GlobalFunctions::GlobalFunctions() {}

void GlobalFunctions::set_user(const QString &name, const int &id) {
    username = name; // Set the static member
    user_ID = id;
    get_business_info(business_name,business_logo_path);
    // Load the permissions for this user
    if (loadPermissions()) {
        qDebug() << "Permissions loaded successfully for user:" << username;
    } else {
        qDebug() << "Failed to load permissions for user:" << username;
    }
}

QString GlobalFunctions::get_username() {
    return username; // Return the static member
}

int GlobalFunctions::get_user_id() {
    return user_ID; // Return the static member
}
void GlobalFunctions::set_admin(){
    admin=true;
}
bool GlobalFunctions::is_admin(){
    return admin;
}

void GlobalFunctions::get_business_info(QString &business_name,QString &business_logo_path) {
    // Create a query to fetch business name and logo URL from the table
    QSqlQuery query;
    query.prepare("SELECT business_name, logo_url FROM business_logos LIMIT 1");

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error fetching business info:" << query.lastError().text();
        return;  // Exit if the query fails
    }

    // Check if the query has returned any rows
    if (query.next()) {
        // Extract the business name and logo URL from the query result
        QString businessName = query.value("business_name").toString();
        QString logoUrl = query.value("logo_url").toString();

        // Debug output to see the fetched values
        qDebug() << "Business Name:" << businessName;
        qDebug() << "Logo URL:" << logoUrl;

        business_name = businessName;
        business_logo_path = logoUrl;
    } else {
        qDebug() << "No business info found.";
    }
}

// Permissions
QMap<QString, bool> GlobalFunctions::permissions;  // Initialize permissions map

// Load permissions for the user from the database
bool GlobalFunctions::loadPermissions() {
    // Clear the previous permissions in case this is called again
    permissions.clear();

    QSqlQuery query;
    query.prepare("SELECT * FROM permissions WHERE user_id = ?");
    query.addBindValue(user_ID);  // Use the static user ID

    if (query.exec()) {
        if (query.next()) {
            // Map each permission from the table to the permissions map
            permissions["product_dashboard"] = query.value("product_dashboard").toBool();
            permissions["add_product"] = query.value("add_product").toBool();
            permissions["update_products"] = query.value("update_products").toBool();
            permissions["delete_products"] = query.value("delete_products").toBool();
            permissions["advance_view_products"] = query.value("advance_view_products").toBool();
            permissions["adjustment_Stock"] = query.value("adjustment_Stock").toBool();

            permissions["sales_dashboard"] = query.value("sales_dashboard").toBool();
            permissions["add_sales"] = query.value("add_sales").toBool();
            permissions["update_sales"] = query.value("update_sales").toBool();
            permissions["delete_sales"] = query.value("delete_sales").toBool();
            permissions["advance_view_sales"] = query.value("advance_view_sales").toBool();

            permissions["orders_dashboard"] = query.value("orders_dashboard").toBool();
            permissions["add_orders"] = query.value("add_orders").toBool();
            permissions["update_orders"] = query.value("update_orders").toBool();
            permissions["delete_orders"] = query.value("delete_orders").toBool();
            permissions["advance_view_orders"] = query.value("advance_view_orders").toBool();

            permissions["activity_dashboard"] = query.value("activity_dashboard").toBool();
            permissions["notification_dashboard"] = query.value("notification_dashboard").toBool();

            permissions["promotion_dashboard"] = query.value("promotion_dashboard").toBool();
            permissions["add_promotion"] = query.value("add_promotion").toBool();
            permissions["update_promotion"] = query.value("update_promotion").toBool();
            permissions["delete_promotion"] = query.value("delete_promotion").toBool();
            permissions["advance_view_promotion"] = query.value("advance_view_promotion").toBool();

            permissions["user_dashboard"] = query.value("user_dashboard").toBool();
            permissions["add_employees"] = query.value("add_employees").toBool();
            permissions["update_employees"] = query.value("update_employees").toBool();
            permissions["delete_employees"] = query.value("delete_employees").toBool();
            permissions["advance_view_employees"] = query.value("advance_view_employees").toBool();
            permissions["settings_dashboard"] = query.value("settings_dashboard").toBool();

            return true; // Successfully loaded permissions
        } else {
            qDebug() << "No permissions found for user ID" << user_ID;
            return false; // No permissions found for the user
        }
    } else {
        qDebug() << "Error loading permissions:" << query.lastError();
        return false; // Query failed
    }
}

// Check if the user has a specific permission
bool GlobalFunctions::hasPermission(const QString &permission) {
    if (permissions.contains(permission)) {
        return permissions[permission];
    }
    return false; // Default to false if permission is not found
}

// General function to log any action
void GlobalFunctions::audit_logs(const QString &action, const QString &details) {
    int userId = get_user_id();
    QString username = get_username();

    QSqlQuery logQuery;
    logQuery.prepare(R"(
        INSERT INTO audit_logs (user_id, action, details)
        VALUES (:user_id, :action, :details)
    )");
    logQuery.bindValue(":user_id", userId);
    logQuery.bindValue(":action", action);
    logQuery.bindValue(":details", details);

    if (!logQuery.exec()) {
        qDebug() << "Error inserting into audit_logs:" << logQuery.lastError().text();
    } else {
        qDebug() << "Audit log entry created successfully for user:" << username;
    }
}

void GlobalFunctions::log_add_product() {
    QString action = "Add Product";
    QString details = QString("Product added by user %1").arg(username);
    audit_logs(action, details);
}

void GlobalFunctions::log_add_sale() {
    QString action = "Add Sale";
    QString details = QString("Sale added by user %1").arg(username);
    audit_logs(action, details);
}

void GlobalFunctions::log_update_record() {
    QString action = "Update Record";
    QString details = QString("Log updated by user %1").arg(username);
    audit_logs(action, details);
}

void GlobalFunctions::log_delete_record() {
    QString action = "Delete Record";
    QString details = QString("Log deleted by user %1").arg(username);
    audit_logs(action, details);
}

void GlobalFunctions::log_user_login() {
    QString action = "User Login";
    QString details = QString("User %1 logged in successfully").arg(username);
    audit_logs(action, details);
}

void GlobalFunctions::log_user_logout() {
    QString action = "User Logout";
    QString details = QString("User %1 logged out successfully").arg(username);
    audit_logs(action, details);
}

QString GlobalFunctions::hashPasswordWithSalt(const QString &password, const QString &salt) {
    // Combine password and salt, then hash using SHA-256
    QByteArray combined = password.toUtf8() + salt.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(combined, QCryptographicHash::Sha256);
    return hashedPassword.toHex();  // Return hashed password as a hex string
}

bool GlobalFunctions::verifyPassword(const QString &enteredPassword, const QString &storedHash, const QString &storedSalt) {
    // Hash the entered password with the stored salt
    QString hashedEnteredPassword = hashPasswordWithSalt(enteredPassword, storedSalt);

    // Compare the entered password hash with the stored hash
    return hashedEnteredPassword == storedHash;
}
