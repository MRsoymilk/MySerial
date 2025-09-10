#ifndef FORMPLOT_H
#define FORMPLOT_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "3d/myglcurvewidget.h"
#include "mychartview.h"

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
    void newDataReceived4k(const QByteArray &data14,
                           const QByteArray &data24,
                           const double &temperature);
    void sendKB(const QByteArray &bytes);
    void sendSin(const QByteArray &bytes);
    void sendOffset14(int val);
    void sendOffset24(int val);
    void changeFrameType(int index);
    void toHistory(const QList<QPointF> &data14,
                   const QList<QPointF> &data24,
                   const double &temperature = 0.0);

public slots:
    void onDataReceived4k(const QByteArray &data14,
                          const QByteArray &data24,
                          const double &temperature);
    void updatePlot4k(const QList<QPointF> &data14,
                      const QList<QPointF> &data24,
                      const double &xMin,
                      const double &xMax,
                      const double &yMin,
                      const double &yMax,
                      const double &temperature,
                      bool record = false);

protected:
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void on_tBtnCrop_clicked();
    void on_tBtnZoom_clicked();
    void on_tBtn3D_clicked();

    void on_spinBox14Offset_valueChanged(int val);
    void on_spinBox24Offset_valueChanged(int val);
    void on_comboBoxAlgorithm_currentIndexChanged(int index);
    void on_tBtnImgSave_clicked();
    void on_spinBoxFrom_valueChanged(int val);
    void on_spinBoxTo_valueChanged(int val);
    void on_dSpinBoxStep_valueChanged(double arg1);
    void on_tBtnStep_clicked();
    void on_tBtnFindPeak_clicked();
    void on_tBtnPause_clicked();

private:
    void init();
    void init2d();
    void init3d();
    void initToolButtons();
    void getINI();
    void setINI();
    void updatePlot2d(const QList<QPointF> &data14,
                      const QList<QPointF> &data24,
                      const double &xMin,
                      const double &xMax,
                      const double &yMin,
                      const double &yMax);
    void updatePlot3d(const QList<QPointF> &data14,
                      const QList<QPointF> &data24,
                      const double &xMin,
                      const double &xMax,
                      const double &yMin,
                      const double &yMax);
    void saveChartAsImage(const QString &filePath);

private:
    Ui::FormPlot *ui;
    QLineSeries *m_series24;
    QLineSeries *m_series14;
    QScatterSeries *m_scatter;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    bool m_enableCrop = false;
    bool m_autoZoom = true;

    bool m_show3D = false;

    double m_fixedYMin = -2.5;
    double m_fixedYMax = 2.5;
    double m_step = 1;

    MyChartView *m_chartView = nullptr;
    MyGLCurveWidget *m_glWidget = nullptr;

    bool m_findPeak = false;
    QVector<QGraphicsSimpleTextItem *> m_peakLabels;

    bool m_pause = false;
};

#endif // FORMPLOT_H
