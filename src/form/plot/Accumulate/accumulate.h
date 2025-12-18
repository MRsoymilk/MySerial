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
    QList<QPointF> accumulate(const QList<QPointF> &v);

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void on_tBtnNoiseEnable_clicked();
    void on_tBtnAccumulateEnable_clicked();
    void on_tBtnBaselineDeductionEnable_clicked();
    void on_spinBoxCount_valueChanged(int arg1);

private:
    void init();
    QList<QPointF> fitSingleCurve(const QList<QPointF> &points, int order);
    void updateAvgFittedCurve(const QList<QPointF> &newFitted);
    QList<QPointF> gaussianSmooth(const QList<QPointF> &points, int window);
    void onMenuClearNose();
    void onMenuClearAccumulate();

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

    int m_smooth_window = 0;
    int m_poly_order = 0;
    int m_count_noise = 0;
    int m_count_noise_remain = 0;
    bool m_enableNoise = false;
    bool m_enableAccumulate = false;
    bool m_enableBaselineDeduction = false;
    int m_avgFitCount = 0;
    QList<QPointF> m_accumulatedCurve;
    QList<QPointF> m_accumulateNoise;
    int m_count_acc = 0;
    int m_target_count = 0;
};

#endif // ACCUMULATE_H
