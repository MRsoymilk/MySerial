#ifndef FORMEASY_H
#define FORMEASY_H

#include <QFrame>
#include <QGraphicsLineItem>
#include <QLineSeries>
#include <QScatterSeries>
#include <QStandardItem>
#include <QToolButton>
#include <QValueAxis>
#include <QWidget>
#include "global.h"

class MyChartView;
class FormSerial;
class ThreadWorker;
class FormPlotSimulate;
class FormPlotHistory;
class FourierTransform;
class Derivation;
class SignalNoiseRatio;
class Accumulate;
class FormSetting;
class PeakTrajectory;
class DraggableLine;

namespace Ui {
class FormEasy;
}

class FormEasy : public QWidget
{
    Q_OBJECT

public:
    explicit FormEasy(QWidget *parent = nullptr);
    ~FormEasy();
    void retranslateUI();
    void setAlgorithm(const QString &algorithm);

signals:
    void toHistory(const MY_DATA &my_data);

public slots:
    void updatePlot4k(const MY_DATA &my_data,
                      bool record);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_tBtnSwitch_clicked();
    void on_tBtnCrop_clicked();
    void on_tBtnZoom_clicked();
    void on_tBtnPeak_clicked();
    void on_tBtnFWHM_clicked();
    void on_tBtnImg_clicked();
    void on_tBtnPause_clicked();
    void on_spinBoxIntegrationTime_valueChanged(int val);
    void doSimulateClicked();
    void doHistoryClicked();
    void on_tBtnFourier_clicked();
    void on_tBtnAccumulate_clicked();
    void doSNRClicked();
    void on_tBtnSetting_clicked();
    void on_tBtnAxisY_clicked();
    void on_tBtnToVoltage_clicked();
    void on_spinBoxYStart_valueChanged(int val);
    void on_spinBoxYEnd_valueChanged(int val);
    void on_spinBoxXStart_valueChanged(int val);
    void on_spinBoxXEnd_valueChanged(int val);
    void on_tBtnAxisX_clicked();
    void on_checkBoxPeakTrack_checkStateChanged(const Qt::CheckState &arg1);
    void on_tBtnInfo_clicked();

private:
    void initAxisControl();
    void initChart();
    void initConnectInfo();
    void initTable();
    void initToolButton();
    void init();
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
    bool connectEasyMode();
    void closeEasyMode();
    void saveChartAsImage(const QString &filePath);
    void loadChart();
    void exportChart();
    void highlightRowByX(double x);

private:
    Ui::FormEasy *ui;
    FormSerial *formSerial;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QStandardItemModel *m_modelValue;
    CURVE m_curve;

    bool m_isPlaying = false;
    bool m_enableCrop = false;
    bool m_autoZoom = true;
    bool m_enableAxisY = false;
    bool m_enableAxisX = false;
    bool m_pause = false;
    bool m_findPeak = false;
    bool m_calcFWHM = false;
    bool m_toVoltage = false;
    int m_y_start = 0;
    int m_y_end = 0;
    int m_x_start = 0;
    int m_x_end = 0;
    QScatterSeries *m_peaks;
    QList<QLineSeries *> m_fwhmLines;
    QList<QGraphicsSimpleTextItem *> m_fwhmLabels;
    ThreadWorker *m_worker;
    QThread *m_workerThread;
    FormPlotSimulate *m_plotSimulate = nullptr;
    FormPlotHistory *m_plotHistory = nullptr;
    FourierTransform *m_fourierTransform = nullptr;
    Derivation *m_derivation = nullptr;
    SignalNoiseRatio *m_snr = nullptr;
    Accumulate *m_accumulate = nullptr;
    FormSetting *m_setting = nullptr;
    PeakTrajectory *m_trajectory;
    DraggableLine *m_lineLeft = nullptr;
    DraggableLine *m_lineRight = nullptr;
    int m_trajectory_start;
    int m_trajectory_end;
    bool m_enablePeakTrack = false;
    bool m_enableFourier = false;
    bool m_enableAccumulate = false;
    bool m_enableSNR = false;
    bool m_enableSetting = false;
    bool m_enableSimulate = false;
    bool m_enableHistory = false;
    QFrame *m_infoPopup = nullptr;
    QToolButton *m_tBtnSimulate = nullptr;
    QToolButton *m_tBtnHistory = nullptr;
    QToolButton *m_tBtnSNR = nullptr;
};

#endif // FORMEASY_H
