#ifndef ADDPROMOTION_H
#define ADDPROMOTION_H

#include <QMainWindow>
#include <products_loader.h>

namespace Ui {
class AddPromotion;
}

class AddPromotion : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddPromotion(QWidget *parent = nullptr);
    ~AddPromotion();

private slots:
    void on_select_product_id_btn_clicked();

    void on_add_promotion_btn_clicked();

private:
    Ui::AddPromotion *ui;
    Products_Loader *ProductsLoader;
};

#endif // ADDPROMOTION_H
