#include "register.h"
#include "ui_register.h"
#include <QString>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QIcon>

#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

//Hashing
#include <QCryptographicHash>
#include <QUuid>
// Constructor
Register::Register(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Register)
{
    ui->setupUi(this);
}

// Validation flags for each input field
bool Business_name_check = false;
bool address_check = false;
bool email_check = false;
bool pin_check = false;
bool name_check = false;
bool username_check = false;
bool password_check = false;
bool role_check = false;

// Destructor
Register::~Register()
{
    delete ui;
}

// Show or hide password based on checkbox state
void Register::on_show_pass_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    ui->setPassword_lineEdit->setEchoMode(arg1 == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password);
    ui->confirmPassword_lineEdit->setEchoMode(arg1 == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password);
}

// Validate confirm password and set password
void Register::on_confirmPassword_lineEdit_textChanged(const QString &arg1)
{
    QString set_password = ui->setPassword_lineEdit->text();
    if (set_password != arg1) {
        ui->statusbar->showMessage("Password didn't match");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
        password_check = false;
    } else if (set_password.length() < 6) {
        ui->statusbar->showMessage("Password must be at least 6 characters");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
        password_check = false;
    } else {
        ui->statusbar->showMessage("Password matched");
        ui->statusbar->setStyleSheet("background-color: green; color: white;");
        password_check = true;
    }
}

// Validate email format
void Register::on_email_lineEdit_textChanged(const QString &arg1)
{
    if (arg1.contains('@') && arg1.contains('.')) {
        ui->statusbar->showMessage("Valid Email");
        ui->statusbar->setStyleSheet("background-color: green; color: white;");
        email_check = true;
    } else {
        ui->statusbar->showMessage("Invalid Email");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
        email_check = false;
    }
}

