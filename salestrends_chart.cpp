#include "salestrends_chart.h"
#include "ui_salestrends_chart.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDate>

SalesTrends_chart::SalesTrends_chart(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SalesTrends_chart)
{
    ui->setupUi(this);

    // Create a line series for the sales data
    QLineSeries *lineSeries = new QLineSeries();

    // Database query to fetch sales trends
    QSqlQuery query;
    if (!query.exec("SELECT DATE(sale_date) AS SaleDate, SUM(total_price) AS TotalSales FROM Sales GROUP BY SaleDate;")) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return;
    }

    // Check if query has results
    if (!query.next()) {
        qDebug() << "No sales data found.";
        return;
    }

    // Populate the series with data
    QCategoryAxis *axisX = new QCategoryAxis(); // X-axis for dates
    int pointCount = 0; // Counter for points added
    do {
        QDate saleDate = query.value(0).toDate();
        double totalSales = query.value(1).toDouble();

        // Append data to the line series
        lineSeries->append(pointCount, totalSales);

        // Add date label to the X-axis
        axisX->append(saleDate.toString("yyyy-MM-dd"), pointCount);
        ++pointCount;
    } while (query.next());

    // Create a chart and add the series
    QChart *chart = new QChart();
    chart->addSeries(lineSeries);
    chart->setTitle("Sales Trends Over Time");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Customize the X-axis
    axisX->setTitleText("Date");
    axisX->setLabelsAngle(-45);
    axisX->setGridLineVisible(true);

    // Customize the Y-axis
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Total Sales");
    axisY->setLabelFormat("%.2f");
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

SalesTrends_chart::~SalesTrends_chart()
{
    delete ui;
}
