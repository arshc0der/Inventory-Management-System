#ifndef PRODUCTS_LOADER_H
#define PRODUCTS_LOADER_H

#include <QDialog>
#include <QStringList>
#include <QCheckBox>

namespace Ui {
class Products_Loader;
}

class Products_Loader : public QDialog
{
    Q_OBJECT

public:
    explicit Products_Loader(QWidget *parent = nullptr);
    ~Products_Loader();

    QStringList getSelectedProductIDs();

private slots:
    void on_applyFilter_linedit_textChanged(const QString &arg1);

    void on_add_btn_clicked();

private:
    Ui::Products_Loader *ui;
    QStringList productIDs;
    QStringList productNames;
    QList<QCheckBox*> checkBoxes;
};

#endif // PRODUCTS_LOADER_H
