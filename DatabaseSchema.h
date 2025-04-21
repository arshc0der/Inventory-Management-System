#ifndef DATABASESCHEMA_H
#define DATABASESCHEMA_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

void setAutoIncrementStart(QSqlQuery &query, const QString &tableName, int startValue) {
    QString updateQuery = QString("UPDATE sqlite_sequence SET seq = %1 WHERE name = '%2';").arg(startValue).arg(tableName);
    query.exec(updateQuery);
}

class DatabaseSchema {
public:
    static void initializeDatabase() {
        QSqlQuery query;

        // Users Table
        QString createUsersTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                business_name TEXT NOT NULL,               -- Name of the business
                address TEXT NOT NULL,                     -- Address of the user/business
                email TEXT UNIQUE NOT NULL,                -- User's email (unique identifier)
                pin INTEGER NOT NULL,                      -- Security PIN
                name TEXT NOT NULL,                        -- Full name of the user
                username TEXT UNIQUE NOT NULL,             -- Unique username
                password_hash TEXT NOT NULL,               -- Hashed password for secure storage
                password_salt TEXT NOT NULL,               -- Salt used for password hashing
                role TEXT NOT NULL DEFAULT 'User',         -- Role of the user ('Admin', 'Manager', etc.)

                -- Security Columns
                last_login TIMESTAMP,                      -- Last successful login timestamp
                failed_login_attempts INTEGER DEFAULT 0,   -- Count of failed login attempts
                account_locked BOOLEAN DEFAULT 0,          -- Indicates if the account is locked
                lockout_time TIMESTAMP,                    -- Timestamp when the account was locked
                password_reset_token TEXT,                 -- Token for password reset
                password_reset_expiry TIMESTAMP,           -- Expiry time for password reset token
                two_factor_enabled BOOLEAN DEFAULT 0,      -- Indicates if 2FA is enabled
                two_factor_secret TEXT,                    -- Secret for generating 2FA codes
                is_active BOOLEAN DEFAULT 1,               -- Indicates if the account is active

                -- Audit Columns
                created_by INTEGER,                        -- ID of the user who created the account
                updated_by INTEGER,                        -- ID of the user who last updated the account
                deleted_at TIMESTAMP,                      -- Timestamp for soft deletion

                -- Preferences
                language_preference TEXT,                  -- Preferred language of the user
                theme_preference TEXT,                     -- UI theme preference (e.g., light/dark mode)

                -- Account Recovery
                recovery_email TEXT,                       -- Alternate email for account recovery
                recovery_phone TEXT,                       -- Alternate phone number for account recovery

                -- Geolocation and Activity Tracking
                last_known_ip TEXT,                        -- Last login IP address
                last_known_location TEXT,                  -- Last known geographical location

                -- Additional Security
                email_verified BOOLEAN DEFAULT 0,          -- Indicates if email is verified
                phone_verified BOOLEAN DEFAULT 0,          -- Indicates if phone is verified
                security_questions JSON                    -- Security questions for account recovery
            );
        )";
        executeQuery(query, createUsersTable, "'users' table");
        setAutoIncrementStart(query, "users", 10001);
        //business_logos
        QString createBusinessLogosTable=R"(
            CREATE TABLE business_logos (
                id INTEGER PRIMARY KEY AUTOINCREMENT,    -- Unique ID for the logo
                business_name TEXT NOT NULL,              -- Name of the business (can be used for reference)
                logo_url TEXT NOT NULL,                   -- File path or URL of the logo image
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- Timestamp when the logo was added
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP   -- Timestamp for when the logo was last updated
            );
        )";
        executeQuery(query, createBusinessLogosTable, "'business_logos' table");
        setAutoIncrementStart(query, "business_logos", 1034000);

        //User Permissions Table
        QString createUser_permissionsTable = R"(
            CREATE TABLE IF NOT EXISTS User_permissions (
                user_id INTEGER PRIMARY KEY, -- or role_id if using roles
                product_dashboard BOOLEAN DEFAULT FALSE,
                add_product BOOLEAN DEFAULT FALSE,
                update_products BOOLEAN DEFAULT FALSE,
                delete_products BOOLEAN DEFAULT FALSE,
                advance_view_products BOOLEAN DEFAULT FALSE,
                adjustment_Stock BOOLEAN DEFAULT FALSE,

                sales_dashboard BOOLEAN DEFAULT FALSE,
                add_sales BOOLEAN DEFAULT FALSE,
                update_sales BOOLEAN DEFAULT FALSE,
                delete_sales BOOLEAN DEFAULT FALSE,
                advance_view_sales BOOLEAN DEFAULT FALSE,

                orders_dashboard BOOLEAN DEFAULT FALSE,
                add_orders BOOLEAN DEFAULT FALSE,
                update_orders BOOLEAN DEFAULT FALSE,
                delete_orders BOOLEAN DEFAULT FALSE,
                advance_view_orders BOOLEAN DEFAULT FALSE,

                activity_dashboard BOOLEAN DEFAULT FALSE,
                notification_dashboard BOOLEAN DEFAULT FALSE,

                promotion_dashboard BOOLEAN DEFAULT FALSE,
                add_promotion BOOLEAN DEFAULT FALSE,
                update_promotion BOOLEAN DEFAULT FALSE,
                delete_promotion BOOLEAN DEFAULT FALSE,
                advance_view_promotion BOOLEAN DEFAULT FALSE,

                user_dashboard BOOLEAN DEFAULT FALSE,
                add_employees BOOLEAN DEFAULT FALSE,
                update_employees BOOLEAN DEFAULT FALSE,
                delete_employees BOOLEAN DEFAULT FALSE,
                advance_view_employees BOOLEAN DEFAULT FALSE,
                settings_dashboard BOOLEAN DEFAULT FALSE
            );
        )";

        executeQuery(query, createUser_permissionsTable, "'User_permissions' table");
        setAutoIncrementStart(query, "User_permissions", 9200001);

        //Permissions Table
        QString createPermissionsTable = R"(
            CREATE TABLE IF NOT EXISTS permissions (
                user_id INTEGER PRIMARY KEY, -- or role_id if using roles
                dashboard_access BOOLEAN DEFAULT FALSE,
                product_dashboard BOOLEAN DEFAULT FALSE,
                add_product BOOLEAN DEFAULT FALSE,
                update_products BOOLEAN DEFAULT FALSE,
                delete_products BOOLEAN DEFAULT FALSE,
                advance_view_products BOOLEAN DEFAULT FALSE,
                adjustment_Stock BOOLEAN DEFAULT FALSE,

                sales_dashboard BOOLEAN DEFAULT FALSE,
                add_sales BOOLEAN DEFAULT FALSE,
                update_sales BOOLEAN DEFAULT FALSE,
                delete_sales BOOLEAN DEFAULT FALSE,
                advance_view_sales BOOLEAN DEFAULT FALSE,

                orders_dashboard BOOLEAN DEFAULT FALSE,
                add_orders BOOLEAN DEFAULT FALSE,
                update_orders BOOLEAN DEFAULT FALSE,
                delete_orders BOOLEAN DEFAULT FALSE,
                advance_view_orders BOOLEAN DEFAULT FALSE,

                activity_dashboard BOOLEAN DEFAULT FALSE,
                notification_dashboard BOOLEAN DEFAULT FALSE,

                promotion_dashboard BOOLEAN DEFAULT FALSE,
                add_promotion BOOLEAN DEFAULT FALSE,
                update_promotion BOOLEAN DEFAULT FALSE,
                delete_promotion BOOLEAN DEFAULT FALSE,
                advance_view_promotion BOOLEAN DEFAULT FALSE,

                user_dashboard BOOLEAN DEFAULT FALSE,
                add_employees BOOLEAN DEFAULT FALSE,
                update_employees BOOLEAN DEFAULT FALSE,
                delete_employees BOOLEAN DEFAULT FALSE,
                advance_view_employees BOOLEAN DEFAULT FALSE,
                settings_dashboard BOOLEAN DEFAULT FALSE
            );
        )";

        executeQuery(query, createPermissionsTable, "'Permissions' table");
        setAutoIncrementStart(query, "permissions", 20001);

        // Suppliers Table
        QString createSuppliersTable = R"(
            CREATE TABLE IF NOT EXISTS suppliers (
                supplier_id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                contact_info TEXT NOT NULL,
                address TEXT
            )
        )";
        executeQuery(query, createSuppliersTable, "'suppliers' table");
        setAutoIncrementStart(query, "suppliers", 30001);

        // Products Table
        QString createProductsTable = R"(
            CREATE TABLE IF NOT EXISTS products (
                product_id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                category TEXT NOT NULL,
                quantity INTEGER NOT NULL CHECK (quantity >= 0), -- Restrict quantity only
                price REAL NOT NULL,                             -- Price can be zero
                supplier_id INTEGER NOT NULL,
                purchase_date DATE NOT NULL,
                minimum_stock_level INTEGER NOT NULL DEFAULT 0,
                description TEXT,
                FOREIGN KEY(supplier_id) REFERENCES suppliers(supplier_id)
            )
        )";
        executeQuery(query, createProductsTable, "'products' table");
        setAutoIncrementStart(query, "products", 40001);

        // Sales Table
        QString createSalesTable = R"(
            CREATE TABLE IF NOT EXISTS sales (
                sale_id INTEGER PRIMARY KEY AUTOINCREMENT,
                product_id INTEGER NOT NULL,
                quantity_sold INTEGER NOT NULL,
                sale_date DATE NOT NULL,
                total_price REAL NOT NULL,
                discount REAL DEFAULT 0,
                customer_id INTEGER,
                FOREIGN KEY(product_id) REFERENCES products(product_id),
                FOREIGN KEY(customer_id) REFERENCES customers(customer_id)
            )
        )";
        executeQuery(query, createSalesTable, "'sales' table");
        setAutoIncrementStart(query, "sales", 50001);

        // Customers Table
        QString createCustomersTable = R"(
            CREATE TABLE IF NOT EXISTS customers (
                customer_id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                contact_info TEXT NOT NULL,
                address TEXT,
                loyalty_points INTEGER DEFAULT 0
            )
        )";
        executeQuery(query, createCustomersTable, "'customers' table");
        setAutoIncrementStart(query, "customers", 60001);

        // Orders Table
        QString createOrdersTable = R"(
            CREATE TABLE IF NOT EXISTS orders (
                order_id INTEGER PRIMARY KEY AUTOINCREMENT,
                customer_id INTEGER NOT NULL,
                order_date DATE NOT NULL,
                status TEXT NOT NULL DEFAULT 'Pending',
                total_price REAL NOT NULL,
                FOREIGN KEY(customer_id) REFERENCES customers(customer_id)
            )
        )";
        executeQuery(query, createOrdersTable, "'orders' table");
        setAutoIncrementStart(query, "orders", 70001);

        // Order Details Table
        QString createOrderDetailsTable = R"(
            CREATE TABLE IF NOT EXISTS order_details (
                order_detail_id INTEGER PRIMARY KEY AUTOINCREMENT,
                order_id INTEGER NOT NULL,
                product_id INTEGER NOT NULL,
                quantity INTEGER NOT NULL,
                price REAL NOT NULL,
                FOREIGN KEY(order_id) REFERENCES orders(order_id),
                FOREIGN KEY(product_id) REFERENCES products(product_id)
            )
        )";
        executeQuery(query, createOrderDetailsTable, "'order_details' table");
        setAutoIncrementStart(query, "order_details", 80001);

        // Inventory Adjustments Table
        QString createInventoryAdjustmentsTable = R"(
            CREATE TABLE IF NOT EXISTS inventory_adjustments (
                adjustment_id INTEGER PRIMARY KEY AUTOINCREMENT,
                product_id INTEGER NOT NULL,
                adjustment_type TEXT NOT NULL, -- 'Add' or 'Remove'
                quantity INTEGER NOT NULL,
                adjustment_date DATE NOT NULL,
                reason TEXT,
                FOREIGN KEY(product_id) REFERENCES products(product_id)
            )
        )";
        executeQuery(query, createInventoryAdjustmentsTable, "'inventory_adjustments' table");
        setAutoIncrementStart(query, "inventory_adjustments", 90001);

        // Promotions Table
        QString createPromotionsTable = R"(
            CREATE TABLE IF NOT EXISTS promotions (
                promotion_id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                description TEXT,
                start_date DATE NOT NULL,
                end_date DATE NOT NULL,
                discount_percentage REAL NOT NULL,
                applicable_product_ids TEXT -- Comma-separated product IDs
            )
        )";
        executeQuery(query, createPromotionsTable, "'promotions' table");
        setAutoIncrementStart(query, "promotions", 100001);

        // Employees Table
        QString createEmployeesTable = R"(
            CREATE TABLE IF NOT EXISTS employees (
                employee_id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                email TEXT UNIQUE NOT NULL,
                phone_number TEXT,
                role TEXT NOT NULL,
                department TEXT,
                hire_date DATE NOT NULL,
                termination_date DATE,
                status TEXT DEFAULT 'active',
                salary REAL,
                bonus REAL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                profile_picture TEXT,
                employee_type TEXT DEFAULT 'full-time',
                password_hash TEXT NOT NULL,
                password_salt TEXT NOT NULL,
                is_active BOOLEAN DEFAULT 1,

                -- Additional security columns
                password_reset_token TEXT,
                password_reset_expiry TIMESTAMP,
                last_login TIMESTAMP,
                failed_login_attempts INTEGER DEFAULT 0,
                account_locked BOOLEAN DEFAULT 0,
                lockout_time TIMESTAMP,
                two_factor_enabled BOOLEAN DEFAULT 0,
                two_factor_secret TEXT,

                -- Additional details
                address TEXT,
                date_of_birth DATE,
                gender TEXT,
                national_id TEXT,
                emergency_contact_name TEXT,
                emergency_contact_phone TEXT,
                bank_account_number TEXT,
                bank_name TEXT,
                tax_identification_number TEXT,
                social_security_number TEXT,
                work_permit_number TEXT,
                join_date DATE,
                work_email TEXT,
                shift_timing TEXT,
                skills TEXT,
                previous_employer TEXT,
                notes TEXT,
                manager_id INTEGER,
                employment_status TEXT,
                force_password_change BOOLEAN DEFAULT 1
            );
        )";
        executeQuery(query, createEmployeesTable, "'employees' table");
        setAutoIncrementStart(query, "employees", 110001);

        // Notifications Table
        QString createNotificationsTable = R"(
            CREATE TABLE IF NOT EXISTS notifications (
                notification_id INTEGER PRIMARY KEY AUTOINCREMENT,
                message TEXT NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                is_read INTEGER DEFAULT 0, -- 0: Unread, 1: Read
                type TEXT DEFAULT 'info', -- 'risk', 'info', etc.
                unique_key TEXT, -- Prevents logic-based duplicates
                auto_generated INTEGER DEFAULT 1 -- 1: system-generated, 0: user
            )
        )";
        executeQuery(query, createNotificationsTable, "'notifications' table");
        setAutoIncrementStart(query, "notifications", 120001);

        // Audit Logs Table
        QString createAuditLogsTable = R"(
            CREATE TABLE IF NOT EXISTS audit_logs (
                log_id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                action TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                details TEXT,
                FOREIGN KEY(user_id) REFERENCES users(id)
            )
        )";
        executeQuery(query, createAuditLogsTable, "'audit_logs' table");
        setAutoIncrementStart(query, "audit_logs", 130001);

        // Add the Products History Table
        QString createProductsHistoryTable = R"(
            CREATE TABLE IF NOT EXISTS products_history (
                history_id INTEGER PRIMARY KEY AUTOINCREMENT,
                product_id INTEGER,
                name TEXT,
                category TEXT,
                quantity INTEGER,
                price REAL,
                supplier_id INTEGER,
                action TEXT NOT NULL, -- 'INSERT', 'UPDATE', 'DELETE'
                action_date DATETIME DEFAULT CURRENT_TIMESTAMP,
                user_id INTEGER, -- Optional: Track the user who made the change
                FOREIGN KEY(product_id) REFERENCES products(product_id)
            )
        )";
        executeQuery(query, createProductsHistoryTable, "'products_history' table");
        setAutoIncrementStart(query, "products_history", 140001);

        // Add the Sales History Table
        QString createSalesHistoryTable = R"(
            CREATE TABLE IF NOT EXISTS sales_history (
                history_id INTEGER PRIMARY KEY AUTOINCREMENT,
                sale_id INTEGER NOT NULL,
                product_id INTEGER NOT NULL,
                quantity_sold INTEGER NOT NULL,
                sale_date DATE NOT NULL,
                total_price REAL NOT NULL,
                discount REAL DEFAULT 0,
                customer_id INTEGER,
                action TEXT NOT NULL, -- 'INSERT', 'UPDATE', or 'DELETE'
                action_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (sale_id) REFERENCES sales(sale_id),
                FOREIGN KEY (product_id) REFERENCES products(product_id),
                FOREIGN KEY (customer_id) REFERENCES customers(customer_id)
            );
        )";
        executeQuery(query, createSalesHistoryTable, "'sales_history' table");
        setAutoIncrementStart(query, "sales_history", 150001);

        // Add the Order History Table
        QString createOrderHistoryTablee = R"(
            CREATE TABLE IF NOT EXISTS orders_history (
                history_id INTEGER PRIMARY KEY AUTOINCREMENT,
                order_id INTEGER NOT NULL,
                customer_id INTEGER NOT NULL,
                order_date TIMESTAMP NOT NULL,
                status TEXT NOT NULL,
                total_price REAL NOT NULL,
                action TEXT NOT NULL, -- 'INSERT', 'UPDATE', or 'DELETE'
                FOREIGN KEY (order_id) REFERENCES orders (order_id),
                FOREIGN KEY (customer_id) REFERENCES customers (customer_id)
            );
        )";
        executeQuery(query, createOrderHistoryTablee, "'orders_history' table");
        setAutoIncrementStart(query, "orders_history", 160001);

        // Promotion History Table
        QString createPromotionsHistoryTable = R"(
            CREATE TABLE IF NOT EXISTS promotions_history (
                history_id INTEGER PRIMARY KEY AUTOINCREMENT,
                promotion_id INTEGER,
                name TEXT,
                description TEXT,
                start_date DATE,
                end_date DATE,
                discount_percentage REAL,
                action TEXT NOT NULL, -- 'INSERT', 'UPDATE', or 'DELETE'
                action_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                user_id INTEGER, -- Optional: Track the user who made the change (if required)
                FOREIGN KEY(promotion_id) REFERENCES promotions(promotion_id)
            );
        )";
        executeQuery(query, createPromotionsHistoryTable, "'promotions_history' table");
        setAutoIncrementStart(query, "promotions_history", 170001);
        //Employees History Table
        QString createEmployeesHistoryTable = R"(
            CREATE TABLE IF NOT EXISTS employees_history (
                history_id INTEGER PRIMARY KEY AUTOINCREMENT,
                employee_id INTEGER NOT NULL,
                name TEXT,
                email TEXT,
                phone_number TEXT,
                role TEXT,
                department TEXT,
                hire_date DATE,
                termination_date DATE,
                status TEXT,
                salary REAL,
                bonus REAL,
                created_at TIMESTAMP,
                updated_at TIMESTAMP,
                profile_picture TEXT,
                employee_type TEXT,
                password_hash TEXT,
                password_salt TEXT,
                is_active BOOLEAN,
                address TEXT,
                date_of_birth DATE,
                gender TEXT,
                national_id TEXT,
                emergency_contact_name TEXT,
                emergency_contact_phone TEXT,
                bank_account_number TEXT,
                bank_name TEXT,
                tax_identification_number TEXT,
                social_security_number TEXT,
                work_permit_number TEXT,
                join_date DATE,
                work_email TEXT,
                shift_timing TEXT,
                skills TEXT,
                previous_employer TEXT,
                notes TEXT,
                manager_id INTEGER,
                employment_status TEXT,
                action TEXT NOT NULL, -- 'INSERT', 'UPDATE', 'DELETE'
                action_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- Timestamp of the action
                FOREIGN KEY (employee_id) REFERENCES employees(employee_id)
            );
        )";
        executeQuery(query, createEmployeesHistoryTable, "'employees_history' table");
        setAutoIncrementStart(query, "promotions_history", 180001);
        //Permissions History Table
        // Permissions History Table
        QString createPermissionsHistoryTable = R"(
            CREATE TABLE IF NOT EXISTS permissions_history (
                history_id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                dashboard_access BOOLEAN DEFAULT FALSE,
                product_dashboard BOOLEAN DEFAULT FALSE,
                add_product BOOLEAN DEFAULT FALSE,
                update_products BOOLEAN DEFAULT FALSE,
                delete_products BOOLEAN DEFAULT FALSE,
                advance_view_products BOOLEAN DEFAULT FALSE,
                adjustment_Stock BOOLEAN DEFAULT FALSE,

                sales_dashboard BOOLEAN DEFAULT FALSE,
                add_sales BOOLEAN DEFAULT FALSE,
                update_sales BOOLEAN DEFAULT FALSE,
                delete_sales BOOLEAN DEFAULT FALSE,
                advance_view_sales BOOLEAN DEFAULT FALSE,

                orders_dashboard BOOLEAN DEFAULT FALSE,
                add_orders BOOLEAN DEFAULT FALSE,
                update_orders BOOLEAN DEFAULT FALSE,
                delete_orders BOOLEAN DEFAULT FALSE,
                advance_view_orders BOOLEAN DEFAULT FALSE,

                activity_dashboard BOOLEAN DEFAULT FALSE,
                notification_dashboard BOOLEAN DEFAULT FALSE,

                promotion_dashboard BOOLEAN DEFAULT FALSE,
                add_promotion BOOLEAN DEFAULT FALSE,
                update_promotion BOOLEAN DEFAULT FALSE,
                delete_promotion BOOLEAN DEFAULT FALSE,
                advance_view_promotion BOOLEAN DEFAULT FALSE,

                user_dashboard BOOLEAN DEFAULT FALSE,
                add_employees BOOLEAN DEFAULT FALSE,
                update_employees BOOLEAN DEFAULT FALSE,
                delete_employees BOOLEAN DEFAULT FALSE,
                advance_view_employees BOOLEAN DEFAULT FALSE,
                settings_dashboard BOOLEAN DEFAULT FALSE,

                action TEXT NOT NULL,  -- 'INSERT', 'UPDATE', or 'DELETE'
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES permissions (user_id)
            );
        )";
        executeQuery(query, createPermissionsHistoryTable, "'permissions_history' table");
        setAutoIncrementStart(query, "permissions_history", 190001);

        //Triggers for product_history
        // Trigger for product_history INSERT
        QString createInsertTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_product_insert
            AFTER INSERT ON products
            FOR EACH ROW
            BEGIN
                INSERT INTO products_history (product_id, name, category, quantity, price, supplier_id, action, user_id)
                VALUES (NEW.product_id, NEW.name, NEW.category, NEW.quantity, NEW.price, NEW.supplier_id, 'INSERT', NULL);
            END;
        )";
        executeQuery(query, createInsertTrigger, "'log_product_insert' trigger");

        // Trigger for product_history UPDATE
        QString createUpdateTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_product_update
            AFTER UPDATE ON products
            FOR EACH ROW
            BEGIN
                INSERT INTO products_history (product_id, name, category, quantity, price, supplier_id, action, user_id)
                VALUES (NEW.product_id, NEW.name, NEW.category, NEW.quantity, NEW.price, NEW.supplier_id, 'UPDATE', NULL);
            END;
        )";
        executeQuery(query, createUpdateTrigger, "'log_product_update' trigger");

        // Trigger for product_history DELETE
        QString createDeleteTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_product_delete
            AFTER DELETE ON products
            FOR EACH ROW
            BEGIN
                INSERT INTO products_history (product_id, name, category, quantity, price, supplier_id, action, user_id)
                VALUES (OLD.product_id, OLD.name, OLD.category, OLD.quantity, OLD.price, OLD.supplier_id, 'DELETE', NULL);
            END;
        )";
        executeQuery(query, createDeleteTrigger, "'log_product_delete' trigger");

        //Triggers for sales_history
        //Insert Trigger for Sales:
        QString createSaleHistoryInsertTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS sale_history_insert
            AFTER INSERT ON sales
            FOR EACH ROW
            BEGIN
                INSERT INTO sales_history (sale_id, product_id, quantity_sold, sale_date, total_price, discount, customer_id, action)
                VALUES (NEW.sale_id, NEW.product_id, NEW.quantity_sold, NEW.sale_date, NEW.total_price, NEW.discount, NEW.customer_id, 'INSERT');
            END;
        )";
        executeQuery(query, createSaleHistoryInsertTrigger, "'sale_history_insert' trigger");


        //Update Trigger for Sales
        QString createSaleHistoryUpdateTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS sale_history_update
            AFTER UPDATE ON sales
            FOR EACH ROW
            BEGIN
                INSERT INTO sales_history (sale_id, product_id, quantity_sold, sale_date, total_price, discount, customer_id, action)
                VALUES (NEW.sale_id, NEW.product_id, NEW.quantity_sold, NEW.sale_date, NEW.total_price, NEW.discount, NEW.customer_id, 'UPDATE');
            END;
        )";
        executeQuery(query, createSaleHistoryUpdateTrigger, "'sale_history_update' trigger");


        //Delete Trigger for Sales
        QString createSaleHistoryDeleteTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS sale_history_delete
            AFTER DELETE ON sales
            FOR EACH ROW
            BEGIN
                INSERT INTO sales_history (sale_id, product_id, quantity_sold, sale_date, total_price, discount, customer_id, action)
                VALUES (OLD.sale_id, OLD.product_id, OLD.quantity_sold, OLD.sale_date, OLD.total_price, OLD.discount, OLD.customer_id, 'DELETE');
            END;
        )";
        executeQuery(query, createSaleHistoryDeleteTrigger, "'sale_history_delete' trigger");



        //Triggers for orders_history
        //Insert Trigger for Orders
        QString createOrderHistoryInsertTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS order_history_insert
            AFTER INSERT ON orders
            FOR EACH ROW
            BEGIN
                INSERT INTO orders_history (order_id, customer_id, order_date, status, total_price, action)
                VALUES (NEW.order_id, NEW.customer_id, NEW.order_date, NEW.status, NEW.total_price, 'INSERT');
            END;
        )";
        executeQuery(query, createOrderHistoryInsertTrigger, "'order_history_insert' trigger");

        //Update Trigger for Orders
        QString createOrderHistoryUpdateTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS order_history_update
            AFTER UPDATE ON orders
            FOR EACH ROW
            BEGIN
                INSERT INTO orders_history (order_id, customer_id, order_date, status, total_price, action)
                VALUES (NEW.order_id, NEW.customer_id, NEW.order_date, NEW.status, NEW.total_price, 'UPDATE');
            END;
        )";
        executeQuery(query, createOrderHistoryUpdateTrigger, "'order_history_update' trigger");

        //Delete Trigger for Orders
                QString createOrderHistoryDeleteTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS order_history_delete
            AFTER DELETE ON orders
            FOR EACH ROW
            BEGIN
                INSERT INTO orders_history (order_id, customer_id, order_date, status, total_price, action)
                VALUES (OLD.order_id, OLD.customer_id, OLD.order_date, OLD.status, OLD.total_price, 'DELETE');
            END;
        )";
        executeQuery(query, createOrderHistoryDeleteTrigger, "'order_history_delete' trigger");

        //Triggers for promtions table
        //Insert Trigger for promtions
        QString createInsertPromotionTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_promotion_insert
            AFTER INSERT ON promotions
            FOR EACH ROW
            BEGIN
                INSERT INTO promotions_history (promotion_id, name, description, start_date, end_date, discount_percentage, action)
                VALUES (NEW.promotion_id, NEW.name, NEW.description, NEW.start_date, NEW.end_date, NEW.discount_percentage, 'INSERT');
            END;
        )";
        executeQuery(query, createInsertPromotionTrigger, "'log_promotion_insert' trigger");

         //Update Trigger for promtions
        QString createUpdatePromotionTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_promotion_update
            AFTER UPDATE ON promotions
            FOR EACH ROW
            BEGIN
                INSERT INTO promotions_history (promotion_id, name, description, start_date, end_date, discount_percentage, action)
                VALUES (NEW.promotion_id, NEW.name, NEW.description, NEW.start_date, NEW.end_date, NEW.discount_percentage, 'UPDATE');
            END;
        )";
        executeQuery(query, createUpdatePromotionTrigger, "'log_promotion_update' trigger");

         //Delete Trigger for promtions
        QString createDeletePromotionTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_promotion_delete
            AFTER DELETE ON promotions
            FOR EACH ROW
            BEGIN
                INSERT INTO promotions_history (promotion_id, name, description, start_date, end_date, discount_percentage, action)
                VALUES (OLD.promotion_id, OLD.name, OLD.description, OLD.start_date, OLD.end_date, OLD.discount_percentage, 'DELETE');
            END;
        )";
        executeQuery(query, createDeletePromotionTrigger, "'log_promotion_delete' trigger");


        //Trigger for Updating Stock After a Sale chnages
        // Add the trigger for reducing stock after a sale
        QString createReduceStockTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS reduce_stock_after_sale
            AFTER INSERT ON sales
            FOR EACH ROW
            BEGIN
                UPDATE products
                SET quantity = quantity - NEW.quantity_sold
                WHERE product_id = NEW.product_id;

                -- Optional: Insert into notifications if stock goes below minimum level
                INSERT INTO notifications (message)
                SELECT 'Stock for product ID ' || NEW.product_id || ' is below the minimum level!'
                WHERE (SELECT quantity FROM products WHERE product_id = NEW.product_id) <
                      (SELECT minimum_stock_level FROM products WHERE product_id = NEW.product_id);
            END;
        )";
        executeQuery(query, createReduceStockTrigger, "'reduce_stock_after_sale' trigger");

        //Trigger for Updating Stock After a Sale update
        QString createUpdateStockAfterSaleUpdateTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS update_stock_after_sale_update
            AFTER UPDATE ON sales
            FOR EACH ROW
            BEGIN
                -- Restore stock for the old quantity sold
                UPDATE products
                SET quantity = quantity + OLD.quantity_sold
                WHERE product_id = OLD.product_id;

                -- Reduce stock for the new quantity sold
                UPDATE products
                SET quantity = quantity - NEW.quantity_sold
                WHERE product_id = NEW.product_id;

                -- Optional: Insert into notifications if stock goes below minimum level after update
                INSERT INTO notifications (message)
                SELECT 'Stock for product ID ' || NEW.product_id || ' is below the minimum level!'
                WHERE (SELECT quantity FROM products WHERE product_id = NEW.product_id) <
                      (SELECT minimum_stock_level FROM products WHERE product_id = NEW.product_id);
            END;
        )";
        executeQuery(query, createUpdateStockAfterSaleUpdateTrigger, "'update_stock_after_sale_update' trigger");

        //Trigger for Updating Stock After a Sale delete
        QString createRestoreStockAfterSaleDeleteTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS restore_stock_after_sale_delete
            AFTER DELETE ON sales
            FOR EACH ROW
            BEGIN
                -- Restore the stock for the deleted sale's quantity sold
                UPDATE products
                SET quantity = quantity + OLD.quantity_sold
                WHERE product_id = OLD.product_id;

                -- Optional: Insert into notifications if stock goes below minimum level after restore
                INSERT INTO notifications (message)
                SELECT 'Stock for product ID ' || OLD.product_id || ' is below the minimum level!'
                WHERE (SELECT quantity FROM products WHERE product_id = OLD.product_id) <
                      (SELECT minimum_stock_level FROM products WHERE product_id = OLD.product_id);
            END;
        )";
        executeQuery(query, createRestoreStockAfterSaleDeleteTrigger, "'restore_stock_after_sale_delete' trigger");

        //Trigger for Updating Stock After a orders changes
        // Trigger for Insert Order
        QString createReduceStockAfterOrderDetailsInsertTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS reduce_stock_after_order_details_insert
            AFTER INSERT ON order_details
            FOR EACH ROW
            BEGIN
                -- Check if the order status is 'Completed' before reducing stock
                UPDATE products
                SET quantity = quantity - NEW.quantity
                WHERE product_id = NEW.product_id
                AND EXISTS (
                    SELECT 1 FROM orders WHERE order_id = NEW.order_id AND status = 'Completed'
                );
            END;
        )";
        executeQuery(query, createReduceStockAfterOrderDetailsInsertTrigger, "'reduce_stock_after_order_details_insert' trigger");

        // Trigger for update order
        QString createReduceStockAfterOrderStatusUpdateTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS reduce_stock_after_order_status_update
            AFTER UPDATE ON orders
            FOR EACH ROW
            WHEN NEW.status = 'Completed' AND OLD.status != 'Completed'
            BEGIN
                -- Reduce the stock for each product in the order_details
                UPDATE products
                SET quantity = quantity - (
                    SELECT quantity
                    FROM order_details
                    WHERE order_id = NEW.order_id
                    AND product_id = products.product_id
                )
                WHERE product_id IN (SELECT product_id FROM order_details WHERE order_id = NEW.order_id);
            END;
        )";
        executeQuery(query, createReduceStockAfterOrderStatusUpdateTrigger, "'reduce_stock_after_order_status_update' trigger");

        // Trigger for DELETE order
        QString createReduceStockAfterOrderDeleteTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS reduce_stock_after_order_delete
            AFTER DELETE ON orders
            FOR EACH ROW
            BEGIN
                -- Add back the stock for each product in the order_details
                UPDATE products
                SET quantity = quantity + (
                    SELECT quantity
                    FROM order_details
                    WHERE order_id = OLD.order_id
                    AND product_id = products.product_id
                )
                WHERE product_id IN (SELECT product_id FROM order_details WHERE order_id = OLD.order_id);
            END;
        )";
        executeQuery(query, createReduceStockAfterOrderDeleteTrigger, "'reduce_stock_after_order_delete' trigger");

        //Triggers for Employees Table
        //Trigger for INSERT in employees Table
        QString createEmployeesInsertTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_employees_insert
                AFTER INSERT ON employees
                FOR EACH ROW
                BEGIN
                    INSERT INTO employees_history (
                        employee_id, name, email, phone_number, role, department, hire_date, termination_date, status, salary, bonus,
                        created_at, updated_at, profile_picture, employee_type, action
                    )
                    VALUES (
                        NEW.employee_id, NEW.name, NEW.email, NEW.phone_number, NEW.role, NEW.department, NEW.hire_date, NEW.termination_date, NEW.status,
                        NEW.salary, NEW.bonus, NEW.created_at, NEW.updated_at, NEW.profile_picture, NEW.employee_type, 'INSERT'
                    );
                END;
        )";
         executeQuery(query, createEmployeesInsertTrigger, "'log_employees_insert' trigger");
        //Trigger for UPDATE in employees Table
        QString createEmployeesUpdateTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_employees_update
                AFTER UPDATE ON employees
                FOR EACH ROW
                BEGIN
                    INSERT INTO employees_history (
                        employee_id, name, email, phone_number, role, department, hire_date, termination_date, status, salary, bonus,
                        created_at, updated_at, profile_picture, employee_type, action
                    )
                    VALUES (
                        NEW.employee_id, NEW.name, NEW.email, NEW.phone_number, NEW.role, NEW.department, NEW.hire_date, NEW.termination_date, NEW.status,
                        NEW.salary, NEW.bonus, NEW.created_at, NEW.updated_at, NEW.profile_picture, NEW.employee_type, 'UPDATE'
                    );
                END;
        )";
          executeQuery(query, createEmployeesUpdateTrigger, "'log_employees_update' trigger");
        //Trigger for DELETE in employees Table
        QString createEmployeesDeleteTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_employees_delete
                AFTER DELETE ON employees
                FOR EACH ROW
                BEGIN
                    INSERT INTO employees_history (
                        employee_id, name, email, phone_number, role, department, hire_date, termination_date, status, salary, bonus,
                        created_at, updated_at, profile_picture, employee_type, action
                    )
                    VALUES (
                        OLD.employee_id, OLD.name, OLD.email, OLD.phone_number, OLD.role, OLD.department, OLD.hire_date, OLD.termination_date, OLD.status,
                        OLD.salary, OLD.bonus, OLD.created_at, OLD.updated_at, OLD.profile_picture, OLD.employee_type, 'DELETE'
                    );
                END;
        )";
        executeQuery(query, createEmployeesDeleteTrigger, "'log_employees_delete' trigger");

        //Trggers for permission table
         // Trigger for INSERT in permissions Table
         QString createPermissionsInsertTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_permissions_insert
            AFTER INSERT ON permissions
            FOR EACH ROW
            BEGIN
                INSERT INTO permissions_history (
                    user_id, product_dashboard, add_product, update_products, delete_products, advance_view_products,
                    sales_dashboard, add_sales, update_sales, delete_sales, advance_view_sales,
                    orders_dashboard, add_orders, update_orders, delete_orders, advance_view_orders,
                    activity_dashboard, notification_dashboard, promotion_dashboard, add_promotion, update_promotion,
                    delete_promotion, advance_view_promotion, user_dashboard, add_employees, update_employees,
                    delete_employees, advance_view_employees, settings_dashboard, action
                )
                VALUES (
                    NEW.user_id, NEW.product_dashboard, NEW.add_product, NEW.update_products, NEW.delete_products, NEW.advance_view_products,
                    NEW.sales_dashboard, NEW.add_sales, NEW.update_sales, NEW.delete_sales, NEW.advance_view_sales,
                    NEW.orders_dashboard, NEW.add_orders, NEW.update_orders, NEW.delete_orders, NEW.advance_view_orders,
                    NEW.activity_dashboard, NEW.notification_dashboard, NEW.promotion_dashboard, NEW.add_promotion, NEW.update_promotion,
                    NEW.delete_promotion, NEW.advance_view_promotion, NEW.user_dashboard, NEW.add_employees, NEW.update_employees,
                    NEW.delete_employees, NEW.advance_view_employees, NEW.settings_dashboard, 'INSERT'
                );
            END;
        )";
         executeQuery(query, createPermissionsInsertTrigger, "'log_permissions_insert' trigger");

         // Trigger for UPDATE in permissions Table
         QString createPermissionsUpdateTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_permissions_update
            AFTER UPDATE ON permissions
            FOR EACH ROW
            BEGIN
                INSERT INTO permissions_history (
                    user_id, product_dashboard, add_product, update_products, delete_products, advance_view_products,
                    sales_dashboard, add_sales, update_sales, delete_sales, advance_view_sales,
                    orders_dashboard, add_orders, update_orders, delete_orders, advance_view_orders,
                    activity_dashboard, notification_dashboard, promotion_dashboard, add_promotion, update_promotion,
                    delete_promotion, advance_view_promotion, user_dashboard, add_employees, update_employees,
                    delete_employees, advance_view_employees, settings_dashboard, action
                )
                VALUES (
                    NEW.user_id, NEW.product_dashboard, NEW.add_product, NEW.update_products, NEW.delete_products, NEW.advance_view_products,
                    NEW.sales_dashboard, NEW.add_sales, NEW.update_sales, NEW.delete_sales, NEW.advance_view_sales,
                    NEW.orders_dashboard, NEW.add_orders, NEW.update_orders, NEW.delete_orders, NEW.advance_view_orders,
                    NEW.activity_dashboard, NEW.notification_dashboard, NEW.promotion_dashboard, NEW.add_promotion, NEW.update_promotion,
                    NEW.delete_promotion, NEW.advance_view_promotion, NEW.user_dashboard, NEW.add_employees, NEW.update_employees,
                    NEW.delete_employees, NEW.advance_view_employees, NEW.settings_dashboard, 'UPDATE'
                );
            END;
        )";
         executeQuery(query, createPermissionsUpdateTrigger, "'log_permissions_update' trigger");

         // Trigger for DELETE in permissions Table
         QString createPermissionsDeleteTrigger = R"(
            CREATE TRIGGER IF NOT EXISTS log_permissions_delete
            AFTER DELETE ON permissions
            FOR EACH ROW
            BEGIN
                INSERT INTO permissions_history (
                    user_id, product_dashboard, add_product, update_products, delete_products, advance_view_products,
                    sales_dashboard, add_sales, update_sales, delete_sales, advance_view_sales,
                    orders_dashboard, add_orders, update_orders, delete_orders, advance_view_orders,
                    activity_dashboard, notification_dashboard, promotion_dashboard, add_promotion, update_promotion,
                    delete_promotion, advance_view_promotion, user_dashboard, add_employees, update_employees,
                    delete_employees, advance_view_employees, settings_dashboard, action
                )
                VALUES (
                    OLD.user_id, OLD.product_dashboard, OLD.add_product, OLD.update_products, OLD.delete_products, OLD.advance_view_products,
                    OLD.sales_dashboard, OLD.add_sales, OLD.update_sales, OLD.delete_sales, OLD.advance_view_sales,
                    OLD.orders_dashboard, OLD.add_orders, OLD.update_orders, OLD.delete_orders, OLD.advance_view_orders,
                    OLD.activity_dashboard, OLD.notification_dashboard, OLD.promotion_dashboard, OLD.add_promotion, OLD.update_promotion,
                    OLD.delete_promotion, OLD.advance_view_promotion, OLD.user_dashboard, OLD.add_employees, OLD.update_employees,
                    OLD.delete_employees, OLD.advance_view_employees, OLD.settings_dashboard, 'DELETE'
                );
            END;
        )";
         executeQuery(query, createPermissionsDeleteTrigger, "'log_permissions_delete' trigger");

         //Trigger for inventory_adjustments Insert
         QString UpdateProductsOnAdjustmentTrigger=R"(
            CREATE TRIGGER update_product_quantity_on_adjustment
            AFTER INSERT ON inventory_adjustments
            BEGIN
                -- Increase quantity for 'Add'
                UPDATE products
                SET quantity = quantity + NEW.quantity
                WHERE product_id = NEW.product_id
                AND NEW.adjustment_type = 'Add';

                -- Decrease quantity for 'Remove'
                UPDATE products
                SET quantity = quantity - NEW.quantity
                WHERE product_id = NEW.product_id
                AND NEW.adjustment_type = 'Remove';

                -- Optional: Prevent negative stock
                UPDATE products
                SET quantity = 0
                WHERE product_id = NEW.product_id
                AND quantity < 0;
            END;

        )";
          executeQuery(query, UpdateProductsOnAdjustmentTrigger, "'Update_Products_On_Adjustment_Trigger' trigger");
    }

private:
    static void executeQuery(QSqlQuery &query, const QString &sql, const QString &description) {
        if (!query.exec(sql)) {
            qDebug() << "Failed to create" << description << ":" << query.lastError().text();
        } else {
            qDebug() << description << "created or already exists.";
        }
    }
};

#endif // DATABASESCHEMA_H
