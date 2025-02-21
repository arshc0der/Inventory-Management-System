#ifndef REGISTERCUSTOMER_H
#define REGISTERCUSTOMER_H

#include <QDialog>

namespace Ui {
class RegisterCustomer;
}

class RegisterCustomer : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterCustomer(QWidget *parent = nullptr);
    ~RegisterCustomer();

private slots:
    void on_register_customer_btn_clicked();

private:
    Ui::RegisterCustomer *ui;
};

#endif // REGISTERCUSTOMER_H
