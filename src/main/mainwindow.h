#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MyChartView/mychartview.h"
#include "global.h"

class FormSerial;
class FormPlot;
class FormData;
class FormLog;
class FormSetting;
class FormPlayMPU6050;
class ThreadWorker;
class FormPlotSimulate;
class FormPlotData;
class FormPlotHistory;
class FormPlotCorrection;

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
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_btnSerial_clicked();
    void on_btnPlot_clicked();
    void on_btnData_clicked();
    void on_btnLog_clicked();
    void on_btnSetting_clicked();
    void on_tBtnData_clicked();
    void on_tBtnHistory_clicked();
    void on_tBtnSimulate_clicked();
    void on_tBtnCorrection_clicked();
    void menuLanguageSelect(QAction *selectedAction);
    void menuThemeSelect(QAction *selectedTheme);

    void plotDataClose();
    void plotHistoryClose();
    void plotSimulateClose();
    void plotCorrectionClose();

    void on_actionEasy_triggered();
    void on_actionExpert_triggered();
    void on_tBtnSwitch_clicked();
    void on_tBtnCrop_clicked();
    void on_tBtnZoom_clicked();
    void on_tBtnPeak_clicked();
    void on_tBtnFWHM_clicked();
    void on_tBtnImg_clicked();
    void on_tBtnPause_clicked();
    void on_spinBox_valueChanged(int arg1);

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
    QThread *m_workerThread;
    ThreadWorker *m_worker;
    FormPlotSimulate *m_plotSimulate = nullptr;
    FormPlotData *m_plotData = nullptr;
    FormPlotHistory *m_plotHistory = nullptr;
    FormPlotCorrection *m_plotCorrection = nullptr;

    int m_currentPageIndex = 0;
    QString m_theme;
    QPixmap m_background;

    bool m_showData = false;
    bool m_showHistory = false;
    bool m_showSimulate = false;
    bool m_showCorrection = false;

    // EasyMode
private:
    void initEasyMode();
    bool connectEasyMode();
    void closeEasyMode();
    void saveChartAsImage(const QString &filePath);
    void updatePlot(const CURVE &curve31,
                    const CURVE &curve33,
                    const double &temperature = 0.0,
                    bool record = true);
    void updateTable(const QVector<double> &v14,
                     const QVector<double> &v24,
                     const QVector<double> &raw14,
                     const QVector<double> &raw24);
    void callFindPeak();
    void callCalcFWHM();
    QString calcIntegrationTime(int value);
    QVector<QPointF> findPeak(int window, double thresholdFactor, double minDist);

    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QStandardItemModel *m_modelValue;
    QMovie *m_movie = nullptr;
    bool m_isPlaying = false;
    bool m_enableCrop = false;
    bool m_autoZoom = true;
    bool m_pause = false;
    bool m_findPeak = false;
    bool m_calcFWHM = false;
    QScatterSeries *m_peaks;
    QList<QLineSeries *> m_fwhmLines;
    QList<QGraphicsSimpleTextItem *> m_fwhmLabels;
};
#endif // MAINWINDOW_H
