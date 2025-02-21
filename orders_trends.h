#ifndef ORDERS_TRENDS_H
#define ORDERS_TRENDS_H

#include <QWidget>

namespace Ui {
class Orders_Trends;
}

class Orders_Trends : public QWidget
{
    Q_OBJECT

public:
    explicit Orders_Trends(QWidget *parent = nullptr);
    ~Orders_Trends();

private:
    Ui::Orders_Trends *ui;
};

#endif // ORDERS_TRENDS_H
