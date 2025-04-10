#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "DatabaseManager.h"  // Singleton database connection
#include <QCloseEvent>
#include <QThread>
#include <globalfunctions.h>
#include <QDateTime>
//animation
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

bool admin = false;
bool employee=true;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->toggle_button_admin_circle->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::Checked) {
        ui->pass_lineEdit->setEchoMode(QLineEdit::Normal);
    } else {
        ui->pass_lineEdit->setEchoMode(QLineEdit::Password);
    }
}

void MainWindow::on_pushButton_clicked()
{
    reg = new Register(this);
    reg->show();
}

void MainWindow::on_login_btn_clicked()
{
    if(admin==true && employee==false){

        GlobalFunctions::set_admin();//setting admin
        QString username = ui->username_lineEdit->text();
        QString password = ui->pass_lineEdit->text();

        QSqlQuery query;
        query.prepare("SELECT id, username, password_hash, password_salt, account_locked, lockout_time, failed_login_attempts, is_active "
                      "FROM users WHERE username = :username LIMIT 1");
        query.bindValue(":username", username);

        if (query.exec()) {
            if (query.next()) {  // If a result is found
                int userId = query.value("id").toInt();
                QString storedPasswordHash = query.value("password_hash").toString();
                QString storedPasswordSalt = query.value("password_salt").toString();
                bool accountLocked = query.value("account_locked").toBool();
                QDateTime lockoutTime = query.value("lockout_time").toDateTime();
                int failedLoginAttempts = query.value("failed_login_attempts").toInt();
                bool isActive = query.value("is_active").toBool();

                // Check if the account is active
                if (!isActive) {
                    qDebug() << "Account is deactivated.";
                    QMessageBox::warning(this, "Account Inactive", "Your account has been deactivated. Please contact support.");
                    return;
                }

                // Check if the account is locked
                if (accountLocked) {
                    QDateTime currentTime = QDateTime::currentDateTime();
                    if (lockoutTime.isValid() && lockoutTime.addSecs(300) > currentTime) {  // 5-minute lockout
                        qDebug() << "Account is locked. Lockout will expire at:" << lockoutTime.addSecs(300).toString();
                        QMessageBox::warning(this, "Account Locked", "Your account is temporarily locked due to multiple failed login attempts. Please try again later.");
                        return;
                    } else {
                        // Unlock account after lockout period
                        query.prepare("UPDATE users SET account_locked = 0, failed_login_attempts = 0 WHERE id = :id");
                        query.bindValue(":id", userId);
                        if (!query.exec()) {
                            qDebug() << "Failed to unlock account:" << query.lastError().text();
                        }
                    }
                }

                // Log login attempt details
                QString logDetails = QString("User %1 login attempt.").arg(userId);
                GlobalFunctions::set_user(username, userId);
                GlobalFunctions::log_user_login();  // Log login attempt

                // Verify the password with the stored hash and salt
                if (GlobalFunctions::verifyPassword(password, storedPasswordHash, storedPasswordSalt)) {
                    qDebug() << "Login successful!";
                    QMessageBox::information(this, "Login Success", "You have successfully logged in.");

                    // Reset failed login attempts on successful login
                    query.prepare("UPDATE users SET failed_login_attempts = 0, last_login = :last_login WHERE id = :id");
                    query.bindValue(":last_login", QDateTime::currentDateTime());
                    query.bindValue(":id", userId);
                    if (!query.exec()) {
                        qDebug() << "Failed to update login details:" << query.lastError().text();
                    }

                    // Proceed to the main application window
                    hide();
                    DashBoard *dash_board = new DashBoard(nullptr);
                    dash_board->show();
                } else {
                    // Increment failed login attempts
                    failedLoginAttempts++;
                    query.prepare("UPDATE users SET failed_login_attempts = :failed_attempts WHERE id = :id");
                    query.bindValue(":failed_attempts", failedLoginAttempts);
                    query.bindValue(":id", userId);
                    if (!query.exec()) {
                        qDebug() << "Failed to update failed login attempts:" << query.lastError().text();
                    }

                    // Lock account after 3 failed attempts
                    if (failedLoginAttempts >= 3) {
                        query.prepare("UPDATE users SET account_locked = 1, lockout_time = :lockout_time WHERE id = :id");
                        query.bindValue(":lockout_time", QDateTime::currentDateTime());
                        query.bindValue(":id", userId);
                        if (!query.exec()) {
                            qDebug() << "Failed to lock account:" << query.lastError().text();
                        }

                        QMessageBox::warning(this, "Account Locked", "Your account has been locked due to multiple failed login attempts. Please try again later.");
                    } else {
                        QMessageBox::warning(this, "Login Failed", "Invalid password. Please try again.");
                    }
                }
            } else {
                qDebug() << "Invalid username.";
                QMessageBox::warning(this, "Login Failed", "Invalid username.");
            }
        } else {
            qDebug() << "Login query error:" << query.lastError().text();
            QMessageBox::critical(this, "Login Error", "Failed to execute login query.");
        }

    }else if(admin==false && employee==true){
        QString email = ui->username_lineEdit->text();
        QString password = ui->pass_lineEdit->text();

        QSqlQuery query;
        query.prepare("SELECT employee_id, email, password_hash, password_salt, account_locked, lockout_time, failed_login_attempts, is_active "
                      "FROM employees WHERE email = :email LIMIT 1");
        query.bindValue(":email", email);

        if (query.exec()) {
            if (query.next()) {  // If a result is found
                int employeeId = query.value("employee_id").toInt();
                QString storedPasswordHash = query.value("password_hash").toString();
                QString storedPasswordSalt = query.value("password_salt").toString();
                bool accountLocked = query.value("account_locked").toBool();
                QDateTime lockoutTime = query.value("lockout_time").toDateTime();
                int failedLoginAttempts = query.value("failed_login_attempts").toInt();
                bool isActive = query.value("is_active").toBool();

                // Check if the account is active
                if (!isActive) {
                    qDebug() << "Account is deactivated.";
                    QMessageBox::warning(this, "Account Inactive", "Your account has been deactivated. Please contact support.");
                    return;
                }

                // Check if the account is locked
                if (accountLocked) {
                    QDateTime currentTime = QDateTime::currentDateTime();
                    if (lockoutTime.isValid() && lockoutTime.addSecs(300) > currentTime) {  // 5-minute lockout
                        qDebug() << "Account is locked. Lockout will expire at:" << lockoutTime.addSecs(300).toString();
                        QMessageBox::warning(this, "Account Locked", "Your account is temporarily locked due to multiple failed login attempts. Please try again later.");
                        return;
                    } else {
                        // Unlock account after lockout period
                        query.prepare("UPDATE employees SET account_locked = 0, failed_login_attempts = 0 WHERE employee_id = :id");
                        query.bindValue(":id", employeeId);
                        if (!query.exec()) {
                            qDebug() << "Failed to unlock account:" << query.lastError().text();
                        }
                    }
                }

                // Log login attempt
                QString logDetails = QString("User %1 login attempt.").arg(email);
                GlobalFunctions::set_user(email, employeeId);
                GlobalFunctions::log_user_login();  // Log login attempt

                // Verify the password with the stored hash and salt
                if (GlobalFunctions::verifyPassword(password, storedPasswordHash, storedPasswordSalt)) {
                    qDebug() << "Login successful!";
                    QMessageBox::information(this, "Login Success", "You have successfully logged in.");

                    // Reset failed login attempts on successful login
                    query.prepare("UPDATE employees SET failed_login_attempts = 0, last_login = :last_login WHERE employee_id = :id");
                    query.bindValue(":last_login", QDateTime::currentDateTime());
                    query.bindValue(":id", employeeId);
                    if (!query.exec()) {
                        qDebug() << "Failed to update login details:" << query.lastError().text();
                    }

                    // Proceed to the main application window
                    hide();
                    DashBoard *dash_board = new DashBoard(nullptr);
                    dash_board->show();
                } else {
                    // Increment failed login attempts
                    failedLoginAttempts++;
                    query.prepare("UPDATE employees SET failed_login_attempts = :failed_attempts WHERE employee_id = :id");
                    query.bindValue(":failed_attempts", failedLoginAttempts);
                    query.bindValue(":id", employeeId);
                    if (!query.exec()) {
                        qDebug() << "Failed to update failed login attempts:" << query.lastError().text();
                    }

                    // Lock account after 3 failed attempts
                    if (failedLoginAttempts >= 3) {
                        query.prepare("UPDATE employees SET account_locked = 1, lockout_time = :lockout_time WHERE employee_id = :id");
                        query.bindValue(":lockout_time", QDateTime::currentDateTime());
                        query.bindValue(":id", employeeId);
                        if (!query.exec()) {
                            qDebug() << "Failed to lock account:" << query.lastError().text();
                        }

                        QMessageBox::warning(this, "Account Locked", "Your account has been locked due to multiple failed login attempts. Please try again later.");
                    } else {
                        QMessageBox::warning(this, "Login Failed", "Invalid password. Please try again.");
                    }
                }
            } else {
                qDebug() << "Invalid email.";
                QMessageBox::warning(this, "Login Failed", "Invalid email.");
            }
        } else {
            qDebug() << "Login query error:" << query.lastError().text();
            QMessageBox::critical(this, "Login Error", "Failed to execute login query.");
        }

    }else{
        QMessageBox::warning(this,"Report","Something went wrong");
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (QMessageBox::question(this, "Close IMS", "Are you sure you want to exit and end all processes?") == QMessageBox::Yes) {
        //logs
        GlobalFunctions::log_user_logout();
        // Close the database connection
        DatabaseManager &dbManager = DatabaseManager::instance();
        if (dbManager.getDatabase().isOpen()) {
            dbManager.getDatabase().close();
            qDebug() << "Database connection closed.";
        }

        // Perform any additional cleanup if required

        QApplication::quit();  // Terminate the application
        event->accept();
    } else {
        event->ignore();  // Keep the window open
    }
}

void MainWindow::on_toggle_button_main_circle_clicked()
{
    // Fade out effect for the main circle button
    QGraphicsOpacityEffect *opacityEffectMain = new QGraphicsOpacityEffect(this);
    ui->toggle_button_main_circle->setGraphicsEffect(opacityEffectMain);

    QPropertyAnimation *fadeOutMain = new QPropertyAnimation(opacityEffectMain, "opacity");
    fadeOutMain->setDuration(300); // 300 ms for fade out
    fadeOutMain->setStartValue(1.0); // Fully visible
    fadeOutMain->setEndValue(0.0);  // Fully transparent

    // Fade in effect for the admin circle button
    QGraphicsOpacityEffect *opacityEffectAdmin = new QGraphicsOpacityEffect(this);
    ui->toggle_button_admin_circle->setGraphicsEffect(opacityEffectAdmin);

    QPropertyAnimation *fadeInAdmin = new QPropertyAnimation(opacityEffectAdmin, "opacity");
    fadeInAdmin->setDuration(300); // 300 ms for fade in
    fadeInAdmin->setStartValue(0.0); // Fully transparent
    fadeInAdmin->setEndValue(1.0);   // Fully visible

    // Chain animations
    connect(fadeOutMain, &QPropertyAnimation::finished, [=]() {
        ui->toggle_button_main_circle->hide(); // Hide the main button after fade-out
        ui->toggle_button_admin_circle->show(); // Show the admin button
        ui->label_input_uername_email->setText("Username");
        ui->username_lineEdit->setPlaceholderText("Enter your username");
        ui->toogle_status_label->setText("Admin");
        ui->title_job_label->setText("Login as Admin");
        admin = true;
        employee = false;
        fadeInAdmin->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOutMain->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::on_toggle_button_admin_circle_clicked()
{
    // Fade out effect for the admin circle button
    QGraphicsOpacityEffect *opacityEffectAdmin = new QGraphicsOpacityEffect(this);
    ui->toggle_button_admin_circle->setGraphicsEffect(opacityEffectAdmin);

    QPropertyAnimation *fadeOutAdmin = new QPropertyAnimation(opacityEffectAdmin, "opacity");
    fadeOutAdmin->setDuration(300); // 300 ms for fade out
    fadeOutAdmin->setStartValue(1.0); // Fully visible
    fadeOutAdmin->setEndValue(0.0);  // Fully transparent

    // Fade in effect for the main circle button
    QGraphicsOpacityEffect *opacityEffectMain = new QGraphicsOpacityEffect(this);
    ui->toggle_button_main_circle->setGraphicsEffect(opacityEffectMain);

    QPropertyAnimation *fadeInMain = new QPropertyAnimation(opacityEffectMain, "opacity");
    fadeInMain->setDuration(300); // 300 ms for fade in
    fadeInMain->setStartValue(0.0); // Fully transparent
    fadeInMain->setEndValue(1.0);   // Fully visible

    // Chain animations
    connect(fadeOutAdmin, &QPropertyAnimation::finished, [=]() {
        ui->toggle_button_admin_circle->hide(); // Hide the admin button after fade-out
        ui->toggle_button_main_circle->show();  // Show the main button
        ui->label_input_uername_email->setText("Email");
        ui->username_lineEdit->setPlaceholderText("Enter your email");
        ui->toogle_status_label->setText("Employee");
        ui->title_job_label->setText("Login as Employee");
        admin = false;
        employee=true;
        fadeInMain->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOutAdmin->start(QAbstractAnimation::DeleteWhenStopped);
}
