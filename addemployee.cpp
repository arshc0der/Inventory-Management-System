#include "addemployee.h"
#include "ui_addemployee.h"
#include <QString>
#include <QDate>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QIcon>

#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

#include <QCryptographicHash>
#include <QUuid>

bool is_password_hide = true;

AddEmployee::AddEmployee(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AddEmployee)
{
    ui->setupUi(this);
}

AddEmployee::~AddEmployee()
{
    delete ui;
}

void AddEmployee::on_open_btn_clicked()
{
    if (ui->product_verticalFrame->isVisible()) {
        ui->open_btn->setIcon(QIcon(":/resources/dashboard-icons/down.svg"));
        ui->product_verticalFrame->hide();
    } else {
        ui->open_btn->setIcon(QIcon(":/resources/dashboard-icons/up.svg"));
        ui->product_verticalFrame->show();
    }
}


void AddEmployee::on_sales_menu_btn_clicked()
{
    if (ui->sales_verticalFrame->isVisible()) {
        ui->sales_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/down.svg"));
        ui->sales_verticalFrame->hide();
    } else {
        ui->sales_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/up.svg"));
        ui->sales_verticalFrame->show();
    }
}


void AddEmployee::on_orders_menu_btn_clicked()
{
    if (ui->orders_verticalFrame->isVisible()) {
        ui->orders_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/down.svg"));
        ui->orders_verticalFrame->hide();
    } else {
        ui->orders_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/up.svg"));
        ui->orders_verticalFrame->show();
    }
}


void AddEmployee::on_users_menu_btn_clicked()
{
    if (ui->users_verticalFrame->isVisible()) {
        ui->users_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/down.svg"));
        ui->users_verticalFrame->hide();
    } else {
        ui->users_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/up.svg"));
        ui->users_verticalFrame->show();
    }
}


void AddEmployee::on_main_dashboard_menu_btn_clicked()
{
    if (ui->main_dashboard_verticalFrame->isVisible()) {
        ui->main_dashboard_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/down.svg"));
        ui->main_dashboard_verticalFrame->hide();
    } else {
        ui->main_dashboard_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/up.svg"));
        ui->main_dashboard_verticalFrame->show();
    }
}

void AddEmployee::on_promotion_dashboard_menu_btn_clicked()
{
    if (ui->promotion_dashboard_verticalFrame->isVisible()) {
        ui->promotion_dashboard_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/down.svg"));
        ui->promotion_dashboard_verticalFrame->hide();
    } else {
        ui->promotion_dashboard_menu_btn->setIcon(QIcon(":/resources/dashboard-icons/up.svg"));
        ui->promotion_dashboard_verticalFrame->show();
    }
}

