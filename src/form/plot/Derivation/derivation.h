#ifndef DERIVATION_H
#define DERIVATION_H

#include <QWidget>
#include <QtCharts>
#include "SingleCurveWindow/singlecurvewindow.h" // 确保包含此头文件

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

private slots:
    void on_tBtnExtraCurve_clicked();

private:
    void initChart();

private:
    Ui::Derivation *ui;
    QChart *m_chart;
    QChartView *m_chartView;

    QLineSeries *m_seriesData33;
    QLineSeries *m_seriesDeriv;
    QLineSeries *m_seriesData31;
    QAreaSeries *m_areaHighlight = nullptr;

    SingleCurveWindow *m_subWindow = nullptr;

    bool m_enableExtraCurve = false;

    // 【新增】缓存数据，以便按钮点击时重绘
    QList<QPointF> m_lastData31;
    QList<QPointF> m_lastData33;
};

#endif // DERIVATION_H
