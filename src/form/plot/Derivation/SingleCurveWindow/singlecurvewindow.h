#ifndef SINGLECURVEWINDOW_H
#define SINGLECURVEWINDOW_H

#include <QWidget>
#include <QtCharts>

class SingleCurveWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SingleCurveWindow(QWidget *parent = nullptr);
    ~SingleCurveWindow();

    // 接收数据并刷新图表
    // linePoints: data31 的波形数据
    // lowerPoints: 计算好的区域下边界（用于高亮）
    void updateChart(const QList<QPointF> &linePoints, const QList<QPointF> &lowerPoints);

private:
    void initChart();

private:
    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_seriesLine;
    QAreaSeries *m_areaHighlight = nullptr;
};

#endif // SINGLECURVEWINDOW_H
