#ifndef FORMPLOT_H
#define FORMPLOT_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

namespace Ui {
class FormPlot;
}

class FormPlot : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlot(QWidget *parent = nullptr);
    ~FormPlot();

    void updateData(const QByteArray &data);
    QRectF calculateSeriesBounds(QLineSeries *series);
public slots:
    void onDataReceived(const QByteArray &data);

private slots:
    void on_tBtnZoom_clicked();

private:
    void init();

private:
    Ui::FormPlot *ui;
    QLineSeries *m_series;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;

    double m_time;
    const int m_maxPoints = 200;
    const double m_fs = 3600.0;
    const double m_T = 1.0 / m_fs;
    bool m_autoZoom = true;
    double m_fixedYMin = -2.5;
    double m_fixedYMax = 2.5;
};

#endif // FORMPLOT_H
