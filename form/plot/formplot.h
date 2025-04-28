#ifndef FORMPLOT_H
#define FORMPLOT_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "formplotdata.h"
#include "plotworker.h"

namespace Ui {
class FormPlot;
}

class FormPlot : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlot(QWidget *parent = nullptr);
    ~FormPlot();

signals:
    void newDataReceived(const QByteArray &data);

private slots:
    void on_tBtnZoom_clicked();
    void on_tBtnData_clicked();
    void plotDataClose();
    void updatePlot(QLineSeries *line,
                    const int &points,
                    const double &min_y,
                    const double &max_y,
                    const double &min_x,
                    const double &max_x);

public slots:
    void onDataReceived(const QByteArray &data);

private:
    void init();

private:
    Ui::FormPlot *ui;
    QLineSeries *m_series;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    bool m_autoZoom = true;
    bool m_showData = false;
    double m_fixedYMin = -2.5;
    double m_fixedYMax = 2.5;
    QThread *m_workerThread;
    PlotWorker *m_worker;
    FormPlotData *m_plotdata;

    // QWidget interface
protected:
    void wheelEvent(QWheelEvent *event) override;
};

#endif // FORMPLOT_H
