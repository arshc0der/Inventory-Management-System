#include "orders_trends.h"
#include "ui_orders_trends.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDate>

Orders_Trends::Orders_Trends(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Orders_Trends)
{
    ui->setupUi(this);
    // Create a line series for the order count data
    QLineSeries *lineSeries = new QLineSeries();

    // Database query to fetch order trends
    QSqlQuery query;
    if (!query.exec("SELECT DATE(order_date) AS OrderDate, COUNT(*) AS OrderCount FROM Orders GROUP BY OrderDate;")) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return;
    }

    // Check if query has results
    if (!query.next()) {
        qDebug() << "No order data found.";
        return;
    }

    // Populate the series with data
    QCategoryAxis *axisX = new QCategoryAxis(); // X-axis for dates
    int pointCount = 0; // Counter for points added
    do {
        QDate orderDate = query.value(0).toDate();
        int orderCount = query.value(1).toInt();

        // Append data to the line series
        lineSeries->append(pointCount, orderCount);

        // Add date label to the X-axis
        axisX->append(orderDate.toString("yyyy-MM-dd"), pointCount);
        ++pointCount;
    } while (query.next());

    // Create a chart and add the series
    QChart *chart = new QChart();
    chart->addSeries(lineSeries);
    chart->setTitle("Order Count Trends Over Time");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Customize the X-axis
    axisX->setTitleText("Order Date");
    axisX->setLabelsAngle(-45);
    axisX->setGridLineVisible(true);

    // Customize the Y-axis
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Order Count");
    axisY->setLabelFormat("%d");
    axisY->setGridLineVisible(true);

    // Assign axes to the chart
    chart->setAxisX(axisX, lineSeries);
    chart->setAxisY(axisY, lineSeries);

    // Add the chart to a ChartView
    QChartView *chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Add the ChartView to the UI layout
    ui->chart_layout->addWidget(chartView); // Ensure `chart_layout` is defined in the .ui file as a layout (e.g., QVBoxLayout)
}

Orders_Trends::~Orders_Trends()
{
    delete ui;
}