void AddEmployee::on_add_employee_btn_clicked()
{

    // Step 1: Retrieve input values from the form
    QString name = ui->name_lineEdit->text().trimmed();
    QString email = ui->email_lineEdit->text().trimmed();
    QString phone_no = ui->phone_lineEdit->text().trimmed();
    QString role = ui->role_lineEdit->text().trimmed();
    QString department = ui->department_lineEdit->text().trimmed();
    QDate hire_date = ui->hireDate_dateEdit->date();
    QDate termination_date = ui->termination_date_dateEdit->date();
    QString status = ui->status_combox->currentText().trimmed();
    double salary = ui->salary_doubleSpinBox->value();
    double bonus = ui->bonus_doubleSpinBox->value();
    QString profile_picture_url = ui->profile_picture_url_label->text();
    QString employee_type = ui->type_comboBox->currentText().trimmed();
    QString password = ui->password_create_lineEdit->text();
    bool is_active = (ui->is_active_comboBox->currentText() == "Yes");

    QString address = ui->address_lineEdit->text();
    QDate date_of_birth = ui->date_of_birth_dateEdit->date();
    QString gender = ui->gender_comboBox->currentText();
    QString national_id = ui->national_id_lineEdit->text();
    QString emergency_contact_name = ui->emergency_contact_name_linedit->text();
    QString emergency_contact_phone = ui->emergency_contact_phone_spinBox->text();
    QString bank_account_number = ui->bank_account_number_spinBox->text();
    QString bank_name = ui->bank_name_lineEdit->text();
    QString tax_identification_number = ui->tax_identification_number_lineEdit->text();
    QString social_security_number = ui->social_security_number_lineEdit->text();
    QString work_permit_number = ui->work_permit_number_lineEdit->text();
    QDate join_date = ui->join_date_dateEdit->date();
    QString work_email = ui->work_email_lineEdit->text();
    QString shift_timing = ui->shift_timing_lineEdit->text();
    QString skills = ui->skills_lineEdit->text();
    QString previous_employer = ui->previous_employer_lineEdit->text();
    QString notes = ui->notes_lineEdit->text();
    QString manager_id = ui->manager_id_lineEdit->text();
    QString employment_status = ui->employment_status_lineEdit->text();

    // Step 2: Validate required inputs
    if (name.isEmpty() || email.isEmpty() || phone_no.isEmpty() || role.isEmpty() || department.isEmpty() || employee_type.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all required fields.");
        return;
    }

    // Step 3: Generate password salt and hash
    QString password_salt = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    QByteArray saltedPassword = (password + password_salt).toUtf8();
    QString password_hash = QString(QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256).toHex());

    //Gather permission values from checkboxes
    auto getCheckboxState = [](QCheckBox *checkbox) { return checkbox->checkState() == Qt::Checked; };

    bool product_dashboard = getCheckboxState(ui->product_dashbaord_checkBox);
    bool add_product = getCheckboxState(ui->product_dashbaord_checkBox_add);
    bool update_products = getCheckboxState(ui->product_dashbaord_checkBox_update);
    bool delete_products = getCheckboxState(ui->product_dashbaord_checkBox_delete);
    bool advance_view_products = getCheckboxState(ui->product_dashbaord_checkBox_advance_view);
    //adjustment
    bool adjustment_stock = getCheckboxState(ui->adjust_stock_checkbox);

    bool sales_dashboard = getCheckboxState(ui->sales_dashbaord_checkBox);
    bool add_sales = getCheckboxState(ui->sales_dashbaord_checkBox_add);
    bool update_sales = getCheckboxState(ui->sales_dashbaord_checkBox_update);
    bool delete_sales = getCheckboxState(ui->sales_dashbaord_checkBox_delete);
    bool advance_view_sales = getCheckboxState(ui->sales_dashbaord_checkBox_advanceview);

    bool orders_dashboard = getCheckboxState(ui->orders_dashbaord_checkBox);
    bool add_orders = getCheckboxState(ui->orders_dashbaord_checkBox_add);
    bool update_orders = getCheckboxState(ui->orders_dashbaord_checkBox_update);
    bool delete_orders = getCheckboxState(ui->orders_dashbaord_checkBox_delete);
    bool advance_view_orders = getCheckboxState(ui->orders_dashbaord_checkBox_advanceview);

    bool activity_dashboard = getCheckboxState(ui->activity_dashbaord_checkBox);
    bool notification_dashboard = getCheckboxState(ui->notification_dashbaord_checkBox);

    bool promotion_dashboard = getCheckboxState(ui->promotion_dashboard_checkBox);
    bool add_promotion = getCheckboxState(ui->promotion_dashboard_checkBox_add);
    bool update_promotion = getCheckboxState(ui->promotion_dashboard_checkBox_update);
    bool delete_promotion = getCheckboxState(ui->promotion_dashboard_checkBox_delete);
    bool advance_view_promotion = getCheckboxState(ui->promotion_dashboard_checkBox_advance_view);

    bool user_dashboard = getCheckboxState(ui->user_dashbaord_checkBox);
    bool add_employees = getCheckboxState(ui->user_dashbaord_checkBox_add);
    bool update_employees = getCheckboxState(ui->user_dashbaord_checkBox_update);
    bool delete_employees = getCheckboxState(ui->user_dashbaord_checkBox_delete);
    bool advance_view_employees = getCheckboxState(ui->user_dashbaord_checkBox_advanceview);

    bool settings_dashboard = getCheckboxState(ui->settings_dashbaord_checkBox);

    // Step 4: Prepare SQL query
    QSqlQuery query;
    query.prepare(R"(
    INSERT INTO employees (
        name, email, phone_number, role, department, hire_date, termination_date, status, salary, bonus,
        profile_picture, employee_type, password_hash, password_salt, is_active, address, date_of_birth, gender,
        national_id, emergency_contact_name, emergency_contact_phone, bank_account_number, bank_name,
        tax_identification_number, social_security_number, work_permit_number, join_date, work_email, shift_timing,
        skills, previous_employer, notes, manager_id, employment_status
    )
    VALUES (
        :name, :email, :phone_number, :role, :department, :hire_date, :termination_date, :status, :salary, :bonus,
        :profile_picture, :employee_type, :password_hash, :password_salt, :is_active, :address, :date_of_birth, :gender,
        :national_id, :emergency_contact_name, :emergency_contact_phone, :bank_account_number, :bank_name,
        :tax_identification_number, :social_security_number, :work_permit_number, :join_date, :work_email, :shift_timing,
        :skills, :previous_employer, :notes, :manager_id, :employment_status
    )
)");

    // Step 5: Bind values
    query.bindValue(":name", name);
    query.bindValue(":email", email);
    query.bindValue(":phone_number", phone_no);
    query.bindValue(":role", role);
    query.bindValue(":department", department);
    query.bindValue(":hire_date", hire_date);
    query.bindValue(":termination_date", termination_date);//.isValid() ? termination_date : QVariant()
    query.bindValue(":status", status.isEmpty() ? "active" : status);
    query.bindValue(":salary", salary);
    query.bindValue(":bonus", bonus);
    query.bindValue(":profile_picture", profile_picture_url);
    query.bindValue(":employee_type", employee_type);
    query.bindValue(":password_hash", password_hash);
    query.bindValue(":password_salt", password_salt);
    query.bindValue(":is_active", is_active);
    query.bindValue(":address", address);
    query.bindValue(":date_of_birth", date_of_birth);//.isValid() ? date_of_birth : QVariant()
    query.bindValue(":gender", gender);
    query.bindValue(":national_id", national_id);
    query.bindValue(":emergency_contact_name", emergency_contact_name);
    query.bindValue(":emergency_contact_phone", emergency_contact_phone);
    query.bindValue(":bank_account_number", bank_account_number);
    query.bindValue(":bank_name", bank_name);
    query.bindValue(":tax_identification_number", tax_identification_number);
    query.bindValue(":social_security_number", social_security_number);
    query.bindValue(":work_permit_number", work_permit_number);
    query.bindValue(":join_date", join_date);//.isValid() ? join_date : QVariant()
    query.bindValue(":work_email", work_email);
    query.bindValue(":shift_timing", shift_timing);
    query.bindValue(":skills", skills);
    query.bindValue(":previous_employer", previous_employer);
    query.bindValue(":notes", notes);
    query.bindValue(":manager_id", manager_id.toInt());
    query.bindValue(":employment_status", employment_status);

    qDebug() << "Executed Query:" << query.executedQuery();
    qDebug() << "Bound Values:" << query.boundValues();

    // Step 6: Execute query and handle result
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error 1", "Failed to add employee:\n" + query.lastError().text());
        return;
    }

    int employee_id = query.lastInsertId().toInt();

    // Step 7: Insert permissions for the new employee
    QSqlQuery permissionQuery;
    permissionQuery.prepare(R"(
        INSERT INTO permissions (user_id, product_dashboard, add_product, update_products, delete_products, advance_view_products,adjustment_Stock,
                                 sales_dashboard, add_sales, update_sales, delete_sales, advance_view_sales,
                                 orders_dashboard, add_orders, update_orders, delete_orders, advance_view_orders,
                                 activity_dashboard, notification_dashboard, promotion_dashboard, add_promotion,
                                 update_promotion, delete_promotion, advance_view_promotion,
                                 user_dashboard, add_employees, update_employees, delete_employees, advance_view_employees,
                                 settings_dashboard)
        VALUES (:user_id, :product_dashboard, :add_product, :update_products, :delete_products, :advance_view_products, :adjustment_Stock,
                :sales_dashboard, :add_sales, :update_sales, :delete_sales, :advance_view_sales,
                :orders_dashboard, :add_orders, :update_orders, :delete_orders, :advance_view_orders,
                :activity_dashboard, :notification_dashboard, :promotion_dashboard, :add_promotion,
                :update_promotion, :delete_promotion, :advance_view_promotion,
                :user_dashboard, :add_employees, :update_employees, :delete_employees, :advance_view_employees,
                :settings_dashboard)
    )");

    permissionQuery.bindValue(":user_id", employee_id);
    permissionQuery.bindValue(":product_dashboard", product_dashboard);
    permissionQuery.bindValue(":add_product", add_product);
    permissionQuery.bindValue(":update_products", update_products);
    permissionQuery.bindValue(":delete_products", delete_products);
    permissionQuery.bindValue(":advance_view_products", advance_view_products);

    permissionQuery.bindValue(":adjustment_Stock",adjustment_stock);

    permissionQuery.bindValue(":sales_dashboard", sales_dashboard);
    permissionQuery.bindValue(":add_sales", add_sales);
    permissionQuery.bindValue(":update_sales", update_sales);
    permissionQuery.bindValue(":delete_sales", delete_sales);
    permissionQuery.bindValue(":advance_view_sales", advance_view_sales);

    permissionQuery.bindValue(":orders_dashboard", orders_dashboard);
    permissionQuery.bindValue(":add_orders", add_orders);
    permissionQuery.bindValue(":update_orders", update_orders);
    permissionQuery.bindValue(":delete_orders", delete_orders);
    permissionQuery.bindValue(":advance_view_orders", advance_view_orders);

    permissionQuery.bindValue(":activity_dashboard", activity_dashboard);
    permissionQuery.bindValue(":notification_dashboard", notification_dashboard);

    permissionQuery.bindValue(":promotion_dashboard", promotion_dashboard);
    permissionQuery.bindValue(":add_promotion", add_promotion);
    permissionQuery.bindValue(":update_promotion", update_promotion);
    permissionQuery.bindValue(":delete_promotion", delete_promotion);
    permissionQuery.bindValue(":advance_view_promotion", advance_view_promotion);

    permissionQuery.bindValue(":user_dashboard", user_dashboard);
    permissionQuery.bindValue(":add_employees", add_employees);
    permissionQuery.bindValue(":update_employees", update_employees);
    permissionQuery.bindValue(":delete_employees", delete_employees);
    permissionQuery.bindValue(":advance_view_employees", advance_view_employees);
    permissionQuery.bindValue(":settings_dashboard", settings_dashboard);

    if (!permissionQuery.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to assign permissions to the new employee:\n" + permissionQuery.lastError().text());
        return;
    }

    // Step 8: Show success message and clear the form
    QMessageBox::information(this, "Success", "Employee added and permissions assigned successfully.");

    ui->name_lineEdit->clear();
    ui->email_lineEdit->clear();
    ui->phone_lineEdit->clear();
    ui->role_lineEdit->clear();
    ui->department_lineEdit->clear();
    ui->hireDate_dateEdit->clear();
    ui->salary_doubleSpinBox->setValue(0.0);
    ui->bonus_doubleSpinBox->setValue(0.0);
    reset_permission(); // Reset permissions checkboxes

}



