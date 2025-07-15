#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FormSerial;
class FormPlot;
class FormData;
class FormLog;
class FormSetting;
class FormPlayMPU6050;

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

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_btnSerial_clicked();
    void on_btnPlot_clicked();
    void on_btnData_clicked();
    void on_btnLog_clicked();
    void on_btnSetting_clicked();
    void menuLanguageSelect(QAction *selectedAction);
    void menuThemeSelect(QAction *selectedTheme);

private:
    void init();
    void initMsgBar();
    void initLanguage();
    void initTheme();
    void setLanguage(const QString &language);
    void setTheme(const QString &theme);
    void initStackWidget();
    void initToolbar();

private:
    Ui::MainWindow *ui;
    FormSerial *formSerial;
    FormPlot *formPlot;
    FormData *formData;
    FormLog *formLog;
    FormSetting *formSetting;
    FormPlayMPU6050 *playMPU6050;

    int m_currentPageIndex = 0;
    QString m_theme;
};
#endif // MAINWINDOW_H
