#ifndef ADDADJUSTMENT_H
#define ADDADJUSTMENT_H

#include <QMainWindow>

namespace Ui {
class AddAdjustment;
}

class AddAdjustment : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddAdjustment(QWidget *parent = nullptr);
    ~AddAdjustment();

private slots:
    void on_Adjustments_btn_clicked();

private:
    Ui::AddAdjustment *ui;

    void load_products();
};

#endif // ADDADJUSTMENT_H
