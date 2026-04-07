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
#include "keydef.h"
#include "findfwhm.h"

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
class PointsTracker;
class DarkSpectrum;
class LoadingOverLay;
class PeakCfg;

namespace Ui {
class FormEasy;
}

class FormEasy : public QWidget {
    Q_OBJECT
public:
    enum EASY_OPT {
        EASY_HANDSHAKE,
        EASY_SET_INTEGRATION_TIME,
        EASY_DO_THRESHOLD,
        EASY_DO_BASELINE,
        EASY_DATA_REQUEST,
        EASY_DISCONNECT,
        EASY_CONNECT,
        EASY_CONNECT_SUCCESS,
        EASY_CONNECT_STOP,
    };
public:
    explicit FormEasy(QWidget *parent = nullptr);
    ~FormEasy();
    void retranslateUI();
    void setAlgorithm(const QString &algorithm);

signals:
    void toHistory(const MY_DATA &my_data);
    void sendOption(const QJsonObject &option);
    void recvThreshold(bool isUse, const QList<double> &values);
    void recvThresholdOption(const QJsonObject &option);
    void initThreshold();

public slots:
    void updatePlot4k(const MY_DATA &my_data, bool record);

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
    void on_tBtnFourier_clicked();
    void on_tBtnAccumulate_clicked();
    void on_tBtnAxisY_clicked();
    void on_tBtnToVoltage_clicked();
    void on_spinBoxYStart_valueChanged(int val);
    void on_spinBoxYEnd_valueChanged(int val);
    void on_spinBoxXStart_valueChanged(int val);
    void on_spinBoxXEnd_valueChanged(int val);
    void on_tBtnAxisX_clicked();
    void on_checkBoxPeakTrack_checkStateChanged(const Qt::CheckState &state);
    void on_tBtnInfo_clicked();
    void on_comboBoxTimeUnit_currentIndexChanged(int index);
    void doSimulateClicked();
    void doHistoryClicked();
    void doSNRClicked();
    void doDarkSpectrum();
    void doPointsTracker();

private:
    void initAxisControl();
    void initChart();
    void initConnectInfo();
    void initTable();
    void initToolButton();
    void init();
    void updatePlot(const CURVE &curve31, const CURVE &curve33, const double &temperature = 0.0, bool record = true);
    void updateTable(const QList<QPointF> &data, const QList<QPointF> &data_raw);
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
    void addToPointsTracker();
    void clearPointsTracker();

private:
    Ui::FormEasy *ui;
    FormSerial *formSerial;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QStandardItemModel *m_modelValue;
    MY_DATA m_data;

    bool m_isPlaying = false;
    bool m_enableCrop = false;
    bool m_autoZoom = true;
    bool m_enableAxisY = false;
    bool m_enableAxisX = false;
    bool m_pause = false;
    PeakCfg *m_peakCfg;
    bool m_findPeak = false;
    bool m_calcFWHM = false;
    bool m_toVoltage = false;
    int m_y_start = 0;
    int m_y_end = 0;
    int m_x_start = 0;
    int m_x_end = 0;
    QScatterSeries *m_peaks;
    QList<FWHMResult> m_fwhmResults;
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
    PointsTracker *m_pointsTracker = nullptr;
    DarkSpectrum *m_darkSpectrum = nullptr;
    PeakTrajectory *m_trajectory;
    DraggableLine *m_lineLeft = nullptr;
    DraggableLine *m_lineRight = nullptr;
    int m_trajectory_start;
    int m_trajectory_end;
    bool m_enablePeakTrack = false;
    bool m_enableFourier = false;
    bool m_enableFourierPercent = false;
    bool m_enableAccumulate = false;
    bool m_enableSNR = false;
    bool m_enableSetting = false;
    bool m_enableSimulate = false;
    bool m_enableHistory = false;
    bool m_enablePointsTracker = false;
    bool m_enableDarkSpectrum = false;
    bool m_doDarkSpectrumCalc = true;
    QFrame *m_infoPopup = nullptr;
    QToolButton *m_tBtnSimulate = nullptr;
    QToolButton *m_tBtnHistory = nullptr;
    QToolButton *m_tBtnSNR = nullptr;
    QToolButton *m_tBtnPointsTracker = nullptr;
    QToolButton *m_tBtnDarkSpectrum = nullptr;
    QVector<double> m_vPointsTracker;
    void sendIntegrationTime();
    LoadingOverLay *m_overlay = nullptr;
    QString m_F30_shown_mode = CFG_F30_MODE_DOUBLE;
    void clearFWHM();
    void drawFWHM(double xPeak, double xLeft, double xRight, double yHalf);

    // QWidget interface
    void updateFWHMLabels();
protected:
    void resizeEvent(QResizeEvent *event);
};

#endif  // FORMEASY_H
