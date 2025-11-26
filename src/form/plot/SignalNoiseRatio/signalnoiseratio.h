#ifndef SIGNALNOISERATIO_H
#define SIGNALNOISERATIO_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class MyChartView;

namespace Ui {
class SignalNoiseRatio;
}

class SignalNoiseRatio : public QWidget
{
    Q_OBJECT

public:
    explicit SignalNoiseRatio(QWidget *parent = nullptr);
    ~SignalNoiseRatio();
    void calculate(const QList<QPointF> &data);

public slots:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void on_checkBoxUseIdx_checkStateChanged(const Qt::CheckState &arg1);

private:
    void init();

private:
    Ui::SignalNoiseRatio *ui;
    int m_idx;
    bool m_enableIdx = false;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QVector<double> m_data = {};
};

#endif // SIGNALNOISERATIO_H
