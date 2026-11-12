#ifndef IMS_EXPLORER_H
#define IMS_EXPLORER_H

#include <QWidget>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QTableView;
class QLabel;
class QStandardItemModel;
class ExplorerProxy;

/*
 * InventoryExplorer - a full data grid for the products table.
 *
 *  - live search across id / name / category / supplier
 *  - category filter and "low stock only" toggle
 *  - click-to-sort on every column
 *  - colour-coded stock status and a suggested reorder quantity
 *  - KPI strip (products, units, stock value, items needing attention)
 *  - one-click CSV export of exactly what is on screen
 */
class InventoryExplorer : public QWidget
{
    Q_OBJECT
public:
    explicit InventoryExplorer(QWidget *parent = nullptr);

public slots:
    void refresh();
    void exportCsv();
    void focusSearch();

private:
    void applyFilters();
    void updateFooter();

    QLineEdit *m_search = nullptr;
    QComboBox *m_category = nullptr;
    QCheckBox *m_lowOnly = nullptr;
    QTableView *m_table = nullptr;
    QLabel *m_footer = nullptr;
    QLabel *m_statSkus = nullptr;
    QLabel *m_statUnits = nullptr;
    QLabel *m_statValue = nullptr;
    QLabel *m_statAttention = nullptr;
    QStandardItemModel *m_model = nullptr;
    ExplorerProxy *m_proxy = nullptr;
};

#endif // IMS_EXPLORER_H