void AddEmployee::on_product_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    // Check if the checkbox is checked
    if (arg1 == Qt::Checked) {
        ui->product_dashbaord_checkBox_add->setCheckState(Qt::Checked);
        ui->product_dashbaord_checkBox_update->setCheckState(Qt::Checked);
        ui->product_dashbaord_checkBox_delete->setCheckState(Qt::Checked);
        ui->product_dashbaord_checkBox_advance_view->setCheckState(Qt::Checked);
    }
    // Optionally, if you want to uncheck the others when unchecked
    else if (arg1 == Qt::Unchecked) {
        ui->product_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
        ui->product_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
        ui->product_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
        ui->product_dashbaord_checkBox_advance_view->setCheckState(Qt::Unchecked);
    }
;
}


void AddEmployee::on_sales_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    // Check if the checkbox is checked
    if (arg1 == Qt::Checked) {
        ui->sales_dashbaord_checkBox_add->setCheckState(Qt::Checked);
        ui->sales_dashbaord_checkBox_update->setCheckState(Qt::Checked);
        ui->sales_dashbaord_checkBox_delete->setCheckState(Qt::Checked);
        ui->sales_dashbaord_checkBox_advanceview->setCheckState(Qt::Checked);
    }
    // Optionally, if you want to uncheck the others when unchecked
    else if (arg1 == Qt::Unchecked) {
        ui->sales_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
        ui->sales_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
        ui->sales_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
        ui->sales_dashbaord_checkBox_advanceview->setCheckState(Qt::Unchecked);
    }
}


