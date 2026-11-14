#include "ims_datacenter.h"
#include "ims_datagrid.h"
#include "ims_toast.h"
#include "globalfunctions.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QList>

namespace {

GridColumn col(const QString &h, GridColumn::Type t = GridColumn::Text, bool visible = true, bool sum = false)
{
    GridColumn c;
    c.header = h;
    c.type = t;
    c.visible = visible;
    c.sum = sum;
    return c;
}

GridStat stat(const QString &label, const QString &sql, char fmt = 'n')
{
    GridStat s;
    s.label = label;
    s.sql = sql;
    s.format = fmt;
    return s;
}

QList<GridSpec> buildSpecs()
{
    using T = GridColumn;
    const bool admin = GlobalFunctions::is_admin();
    QList<GridSpec> list;

    {   // ------------------------------------------------------------- sales
        GridSpec s;
        s.id = "sales"; s.title = "Sales"; s.permission = "advance_view_sales";
        s.subtitle = "Every sale with product, discount and customer.";
        s.sql = "SELECT s.sale_id, substr(s.sale_date,1,16), COALESCE(p.name,'(deleted)'), COALESCE(p.category,'-'), "
                "s.quantity_sold, s.discount, s.total_price, COALESCE(c.name,'Walk-in') "
                "FROM sales s LEFT JOIN products p ON p.product_id = s.product_id "
                "LEFT JOIN customers c ON c.customer_id = s.customer_id ORDER BY s.sale_date DESC";
        s.columns = {col("ID", T::Int), col("Date", T::Date), col("Product"), col("Category"),
                     col("Qty", T::Int, true, true), col("Discount", T::Percent),
                     col("Total", T::Money, true, true), col("Customer")};
        s.stats = {stat("Sales", "SELECT COUNT(*) FROM sales"),
                   stat("Revenue", "SELECT COALESCE(SUM(total_price),0) FROM sales", 'm'),
                   stat("Units sold", "SELECT COALESCE(SUM(quantity_sold),0) FROM sales"),
                   stat("Average sale", "SELECT COALESCE(AVG(total_price),0) FROM sales", 'm')};
        list << s;
    }
    {   // ------------------------------------------------------------ orders
        GridSpec s;
        s.id = "orders"; s.title = "Orders"; s.permission = "advance_view_orders";
        s.subtitle = "Customer orders with status, line count and value.";
        s.sql = "SELECT o.order_id, substr(o.order_date,1,16), COALESCE(c.name,'-'), o.status, "
                "(SELECT COUNT(*) FROM order_details d WHERE d.order_id = o.order_id), "
                "COALESCE((SELECT SUM(d.quantity) FROM order_details d WHERE d.order_id = o.order_id),0), "
                "o.total_price FROM orders o LEFT JOIN customers c ON c.customer_id = o.customer_id "
                "ORDER BY o.order_date DESC";
        s.columns = {col("Order", T::Int), col("Date", T::Date), col("Customer"), col("Status", T::Status),
                     col("Lines", T::Int), col("Units", T::Int, true, true), col("Total", T::Money, true, true)};
        s.stats = {stat("Orders", "SELECT COUNT(*) FROM orders"),
                   stat("Pending", "SELECT COUNT(*) FROM orders WHERE status='Pending'"),
                   stat("Completed", "SELECT COUNT(*) FROM orders WHERE status='Completed'"),
                   stat("Order value", "SELECT COALESCE(SUM(total_price),0) FROM orders", 'm')};
        list << s;
    }
    {   // ------------------------------------------------------- order lines
        GridSpec s;
        s.id = "orderlines"; s.title = "Order lines"; s.permission = "advance_view_orders";
        s.subtitle = "Every product line on every order.";
        s.sql = "SELECT d.order_detail_id, d.order_id, COALESCE(p.name,'(deleted)'), d.quantity, d.price, "
                "d.quantity * d.price, COALESCE(o.status,'-') FROM order_details d "
                "LEFT JOIN products p ON p.product_id = d.product_id "
                "LEFT JOIN orders o ON o.order_id = d.order_id ORDER BY d.order_id DESC, d.order_detail_id";
        s.columns = {col("Line", T::Int), col("Order", T::Int), col("Product"), col("Qty", T::Int, true, true),
                     col("Unit price", T::Money), col("Line total", T::Money, true, true), col("Status", T::Status)};
        s.stats = {stat("Lines", "SELECT COUNT(*) FROM order_details"),
                   stat("Units ordered", "SELECT COALESCE(SUM(quantity),0) FROM order_details"),
                   stat("Line value", "SELECT COALESCE(SUM(quantity*price),0) FROM order_details", 'm')};
        list << s;
    }
    {   // --------------------------------------------------------- suppliers
        GridSpec s;
        s.id = "suppliers"; s.title = "Suppliers"; s.permission = "advance_view_products";
        s.subtitle = "Suppliers with the products and stock value they provide.";
        s.sql = "SELECT s.supplier_id, s.name, COALESCE(s.contact_info,''), COALESCE(s.address,''), "
                "(SELECT COUNT(*) FROM products p WHERE p.supplier_id = s.supplier_id), "
                "COALESCE((SELECT SUM(p.quantity * p.price) FROM products p WHERE p.supplier_id = s.supplier_id),0) "
                "FROM suppliers s ORDER BY s.name";
        s.columns = {col("ID", T::Int), col("Supplier"), col("Contact"), col("Address"),
                     col("Products", T::Int, true, true), col("Stock value", T::Money, true, true)};
        s.stats = {stat("Suppliers", "SELECT COUNT(*) FROM suppliers"),
                   stat("Products supplied", "SELECT COUNT(*) FROM products"),
                   stat("Stock value", "SELECT COALESCE(SUM(quantity*price),0) FROM products", 'm')};
        list << s;
    }
    {   // --------------------------------------------------------- customers
        GridSpec s;
        s.id = "customers"; s.title = "Customers"; s.permission = "advance_view_sales";
        s.subtitle = "Customers ranked by lifetime value.";
        s.sql = "SELECT c.customer_id, c.name, COALESCE(c.contact_info,''), COALESCE(c.address,''), "
                "COALESCE(c.loyalty_points,0), (SELECT COUNT(*) FROM sales s WHERE s.customer_id = c.customer_id), "
                "COALESCE((SELECT SUM(s.total_price) FROM sales s WHERE s.customer_id = c.customer_id),0) "
                "FROM customers c ORDER BY 7 DESC";
        s.columns = {col("ID", T::Int), col("Customer"), col("Contact"), col("Address"),
                     col("Loyalty", T::Int, true, true), col("Purchases", T::Int, true, true),
                     col("Lifetime value", T::Money, true, true)};
        s.stats = {stat("Customers", "SELECT COUNT(*) FROM customers"),
                   stat("Loyalty points", "SELECT COALESCE(SUM(loyalty_points),0) FROM customers"),
                   stat("Lifetime revenue", "SELECT COALESCE(SUM(total_price),0) FROM sales", 'm')};
        list << s;
    }
    {   // --------------------------------------------------------- employees
        GridSpec s;
        s.id = "employees"; s.title = "Employees"; s.permission = "advance_view_employees";
        s.subtitle = admin ? "Staff directory including pay." : "Staff directory.";
        s.sql = QString("SELECT employee_id, name, COALESCE(email,''), COALESCE(role,''), COALESCE(department,''), "
                        "COALESCE(employee_type,''), CASE WHEN is_active = 1 THEN 'Active' ELSE 'Inactive' END, "
                        "COALESCE(hire_date,'')%1 FROM employees ORDER BY name")
                    .arg(admin ? QStringLiteral(", COALESCE(salary,0)") : QString());
        s.columns = {col("ID", T::Int), col("Name"), col("Email"), col("Role"), col("Department"),
                     col("Type", T::Status), col("Status", T::Status), col("Hired", T::Date)};
        if (admin)
            s.columns << col("Salary", T::Money, true, true);
        s.stats = {stat("Employees", "SELECT COUNT(*) FROM employees"),
                   stat("Active", "SELECT COUNT(*) FROM employees WHERE is_active = 1"),
                   stat("Departments", "SELECT COUNT(DISTINCT department) FROM employees")};
        if (admin)
            s.stats << stat("Monthly payroll", "SELECT COALESCE(SUM(salary),0) FROM employees WHERE is_active = 1", 'm');
        list << s;
    }
    {   // -------------------------------------------------------- promotions
        GridSpec s;
        s.id = "promotions"; s.title = "Promotions"; s.permission = "advance_view_promotion";
        s.subtitle = "Campaigns with their live status.";
        s.sql = "SELECT promotion_id, name, COALESCE(description,''), start_date, end_date, discount_percentage, "
                "CASE WHEN date('now') < date(start_date) THEN 'Upcoming' "
                "WHEN date('now') > date(end_date) THEN 'Expired' ELSE 'Active' END, "
                "CASE WHEN applicable_product_ids IS NULL OR applicable_product_ids = '' THEN 0 "
                "ELSE length(applicable_product_ids) - length(replace(applicable_product_ids, ',', '')) + 1 END "
                "FROM promotions ORDER BY start_date DESC";
        s.columns = {col("ID", T::Int), col("Promotion"), col("Description"), col("Starts", T::Date),
                     col("Ends", T::Date), col("Discount", T::Percent), col("Status", T::Status),
                     col("Products", T::Int)};
        s.stats = {stat("Promotions", "SELECT COUNT(*) FROM promotions"),
                   stat("Active now", "SELECT COUNT(*) FROM promotions WHERE date('now') BETWEEN date(start_date) AND date(end_date)"),
                   stat("Average discount", "SELECT COALESCE(AVG(discount_percentage),0) FROM promotions", 'p')};
        list << s;
    }
    {   // ------------------------------------------------------- adjustments
        GridSpec s;
        s.id = "adjustments"; s.title = "Stock adjustments"; s.permission = "advance_view_products";
        s.subtitle = "Manual stock-ins and write-offs with reasons.";
        s.sql = "SELECT a.adjustment_id, substr(a.adjustment_date,1,16), COALESCE(p.name,'(deleted)'), "
                "a.adjustment_type, a.quantity, COALESCE(a.reason,'') FROM inventory_adjustments a "
                "LEFT JOIN products p ON p.product_id = a.product_id ORDER BY a.adjustment_date DESC";
        s.columns = {col("ID", T::Int), col("Date", T::Date), col("Product"), col("Type", T::Status),
                     col("Qty", T::Int, true, true), col("Reason")};
        s.stats = {stat("Adjustments", "SELECT COUNT(*) FROM inventory_adjustments"),
                   stat("Units added", "SELECT COALESCE(SUM(quantity),0) FROM inventory_adjustments WHERE adjustment_type='Add'"),
                   stat("Units removed", "SELECT COALESCE(SUM(quantity),0) FROM inventory_adjustments WHERE adjustment_type='Remove'")};
        list << s;
    }
    {   // ----------------------------------------------------- notifications
        GridSpec s;
        s.id = "notifications"; s.title = "Notifications"; s.permission = "notification_dashboard";
        s.subtitle = "System and stock alerts.";
        s.sql = "SELECT notification_id, created_at, CASE WHEN is_read = 1 THEN 'Read' ELSE 'Unread' END, "
                "COALESCE(type,''), message FROM notifications ORDER BY created_at DESC";
        s.columns = {col("ID", T::Int), col("Created", T::Date), col("State", T::Status),
                     col("Type", T::Status), col("Message")};
        s.stats = {stat("Notifications", "SELECT COUNT(*) FROM notifications"),
                   stat("Unread", "SELECT COUNT(*) FROM notifications WHERE is_read = 0"),
                   stat("Risk alerts", "SELECT COUNT(*) FROM notifications WHERE type = 'risk'")};
        list << s;
    }
    {   // ---------------------------------------------------------- audit log
        GridSpec s;
        s.id = "audit"; s.title = "Audit log"; s.permission = "activity_dashboard";
        s.subtitle = "Who did what, and when.";
        s.sql = "SELECT l.log_id, l.timestamp, COALESCE(u.username, 'user ' || l.user_id), l.action, "
                "COALESCE(l.details,'') FROM audit_logs l LEFT JOIN users u ON u.id = l.user_id "
                "ORDER BY l.timestamp DESC";
        s.columns = {col("ID", T::Int), col("Time", T::Date), col("User"), col("Action"), col("Details")};
        s.stats = {stat("Events", "SELECT COUNT(*) FROM audit_logs"),
                   stat("Users", "SELECT COUNT(DISTINCT user_id) FROM audit_logs"),
                   stat("Today", "SELECT COUNT(*) FROM audit_logs WHERE date(timestamp) = date('now')")};
        list << s;
    }
    return list;
}

// built fresh each time so permissions / admin state are always current
QList<GridSpec> specs()
{
    return buildSpecs();
}

bool allowed(const GridSpec &s)
{
    return s.permission.isEmpty() || GlobalFunctions::is_admin() || GlobalFunctions::hasPermission(s.permission);
}
} // namespace

