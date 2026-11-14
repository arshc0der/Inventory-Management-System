#ifndef IMS_DATAGRID_H
#define IMS_DATAGRID_H

#include <QWidget>
#include <QList>
#include <QString>

class QLineEdit;
class QComboBox;
class QTableView;
class QLabel;
class QFrame;
class QVBoxLayout;
class QToolButton;
class QPushButton;
class QStandardItemModel;
class GridProxy;

// One column of a data set. The SQL query must return the columns in this order.
struct GridColumn {
    enum Type { Text, Int, Money, Date, Status, Percent };
    QString header;
    Type type = Text;
    bool visible = true;   // shown by default (the user can change it in the Columns menu)
    bool sum = false;      // contribute to the footer totals
};

// One KPI card above the table. The SQL must return a single value.
struct GridStat {
    QString label;
    QString sql;
    char format = 'n';     // 'n' number, 'm' money, 'p' percent
};

// Everything the grid needs to know about one data set.
struct GridSpec {
    QString id;            // "sales"
    QString title;         // "Sales"
    QString subtitle;      // one line under the title
    QString sql;
    QList<GridColumn> columns;
    QList<GridStat> stats;
    QString permission;    // empty = everyone, otherwise admin or this permission flag
};

/*
 * DataGrid - the "advanced view" used for every module.
 *   search, status filter, column chooser, click-to-sort, KPI strip,
 *   footer totals, record-detail side panel and CSV export.
 */
class DataGrid : public QWidget
{
    Q_OBJECT
public:
    explicit DataGrid(const GridSpec &spec, QWidget *parent = nullptr);
    const GridSpec &spec() const { return m_spec; }

public slots:
    void refresh();
    void exportCsv();
    void focusSearch();

private:
    void applyFilters();
    void updateFooter();
    void showDetails();

    GridSpec m_spec;
    QLineEdit *m_search = nullptr;
    QComboBox *m_status = nullptr;
    QTableView *m_table = nullptr;
    QLabel *m_footer = nullptr;
    QFrame *m_detailCard = nullptr;
    QVBoxLayout *m_detailBody = nullptr;
    QPushButton *m_detailBtn = nullptr;
    QList<QLabel *> m_statLabels;
    QStandardItemModel *m_model = nullptr;
    GridProxy *m_proxy = nullptr;
    int m_statusColumn = -1;
};

#endif // IMS_DATAGRID_H