// Handle registration button click
void Register::on_register_btn_clicked()
{
    // Validate individual fields
    Business_name_check = ui->BusinessName_lineEdit->text().length() > 0;
    address_check = ui->address_lineEdit->text().length() > 0;
    name_check = ui->name_lineEdit->text().length() > 2;
    pin_check = ui->forgotPassword_pin_lineEdit->text().length() == 4;
    email_check = ui->email_lineEdit->text().length() > 0;
    username_check = ui->username_lineEdit->text().length() > 0;
    password_check = ui->confirmPassword_lineEdit->text().length() > 0;
    role_check = ui->role_lineEdit->text().length() > 0;

    // Proceed if all checks pass
    if (Business_name_check && address_check && email_check && pin_check && name_check && username_check && password_check && role_check) {
        QString businessName = ui->BusinessName_lineEdit->text();
        QString address = ui->address_lineEdit->text();
        QString email = ui->email_lineEdit->text();
        int pin = ui->forgotPassword_pin_lineEdit->text().toInt();
        QString name = ui->name_lineEdit->text();
        QString username = ui->username_lineEdit->text();
        QString password = ui->confirmPassword_lineEdit->text();
        QString role = ui->role_lineEdit->text();

        // Generate hashing
        QString password_salt = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
        QByteArray saltedPassword = (password + password_salt).toUtf8();
        QString password_hash = QString(QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256).toHex());

        QSqlDatabase db = QSqlDatabase::database(); // Get the current database connection
        if (!db.transaction()) {
            qDebug() << "Failed to start transaction:" << db.lastError().text();
            return;
        }

        QSqlQuery query;
        query.prepare(R"(
            INSERT INTO users (business_name, address, email, pin, name, username, password_hash, password_salt, role)
            VALUES (:business_name, :address, :email, :pin, :name, :username, :password_hash, :password_salt, :role)
        )");
        query.bindValue(":business_name", businessName);
        query.bindValue(":address", address);
        query.bindValue(":email", email);
        query.bindValue(":pin", pin);
        query.bindValue(":name", name);
        query.bindValue(":username", username);
        query.bindValue(":password_hash", password_hash);
        query.bindValue(":password_salt", password_salt);
        query.bindValue(":role", role);

        if (!query.exec()) {
            db.rollback();
            qDebug() << "Error inserting into 'users' table:" << query.lastError().text();
            QMessageBox::critical(this, "Registration Failed", "Error registering the user. Please try again.");
            return;
        }

        // Get the last inserted user_id
        QSqlQuery getUserIdQuery;
        if (!getUserIdQuery.exec("SELECT last_insert_rowid()")) {
            db.rollback();
            qDebug() << "Failed to retrieve last inserted user_id:" << getUserIdQuery.lastError().text();
            QMessageBox::critical(this, "Registration Failed", "Error retrieving user ID.");
            return;
        }

        int userId = -1;
        if (getUserIdQuery.next()) {
            userId = getUserIdQuery.value(0).toInt();
        } else {
            db.rollback();
            qDebug() << "Could not fetch user ID.";
            QMessageBox::critical(this, "Registration Failed", "Could not fetch user ID.");
            return;
        }

        // Insert into permissions table with all permissions set to true
        QSqlQuery permissionQuery;
        permissionQuery.prepare(R"(
            INSERT INTO User_permissions (
                user_id,
                product_dashboard, add_product, update_products, delete_products, advance_view_products, adjustment_Stock,
                sales_dashboard, add_sales, update_sales, delete_sales, advance_view_sales,
                orders_dashboard, add_orders, update_orders, delete_orders, advance_view_orders,
                activity_dashboard, notification_dashboard,
                promotion_dashboard, add_promotion, update_promotion, delete_promotion, advance_view_promotion,
                user_dashboard, add_employees, update_employees, delete_employees, advance_view_employees,
                settings_dashboard
            ) VALUES (
                :user_id,
                TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
                TRUE, TRUE, TRUE, TRUE, TRUE,
                TRUE, TRUE, TRUE, TRUE, TRUE,
                TRUE, TRUE,
                TRUE, TRUE, TRUE, TRUE, TRUE,
                TRUE, TRUE, TRUE, TRUE, TRUE,
                TRUE
            )
        )");
        permissionQuery.bindValue(":user_id", userId);

        if (!permissionQuery.exec()) {
            db.rollback();
            qDebug() << "Error inserting into 'permissions' table:" << permissionQuery.lastError().text();
            QMessageBox::critical(this, "Registration Failed", "Error setting user permissions.");
            return;
        }

        if (db.commit()) {
            QMessageBox::information(this, "Register Success", "Registration Successful and Permissions Set.");
        } else {
            db.rollback();
            qDebug() << "Failed to commit transaction:" << db.lastError().text();
            QMessageBox::critical(this, "Registration Failed", "Could not save changes. Please try again.");
        }
    } else {
        ui->statusbar->showMessage("Please fill all fields correctly");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
    }
}


// Validate business name
void Register::on_BusinessName_lineEdit_textChanged(const QString &arg1)
{
    if (arg1.contains(QRegularExpression("[@#$%&*]"))) {
        ui->statusbar->showMessage("Invalid input: @, #, $, %, &, * are not allowed");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
        Business_name_check = false;
    } else {
        if(arg1.length()==0){
            ui->label_business_name->setText("Your Business Name");
        }else{
            ui->label_business_name->setText(arg1);
        }
        ui->statusbar->showMessage("Valid Name");
        ui->statusbar->setStyleSheet("background-color: green; color: white;");
        Business_name_check = true;
    }
}

// Validate PIN
void Register::on_forgotPassword_pin_lineEdit_textChanged(const QString &arg1)
{
    bool isInteger = false;
    arg1.toInt(&isInteger);

    if (isInteger && arg1.length() == 4) {
        ui->statusbar->showMessage("Valid PIN");
        ui->statusbar->setStyleSheet("background-color: green; color: white;");
        pin_check = true;
    } else {
        ui->statusbar->showMessage("Only a 4-digit PIN is allowed");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
        pin_check = false;
    }
}

