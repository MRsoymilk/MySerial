#ifndef FORMPLOT_H
#define FORMPLOT_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "MyChartView/mychartview.h"
#include "global.h"

class DraggableLine;
class PeakTrajectory;
class FourierTransform;
class Derivation;
class Accumulate;
class SignalNoiseRatio;

namespace Ui {
class FormPlot;
}

class FormPlot : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlot(QWidget *parent = nullptr);
    ~FormPlot();
    void retranslateUI();

signals:
    void newDataReceivedLLC(const QByteArray &data31,
                            const QByteArray &data33,
                            const double &temperature);
    void newDataReceivedF30(const QByteArray &data31,
                            const QByteArray &data33,
                            const double &temperature);
    void newDataReceivedF15(const QByteArray &data31,
                            const QByteArray &data33,
                            const double &temperature);
    void sendOffset31(int val);
    void sendOffset33(int val);
    void changeFrameType(int index);
    void toHistory(const CURVE &data31, const CURVE &data33, const double &temperature = 0.0);

public slots:
    void onDataReceivedLLC(const QByteArray &data31,
                           const QByteArray &data33,
                           const double temperature);
    void onDataReceivedF30(const QByteArray &data31,
                           const QByteArray &data33,
                           const double &temperature);
    void onDataReceivedF15(const QByteArray &data31,
                           const QByteArray &data33,
                           const double &temperature);
    void updatePlot4k(const CURVE &curve31,
                      const CURVE &curve33,
                      const double &temperature,
                      bool record = false);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_tBtnCrop_clicked();
    void on_tBtnZoom_clicked();
    void on_spinBox31Offset_valueChanged(int val);
    void on_spinBox33Offset_valueChanged(int val);
    void on_comboBoxAlgorithm_currentIndexChanged(int index);
    void on_tBtnImgSave_clicked();
    void on_spinBoxStartX_valueChanged(int val);
    void on_spinBoxEndX_valueChanged(int val);
    void on_spinBoxStartY_valueChanged(int val);
    void on_spinBoxEndY_valueChanged(int val);
    void on_dSpinBoxStep_valueChanged(double arg1);
    void on_tBtnStep_clicked();
    void on_tBtnFindPeak_clicked();
    void on_tBtnPause_clicked();
    void on_tBtnOffset_clicked();
    void on_tBtnFWHM_clicked();
    void on_checkBoxTrajectory_clicked();
    void on_tBtnRangeX_clicked();
    void on_tBtnRangeY_clicked();
    void on_tBtnFourier_clicked();
    void on_tBtnDerivation_clicked();
    void on_tBtnAccumulate_clicked();
    void on_tBtnSNR_clicked();

private:
    void init();
    void init2d();
    void initToolButtons();
    void getINI();
    void setINI();
    void updatePlot2d(const QList<QPointF> &data31,
                      const QList<QPointF> &data33,
                      const double &xMin,
                      const double &xMax,
                      const double &yMin,
                      const double &yMax);
    void saveChartAsImage(const QString &filePath);
    QVector<QPointF> findPeak(int window, double thresholdFactor, double minDist);
    void callFindPeak();
    void callCalcFWHM();
    void peakTrajectory(const QVector<QPointF> &peaks);

private:
    Ui::FormPlot *ui;
    QLineSeries *m_series33;
    QLineSeries *m_series31;
    QScatterSeries *m_peaks;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    bool m_enableCrop = false;
    bool m_autoZoom = true;

    double m_fixedYMin = -2.5;
    double m_fixedYMax = 2.5;
    double m_step = 1;

    MyChartView *m_chartView = nullptr;
    bool m_findPeak = false;
    bool m_findFWHM = false;
    bool m_pause = false;
    bool m_enableFourier = false;
    bool m_enableDerivation = false;
    bool m_enableAccumulate = false;
    bool m_enableSNR = false;

private:
    QList<QLineSeries *> m_fwhmLines;
    QList<QGraphicsSimpleTextItem *> m_fwhmLabels;
    PeakTrajectory *m_trajectory = nullptr;
    FourierTransform *m_fourierTransform = nullptr;
    Derivation *m_derivation = nullptr;
    SignalNoiseRatio *m_snr = nullptr;
    Accumulate *m_accumulate = nullptr;
    DraggableLine *m_lineLeft = nullptr;
    DraggableLine *m_lineRight = nullptr;
    int m_trajectory_start;
    int m_trajectory_end;
};

#endif // FORMPLOT_H
