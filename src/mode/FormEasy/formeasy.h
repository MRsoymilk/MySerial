#ifndef FORMEASY_H
#define FORMEASY_H

#include <QGraphicsLineItem>
#include <QLineSeries>
#include <QScatterSeries>
#include <QStandardItem>
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
    void toHistory(const CURVE &data31, const CURVE &data33, const double &temperature = 0.0);

public slots:
    void updatePlot4k(const CURVE &curve31,
                      const CURVE &curve33,
                      const double &temperature,
                      bool record);
private slots:
    void on_tBtnSwitch_clicked();
    void on_tBtnCrop_clicked();
    void on_tBtnZoom_clicked();
    void on_tBtnPeak_clicked();
    void on_tBtnFWHM_clicked();
    void on_tBtnImg_clicked();
    void on_tBtnPause_clicked();
    void on_spinBoxIntegrationTime_valueChanged(int val);
    void on_tBtnSimulate_clicked();
    void on_tBtnHistory_clicked();
    void on_tBtnFourier_clicked();
    void on_tBtnAccumulate_clicked();
    void on_tBtnSNR_clicked();
    void on_tBtnSetting_clicked();
    void on_tBtnAxisY_clicked();
    void on_tBtnToVoltage_clicked();

    void on_spinBoxYStart_valueChanged(int val);
    void on_spinBoxYEnd_valueChanged(int val);
    void on_spinBoxXStart_valueChanged(int val);
    void on_spinBoxXEnd_valueChanged(int val);
    void on_tBtnAxisX_clicked();

private:
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

private:
    Ui::FormEasy *ui;
    FormSerial *formSerial;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QStandardItemModel *m_modelValue;

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
    // QWidget interface
    void highlightRowByX(double x);
    bool m_enableFourier = false;
    bool m_enableAccumulate = false;
    bool m_enableSNR = false;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // FORMEASY_H