void AddEmployee::on_orders_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    // Check if the checkbox is checked
    if (arg1 == Qt::Checked) {
        ui->orders_dashbaord_checkBox_add->setCheckState(Qt::Checked);
        ui->orders_dashbaord_checkBox_update->setCheckState(Qt::Checked);
        ui->orders_dashbaord_checkBox_delete->setCheckState(Qt::Checked);
        ui->orders_dashbaord_checkBox_advanceview->setCheckState(Qt::Checked);
    }
    // Optionally, if you want to uncheck the others when unchecked
    else if (arg1 == Qt::Unchecked) {
        ui->orders_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
        ui->orders_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
        ui->orders_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
        ui->orders_dashbaord_checkBox_advanceview->setCheckState(Qt::Unchecked);
    }
}


void AddEmployee::on_user_dashbaord_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    // Check if the checkbox is checked
    if (arg1 == Qt::Checked) {
        ui->user_dashbaord_checkBox_add->setCheckState(Qt::Checked);
        ui->user_dashbaord_checkBox_update->setCheckState(Qt::Checked);
        ui->user_dashbaord_checkBox_delete->setCheckState(Qt::Checked);
        ui->user_dashbaord_checkBox_advanceview->setCheckState(Qt::Checked);
    }
    // Optionally, if you want to uncheck the others when unchecked
    else if (arg1 == Qt::Unchecked) {
        ui->user_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
        ui->user_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
        ui->user_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
        ui->user_dashbaord_checkBox_advanceview->setCheckState(Qt::Unchecked);
    }
}

