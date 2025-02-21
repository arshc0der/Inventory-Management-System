#ifndef SALESTRENDS_CHART_H
#define SALESTRENDS_CHART_H

#include <QWidget>

namespace Ui {
class SalesTrends_chart;
}

class SalesTrends_chart : public QWidget
{
    Q_OBJECT

public:
    explicit SalesTrends_chart(QWidget *parent = nullptr);
    ~SalesTrends_chart();

private:
    Ui::SalesTrends_chart *ui;
};

#endif // SALESTRENDS_CHART_H
