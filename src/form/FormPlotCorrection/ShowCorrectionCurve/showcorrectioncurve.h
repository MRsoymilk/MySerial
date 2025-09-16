#ifndef SHOWCORRECTIONCURVE_H
#define SHOWCORRECTIONCURVE_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "MyChartView/mychartview.h"

namespace Ui {
class ShowCorrectionCurve;
}

class ShowCorrectionCurve : public QWidget
{
    Q_OBJECT

public:
    explicit ShowCorrectionCurve(QWidget *parent = nullptr);
    ~ShowCorrectionCurve();

public:
    void updatePlot(const QList<QPointF> &data,
                    const double &xMin,
                    const double &xMax,
                    const double &yMin,
                    const double &yMax,
                    const double &temperature);

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_tBtnPrev_clicked();
    void on_tBtnNext_clicked();

private:
    void init();

private:
    Ui::ShowCorrectionCurve *ui;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QList<QList<QPointF>> m_data;
    int m_current_page;
};

#endif // SHOWCORRECTIONCURVE_H
