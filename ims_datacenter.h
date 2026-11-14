#ifndef IMS_DATACENTER_H
#define IMS_DATACENTER_H

#include <QWidget>
#include <QMap>
#include <QString>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class DataGrid;

/*
 * DataCenter - the "advanced view" hub. A list of data sets on the left
 * (Sales, Orders, Order lines, Suppliers, Customers, Employees, Promotions,
 * Stock adjustments, Notifications, Audit log) and a DataGrid on the right.
 * Data sets are hidden behind the same advance_view_* permissions as before.
 */
class DataCenter : public QWidget
{
    Q_OBJECT
public:
    explicit DataCenter(QWidget *parent = nullptr);

public slots:
    void showDataset(const QString &id);   // e.g. "orders"
    void refreshCurrent();

private:
    void select(int row);

    QListWidget *m_list = nullptr;
    QStackedWidget *m_stack = nullptr;
    QMap<QString, int> m_pageOfId;         // id -> stack index (lazy)
};

#endif // IMS_DATACENTER_H
