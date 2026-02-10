#ifndef PEAKTRAJECTORY_H
#define PEAKTRAJECTORY_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class MyChartView;

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

signals:
    void windowClose();
    void broadcast(const double &avg);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void on_tBtnAxisY_clicked();
    void on_spinBoxStartY_valueChanged(int arg1);
    void on_spinBoxEndY_valueChanged(int arg1);
    void getSelect(const QPointF &point);
    void on_tBtnBroadcast_clicked();

private:
    void init();
    void onRemoveCurrentPoint();
    void clearChart();

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
    bool m_enableAxisY = false;
    int m_y_start = 0;
    int m_y_end = 0;
    QPointF m_point;
};

#endif // PEAKTRAJECTORY_H