void AddEmployee::on_promotion_dashboard_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    // Check if the checkbox is checked
    if (arg1 == Qt::Checked) {
        ui->promotion_dashboard_checkBox_add->setCheckState(Qt::Checked);
        ui->promotion_dashboard_checkBox_update->setCheckState(Qt::Checked);
        ui->promotion_dashboard_checkBox_delete->setCheckState(Qt::Checked);
        ui->promotion_dashboard_checkBox_advance_view->setCheckState(Qt::Checked);
    }
    // Optionally, if you want to uncheck the others when unchecked
    else if (arg1 == Qt::Unchecked) {
        ui->promotion_dashboard_checkBox_add->setCheckState(Qt::Unchecked);
        ui->promotion_dashboard_checkBox_update->setCheckState(Qt::Unchecked);
        ui->promotion_dashboard_checkBox_delete->setCheckState(Qt::Unchecked);
        ui->promotion_dashboard_checkBox_advance_view->setCheckState(Qt::Unchecked);
    }
}

void AddEmployee::reset_permission(){
    ui->product_dashbaord_checkBox->setCheckState(Qt::Unchecked);
    ui->product_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
    ui->product_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
    ui->product_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
    ui->product_dashbaord_checkBox_advance_view->setCheckState(Qt::Unchecked);

    ui->sales_dashbaord_checkBox->setCheckState(Qt::Unchecked);
    ui->sales_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
    ui->sales_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
    ui->sales_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
    ui->sales_dashbaord_checkBox_advanceview->setCheckState(Qt::Unchecked);

    ui->orders_dashbaord_checkBox->setCheckState(Qt::Unchecked);
    ui->orders_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
    ui->orders_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
    ui->orders_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
    ui->orders_dashbaord_checkBox_advanceview->setCheckState(Qt::Unchecked);

    ui->user_dashbaord_checkBox->setCheckState(Qt::Unchecked);
    ui->user_dashbaord_checkBox_add->setCheckState(Qt::Unchecked);
    ui->user_dashbaord_checkBox_update->setCheckState(Qt::Unchecked);
    ui->user_dashbaord_checkBox_delete->setCheckState(Qt::Unchecked);
    ui->user_dashbaord_checkBox_advanceview->setCheckState(Qt::Unchecked);

    ui->activity_dashbaord_checkBox->setCheckState(Qt::Unchecked);
    ui->notification_dashbaord_checkBox->setCheckState(Qt::Unchecked);
    ui->settings_dashbaord_checkBox->setCheckState(Qt::Unchecked);

    ui->promotion_dashboard_checkBox->setCheckState(Qt::Unchecked);
    ui->promotion_dashboard_checkBox_add->setCheckState(Qt::Unchecked);
    ui->promotion_dashboard_checkBox_update->setCheckState(Qt::Unchecked);
    ui->promotion_dashboard_checkBox_delete->setCheckState(Qt::Unchecked);
    ui->promotion_dashboard_checkBox_advance_view->setCheckState(Qt::Unchecked);
}

