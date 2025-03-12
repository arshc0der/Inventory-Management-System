#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>

class DatabaseManager {
public:
    // Public method to get the singleton instance
    static DatabaseManager& instance() {
        static DatabaseManager instance;
        return instance;
    }

    // Method to get the database connection
    QSqlDatabase getDatabase() const {
        return db;
    }

    // Method to close the database connection
    void closeConnection() {
        if (db.isOpen()) {
            db.close();
        }
    }

private:
    QSqlDatabase db;

    // Private constructor for singleton pattern
    DatabaseManager() {
        db = QSqlDatabase::addDatabase("QSQLITE");
        
        db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/i_m_s.db");

        if (!db.open()) {
            qDebug() << "Error opening database: " << db.lastError().text();
        } else {
            qDebug() << "Database connected successfully!";
        }
    }

    // Delete copy constructor and assignment operator
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

#endif // DATABASEMANAGER_H
