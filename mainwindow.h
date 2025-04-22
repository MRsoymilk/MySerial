#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FormSerial;
class FormPlot;
class FormData;
class FormLog;

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

private slots:
    void on_btnSerial_clicked();
    void on_btnPlot_clicked();
    void on_btnData_clicked();
    void on_btnLog_clicked();

private:
    Ui::MainWindow *ui;
    FormSerial *formSerial;
    FormPlot *formPlot;
    FormData *formData;
    FormLog *formLog;
    void init();
    void initStackWidget();
    void initToolbar();
};
#endif // MAINWINDOW_H