void AddEmployee::on_password_view_hide_btn_clicked()
{
    if (is_password_hide) {
        ui->password_view_hide_btn->setIcon(QIcon(":/resources/other_icons/view-password.svg"));
        ui->password_create_lineEdit->setEchoMode(QLineEdit::Normal); // Show password
        is_password_hide = false;
    } else {
        ui->password_view_hide_btn->setIcon(QIcon(":/resources/other_icons/view-off-password.svg"));
        ui->password_create_lineEdit->setEchoMode(QLineEdit::Password); // Hide password
        is_password_hide = true;
    }
}


void AddEmployee::on_select_profile_image_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Image"),
        "",
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*.*)")
        );

    if (!filePath.isEmpty()) {
        QString destinationDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/profile_image/";

        QDir dir;
        if (!dir.exists(destinationDir)) {
            if (!dir.mkpath(destinationDir)) {
                qDebug() << "Failed to create directory:" << destinationDir;
                return;
            }
        }

        QFileInfo fileInfo(filePath);
        QString originalName = fileInfo.fileName();

        // Generate timestamp string
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

        // Create new filename with emp_ prefix and timestamp
        QString newFileName = "emp_" + timestamp + "_" + originalName;

        // Full destination path with renamed file
        QString destinationPath = destinationDir + newFileName;

        if (QFile::copy(filePath, destinationPath)) {
            ui->profile_picture_url_label->setText(destinationPath);

            QPixmap image(destinationPath);  // Load the copied (renamed) image
            ui->label_profile_icon->setPixmap(image);
            ui->label_profile_icon->setScaledContents(true);

            qDebug() << "Image copied and renamed to" << destinationPath;
        } else {
            qDebug() << "Failed to copy image.";
        }
    } else {
        qDebug() << "No file selected.";
    }
}