// ---------------------------------------------------------------------------
DataCenter::DataCenter(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("DataCenter");
    setProperty("ims_managed", true);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(20, 20, 0, 16);
    root->setSpacing(0);

    // ---- left: data set list --------------------------------------------
    auto *side = new QFrame(this);
    side->setProperty("role", "card");
    side->setFixedWidth(220);
    auto *sl = new QVBoxLayout(side);
    sl->setContentsMargins(0, 12, 0, 8);
    sl->setSpacing(4);
    auto *cap = new QLabel(tr("   DATA CENTER"), side);
    cap->setProperty("role", "muted");
    sl->addWidget(cap);

    m_list = new QListWidget(side);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setFocusPolicy(Qt::NoFocus);
    sl->addWidget(m_list, 1);
    root->addWidget(side);

    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack, 1);

    // placeholder page 0
    auto *empty = new QLabel(tr("Choose a data set on the left."), m_stack);
    empty->setAlignment(Qt::AlignCenter);
    empty->setProperty("role", "muted");
    m_stack->addWidget(empty);

    const QList<GridSpec> all = specs();
    for (int i = 0; i < all.size(); ++i) {
        const GridSpec &s = all.at(i);
        auto *item = new QListWidgetItem(s.title, m_list);
        item->setData(Qt::UserRole, s.id);
        if (!allowed(s)) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setText(s.title + QString::fromUtf8("   \xF0\x9F\x94\x92"));
            item->setToolTip(tr("You need the '%1' permission to open this view.").arg(s.permission));
        }
    }

    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) { select(row); });

    for (int i = 0; i < m_list->count(); ++i) {
        if (m_list->item(i)->flags() & Qt::ItemIsEnabled) {
            m_list->setCurrentRow(i);
            break;
        }
    }
}

void DataCenter::select(int row)
{
    if (row < 0 || row >= m_list->count())
        return;
    const QString id = m_list->item(row)->data(Qt::UserRole).toString();

    if (!m_pageOfId.contains(id)) {
        for (const GridSpec &s : specs()) {
            if (s.id == id) {
                auto *grid = new DataGrid(s, m_stack);
                m_pageOfId.insert(id, m_stack->addWidget(grid));
                break;
            }
        }
    }
    if (m_pageOfId.contains(id))
        m_stack->setCurrentIndex(m_pageOfId.value(id));
}

void DataCenter::showDataset(const QString &id)
{
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem *it = m_list->item(i);
        if (it->data(Qt::UserRole).toString() == id) {
            if (it->flags() & Qt::ItemIsEnabled)
                m_list->setCurrentRow(i);
            else
                Toast::notify(this, tr("You do not have permission to open this view."), Toast::Warning);
            return;
        }
    }
}

void DataCenter::refreshCurrent()
{
    if (auto *g = qobject_cast<DataGrid *>(m_stack->currentWidget()))
        g->refresh();
}
