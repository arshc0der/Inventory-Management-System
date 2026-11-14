#ifndef IMS_OVERVIEW_H
#define IMS_OVERVIEW_H

#include <QWidget>

class QLabel;
class QVBoxLayout;
class MiniBars;

/*
 * OverviewPage - an executive summary: KPI cards, 14-day revenue bars,
 * top products, stock alerts, recent activity and one-click quick actions.
 */
class OverviewPage : public QWidget
{
    Q_OBJECT
public:
    explicit OverviewPage(QWidget *parent = nullptr);

public slots:
    void refresh();

signals:
    // keys: add_product, add_sale, add_order, adjust_stock, inventory, datacenter, appearance
    void actionRequested(const QString &key);

private:
    QLabel *m_subtitle = nullptr;
    QLabel *m_revenue = nullptr;
    QLabel *m_sales = nullptr;
    QLabel *m_pending = nullptr;
    QLabel *m_attention = nullptr;
    QLabel *m_promos = nullptr;
    QLabel *m_stockValue = nullptr;
    QLabel *m_barsTotal = nullptr;
    MiniBars *m_bars = nullptr;
    QVBoxLayout *m_topBody = nullptr;
    QVBoxLayout *m_alertBody = nullptr;
    QVBoxLayout *m_activityBody = nullptr;
};

#endif // IMS_OVERVIEW_H
