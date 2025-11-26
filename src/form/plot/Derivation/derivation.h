#ifndef DERIVATION_H
#define DERIVATION_H

#include <QWidget>
#include <QtCharts>

namespace Ui {
class Derivation;
}

class Derivation : public QWidget
{
    Q_OBJECT

public:
    explicit Derivation(QWidget *parent = nullptr);
    ~Derivation();

    void derivation(const QList<QPointF> &data31, const QList<QPointF> &data33);

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initChart();

private:
    Ui::Derivation *ui;
    QChart *m_chart;
    QChartView *m_chartView;

    QLineSeries *m_seriesData33;
    QLineSeries *m_seriesDeriv;
    QLineSeries *m_seriesData31;
    QAreaSeries *m_areaHighlight = nullptr;

    QList<QPointF> m_lastData31;
    QList<QPointF> m_lastData33;
};

#endif // DERIVATION_H
