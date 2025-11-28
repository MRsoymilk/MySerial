#ifndef TEMPERATUREVIEW_H
#define TEMPERATUREVIEW_H

#include <QWidget>
#include <QtCharts>

class MyChartView;

namespace Ui {
class TemperatureView;
}

class TemperatureView : public QWidget
{
    Q_OBJECT

public:
    explicit TemperatureView(QWidget *parent = nullptr);
    ~TemperatureView();
    void appendTemperature(const double &value);

signals:
    void windowClose();

private:
    void closeEvent(QCloseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void init();

private:
    Ui::TemperatureView *ui;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QVector<double> m_data = {};
    int m_range = 100;
    double m_history_min;
    double m_history_max;
};

#endif // TEMPERATUREVIEW_H
