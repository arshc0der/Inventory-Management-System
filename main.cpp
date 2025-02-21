#include "mainwindow.h"
#include "DatabaseManager.h"  // Singleton database connection
#include "DatabaseSchema.h"    // Database schema management

#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QDebug>

#include <QSplashScreen>
#include <QThread>
#include <QPixmap>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create and show the splash screen
    QPixmap pixmap(":/resources/splash_screen_images/splash_image.png");  // image path

    // Scale the image to a smaller size (for example, 50% of its original size)
    QSize newSize(pixmap.width() / 1.1, pixmap.height() / 1.1);  // Resize to 50% of original size
    QPixmap scaledPixmap = pixmap.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QSplashScreen *splash = new QSplashScreen(scaledPixmap);
    splash->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
    splash->show();

    splash->showMessage("Preparing Database...");
    // Initialize the database connection
    DatabaseManager &dbManager = DatabaseManager::instance();
    if (!dbManager.getDatabase().isOpen()) {
        QMessageBox::critical(nullptr, "Database Error", "Failed to connect to the database!");
        splash->close();  // Close splash screen before exiting
        return 1;  // Exit the application
    }

    // Initialize database schema (create tables if needed)
    DatabaseSchema::initializeDatabase();

    // Check if the 'users' table exists (optional validation)
    QSqlQuery query;
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='users';")) {
        qDebug() << "Error checking for table existence:" << query.lastError().text();
        splash->close();  // Close splash screen before exiting
        return 1;
    }
    query.next();
    if (query.size() == 0) {
        qDebug() << "'users' table does not exist!";
        splash->close();  // Close splash screen before exiting
        return 1;
    }

    QThread::sleep(2);  // Simulate some initialization delay (e.g., 2 seconds)
    // Now that everything is ready, close the splash screen
    splash->close();

    // Show the main window
    MainWindow w;
    w.show();

    return a.exec();
}