// Validate username
void Register::on_username_lineEdit_textChanged(const QString &arg1)
{
    if (arg1.length() > 0) {
        ui->statusbar->showMessage("Valid username");
        ui->statusbar->setStyleSheet("background-color: green; color: white;");
        username_check = true;
    } else {
        ui->statusbar->showMessage("Empty username is not allowed");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
        username_check = false;
    }
}

// Validate role
void Register::on_role_lineEdit_textChanged(const QString &arg1)
{
    if (arg1.length() >= 4) {
        ui->statusbar->showMessage("Valid role");
        ui->statusbar->setStyleSheet("background-color: green; color: white;");
        role_check = true;
    } else {
        ui->statusbar->showMessage("Minimum 4 characters required for role");
        ui->statusbar->setStyleSheet("background-color: red; color: white;");
        role_check = false;
    }
}

void Register::on_already_signin_btn_clicked()
{
    this->close();
}

void Register::on_select_btn_clicked()
{
    // Open file dialog to select an image
    QString filePath = QFileDialog::getOpenFileName(
        this,                               // Parent widget
        tr("Select Image"),                 // Dialog title
        "",                                 // Default directory (empty for home directory)
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*.*)") // Filter for image files
        );

    // Check if a file was selected
    if (!filePath.isEmpty()) {

        QString destinationDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/company_images/";

        // Create the directory if it doesn't exist
        QDir dir;
        if (!dir.exists(destinationDir)) {
            bool dirCreated = dir.mkpath(destinationDir);  // Create the directory if it doesn't exist
            if (dirCreated) {
                qDebug() << "Directory created:" << destinationDir;
            } else {
                qDebug() << "Failed to create directory:" << destinationDir;
                return;  // Exit function if the directory creation fails
            }
        }

        // Get the filename from the selected image path
        QFileInfo fileInfo(filePath);
        QString originalName = fileInfo.fileName();

        // Generate timestamp
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

        // Create new file name
        QString newFileName = "cmp_" + timestamp + "_" + originalName;

        // Define the destination file path (the new path where the image will be copied)
        QString destinationPath = destinationDir + newFileName;

        // Get the business name from the line edit field
        QString businessName = ui->BusinessName_lineEdit->text();  // The business name

        // Check if businessName is empty
        if (businessName.isEmpty()) {
            // Show an error message if the business name is empty
            QMessageBox::warning(this, tr("Input Error"), tr("Please enter a business name before selecting an image."));
            return;  // Exit the function if the business name is empty
        }

        // Copy the file from source to destination
        if (QFile::copy(filePath, destinationPath)) {
            // Set the icon from the copied file
            QIcon buttonIcon(destinationPath);
            ui->select_btn->setIcon(buttonIcon);

            // Optional: Set the size of the icon to fit the button
            ui->select_btn->setIconSize(QSize(48, 48));  // Adjust the size as needed

            // Update the label with the selected image path
            ui->label_select_url->setText(filePath);
            qDebug() << "Image copied successfully to" << destinationPath;

            // Delete any existing data in the business_logos table
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM business_logos");

            if (!deleteQuery.exec()) {
                qDebug() << "Failed to delete existing logo records:" << deleteQuery.lastError().text();
                return;  // Exit if the deletion fails
            }

            // Insert the new logo into the database
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO business_logos (business_name, logo_url) VALUES (:businessName, :logoUrl)");
            insertQuery.bindValue(":businessName", businessName);
            insertQuery.bindValue(":logoUrl", destinationPath);

            if (!insertQuery.exec()) {
                qDebug() << "Failed to insert new logo URL:" << insertQuery.lastError().text();
            } else {
                qDebug() << "Logo inserted successfully.";
            }
        } else {
            qDebug() << "Failed to copy image.";
        }
    } else {
        qDebug() << "No file selected.";
    }
}



