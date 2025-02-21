#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <register.h>
#include <dashboard.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_pushButton_clicked();

    void on_login_btn_clicked();

    //void audit_logs(int userId, const QString &username, const QString &action, const QString &details);

    void on_toggle_button_main_circle_clicked();

    void on_toggle_button_admin_circle_clicked();

private:
    Ui::MainWindow *ui;
    Register *reg;
    DashBoard *dash_board;
};
#endif // MAINWINDOW_H
