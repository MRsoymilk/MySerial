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
    void retranslateUI();

public:
    void updatePlot(const QList<QPointF> &data,
                    const double &xMin,
                    const double &xMax,
                    const double &yMin,
                    const double &yMax,
                    const double &temperature);

signals:
    void windowClose();
    void useLoadedThreshold(bool isUse, QVector<int> v);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_tBtnPrev_clicked();
    void on_tBtnNext_clicked();
    void on_tBtnLoadData_clicked();
    void on_doubleSpinBoxOffset_valueChanged(double offset);
    void on_doubleSpinBoxStep_valueChanged(double step);

private:
    void init();
    void updateIndex();

private:
    Ui::ShowCorrectionCurve *ui;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QList<QList<QPointF>> m_data;
    int m_current_page;
    QStandardItemModel *m_model;
    bool m_load_data;
};

#endif // SHOWCORRECTIONCURVE_H
