#ifndef PEAKTRAJECTORY_H
#define PEAKTRAJECTORY_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "MyChartView/mychartview.h"

namespace Ui {
class PeakTrajectory;
}

class PeakTrajectory : public QWidget
{
    Q_OBJECT

public:
    explicit PeakTrajectory(QWidget *parent = nullptr);
    ~PeakTrajectory();
    void appendPeak(const int &value);

public slots:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void init();

private:
    Ui::PeakTrajectory *ui;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QVector<int> m_data = {};
    int m_range = 100;
    int m_history_min;
    int m_history_max;
};

#endif // PEAKTRAJECTORY_H
