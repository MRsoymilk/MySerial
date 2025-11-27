#ifndef DERIVATION_H
#define DERIVATION_H

#include <QWidget>
#include <QtCharts>
#include "global.h"

namespace Ui {
class Derivation;
}

class Derivation : public QWidget
{
    Q_OBJECT

public:
    explicit Derivation(QWidget *parent = nullptr);
    ~Derivation();

    void derivation(const QList<QPointF> &data31, const QList<QPointF> &data33);

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_tBtnNextExtra_clicked();
    void on_tBtnPrevExtra_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void on_spinBoxMinLength_valueChanged(int arg1);
    void on_tBtnFindPeak_clicked();

private:
    void initChart();
    void updateExtraCurve();

private:
    Ui::Derivation *ui;
    QChart *m_chart;
    QChartView *m_chartView;

    QLineSeries *m_seriesData33;
    QLineSeries *m_seriesDeriv;
    QLineSeries *m_seriesData31;
    QAreaSeries *m_areaHighlight = nullptr;

    QList<QPointF> m_lastData31;
    QList<QPointF> m_lastData33;

    QChart *m_chartExtra;
    QChartView *m_chartViewExtra;
    QLineSeries *m_seriesExtra31;
    QLineSeries *m_seriesExtra33;
    QScatterSeries *m_peakMarker = nullptr;

    struct DOUBLE_CURVE
    {
        CURVE curve31;
        CURVE curve33;
    };

    QList<DOUBLE_CURVE> m_curve;
    int m_current_extra = -1;
    void callFindPeak(const QList<QPointF> &points31, const QList<QPointF> &points33);
    QValueAxis *m_axisExtraX = nullptr;
    QValueAxis *m_axisExtraY = nullptr;
    bool m_enableFindPeak = false;
};

#endif // DERIVATION_H
