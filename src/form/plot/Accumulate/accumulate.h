#ifndef ACCUMULATE_H
#define ACCUMULATE_H

#include <QWidget>
#include <QtCharts>

class MyChartView;

namespace Ui {
class Accumulate;
}

class Accumulate : public QWidget
{
    Q_OBJECT

public:
    explicit Accumulate(QWidget *parent = nullptr);
    ~Accumulate();
    void accumulate(const QList<QPointF> &v);

private slots:
    void on_tBtnNoiseEnable_clicked();
    void on_tBtnAccumulateEnable_clicked();

private:
    void init();
    QList<QPointF> fitSingleCurve(const QList<QPointF> &points, int order);
    void updateAvgFittedCurve(const QList<QPointF> &newFitted);
    QList<QPointF> gaussianSmooth(const QList<QPointF> &points, int window);

private:
    Ui::Accumulate *ui;
    QChart *m_chartNoise = nullptr;
    MyChartView *m_chartViewNoise = nullptr;
    QLineSeries *m_lineNoise = nullptr;
    QValueAxis *m_axisXNoise = nullptr;
    QValueAxis *m_axisYNoise = nullptr;
    QList<QList<QPointF>> m_noiseBuffer;
    QList<QLineSeries *> m_lineNoiseGroups;
    QLineSeries *m_lineNoiseFit = nullptr;
    QList<QPointF> m_avgFittedCurve;

    QChart *m_chartAcc = nullptr;
    MyChartView *m_chartViewAcc = nullptr;
    QLineSeries *m_lineAcc = nullptr;
    QValueAxis *m_axisXAcc = nullptr;
    QValueAxis *m_axisYAcc = nullptr;

    int m_smooth_window = 0;
    int m_poly_order = 0;
    int m_count_noise = 0;
    int m_count_noise_remain = 0;
    bool m_enableNoise = false;
    bool m_enableAccumulate = false;
    QList<QPointF> m_accumulatedCurve;
    QList<QPointF> m_accumulateNoise;
    int m_count_acc = 0;
};

#endif // ACCUMULATE_H
