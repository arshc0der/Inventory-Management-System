#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>

class GlobalFunctions
{
private:
    static QString username;           // Static member to store username
    static int user_ID;                // Static member to store user ID
    static QMap<QString, bool> permissions;  // Store user permissions
    static QString hashPasswordWithSalt(const QString &password, const QString &salt);
    static bool admin;

public:
    GlobalFunctions();

    static QString business_name;
    static QString business_logo_path;

    // Declare static methods
    static bool verifyPassword(const QString &enteredPassword, const QString &storedHash, const QString &storedSalt);
    static void set_user(const QString &name, const int &id);
    static QString get_username(); // Getter for username
    static int get_user_id();      // Getter for user ID
    static void audit_logs(const QString &action, const QString &details);
    static void set_admin();
    static bool is_admin();
    static void get_business_info(QString &business_name,QString &business_logo_path);

    static void log_user_login();
    static void log_user_logout();
    static void log_add_product();
    static void log_add_sale();
    static void log_update_record();
    static void log_delete_record();

    // New static function to load user permissions
    static bool loadPermissions();
    static bool hasPermission(const QString &permission); // Check if user has specific permission
};

#endif // GLOBALFUNCTIONS_H
