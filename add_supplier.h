#ifndef ADD_SUPPLIER_H
#define ADD_SUPPLIER_H

#include <QDialog>

namespace Ui {
class Add_Supplier;
}

class Add_Supplier : public QDialog
{
    Q_OBJECT

public:
    explicit Add_Supplier(QWidget *parent = nullptr);
    ~Add_Supplier();

private slots:
    void on_register_supplier_btn_clicked();

private:
    Ui::Add_Supplier *ui;
};

#endif // ADD_SUPPLIER_H
