#ifndef PRODUCTS_HIGHTLIGHTS_MIN_LEVEL_CHART_H
#define PRODUCTS_HIGHTLIGHTS_MIN_LEVEL_CHART_H

#include <QWidget>

namespace Ui {
class Products_Hightlights_Min_Level_Chart;
}

class Products_Hightlights_Min_Level_Chart : public QWidget
{
    Q_OBJECT

public:
    explicit Products_Hightlights_Min_Level_Chart(QWidget *parent = nullptr);
    ~Products_Hightlights_Min_Level_Chart();

private:
    Ui::Products_Hightlights_Min_Level_Chart *ui;
};

#endif // PRODUCTS_HIGHTLIGHTS_MIN_LEVEL_CHART_H
