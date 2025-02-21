#ifndef CHECKDISCOUNT_H
#define CHECKDISCOUNT_H

#include <QDialog>
#include <QString>

namespace Ui {
class CheckDiscount;
}

class CheckDiscount : public QDialog
{
    Q_OBJECT

public:
    explicit CheckDiscount(QWidget *parent = nullptr);
    ~CheckDiscount();

    // New method to return the selected discount value
    QString getSelectedDiscount() const;

private slots:
    void on_apply_discount_btn_clicked();

private:
    Ui::CheckDiscount *ui;
    QString selectedDiscount;  // Store the selected discount
};

#endif // CHECKDISCOUNT_H
