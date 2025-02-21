#include "products_hightlights_min_level_chart.h"
#include "ui_products_hightlights_min_level_chart.h"
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Products_Hightlights_Min_Level_Chart::Products_Hightlights_Min_Level_Chart(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Products_Hightlights_Min_Level_Chart)
{
    ui->setupUi(this);

    // Create a bar series to display product quantities
    QBarSeries *barSeries = new QBarSeries();

    // Create a set for product quantities
    QBarSet *barSet = new QBarSet("Product Quantity");

    // Database query to fetch products with quantity < minimum_stock_level
    QSqlQuery query;
    if (!query.exec("SELECT name, quantity FROM products WHERE quantity < minimum_stock_level;")) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return;
    }

    // Check if query has results
    if (!query.next()) {
        qDebug() << "No products found with quantity less than minimum stock level.";
        return;
    }

    // Create lists to store product names and quantities
    QStringList categories;
    QList<int> quantities;

    // Populate the bar set and categories with data
    do {
        QString productName = query.value(0).toString();
        int productQuantity = query.value(1).toInt();

        // Append data to bar set
        *barSet << productQuantity;

        // Add product name to categories list
        categories << productName;
    } while (query.next());

    // Add the bar set to the series
    barSeries->append(barSet);

    // Create a chart and add the bar series
    QChart *chart = new QChart();
    chart->addSeries(barSeries);
    chart->setTitle("Products with Quantity Below Minimum Stock Level");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Customize the X-axis (product names)
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Product Name");

    // Customize the Y-axis (quantity)
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Quantity");
    axisY->setLabelFormat("%d");

    // Assign axes to the chart
    chart->setAxisX(axisX, barSeries);
    chart->setAxisY(axisY, barSeries);

    // Add the chart to a ChartView
    QChartView *chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Add the ChartView to the UI layout
    ui->chart_layout->addWidget(chartView); // Ensure `chart_layout` is defined in the .ui file as a layout (e.g., QVBoxLayout)
}

Products_Hightlights_Min_Level_Chart::~Products_Hightlights_Min_Level_Chart()
{
    delete ui;
}
