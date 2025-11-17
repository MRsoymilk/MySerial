#ifndef FORMPLOTHISTORY_H
#define FORMPLOTHISTORY_H

#include <QWidget>
#include "MyChartView/mychartview.h"

namespace Ui {
class FormPlotHistory;
}

class FormPlotHistory : public QWidget
{
    Q_OBJECT
public:
    const int INDEX_31 = 1;
    const int INDEX_33 = 2;

public:
    explicit FormPlotHistory(QWidget *parent = nullptr);
    ~FormPlotHistory();
    void retranslateUI();

signals:
    void windowClose();
    void sendToPlot(const QList<QPointF> &data31,
                    const QList<QPointF> &data33,
                    const double &xMin,
                    const double &xMax,
                    const double &yMin,
                    const double &yMax,
                    const double &temperature,
                    bool record = false);

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void onHistoryRecv(const QList<QPointF> &data31,
                       const QList<QPointF> &data33,
                       const double &temperature);
    void onTemperature(double temperature);

private slots:
    void on_tBtnNext14_clicked();
    void on_tBtnPrev14_clicked();
    void on_tBtnNext24_clicked();
    void on_tBtnPrev24_clicked();
    void on_lineEdit14Go_editingFinished();
    void on_lineEdit24Go_editingFinished();
    void on_radioButtonMix_clicked();
    void on_radioButtonSplit_clicked();
    void on_toolButtonDumpPlot_clicked();
    void on_toolButtonDumpData_clicked();

    void on_tBtnToPlot_clicked();

private:
    void updatePlot31();
    void updatePlot33();
    void updatePlot(int index = 0);
    void init();
    void getFittingChart();

private:
    Ui::FormPlotHistory *ui;
    QList<QList<QPointF>> m_p31, m_p33;
    QList<double> m_temperature;
    int m_index_31, m_index_33;
    MyChartView *m_chartView31Split = nullptr;
    MyChartView *m_chartView33Split = nullptr;
    MyChartView *m_chartMix = nullptr;
    QChart *m_chart = nullptr;
    QWidget *m_widgetSplit = nullptr;
    QWidget *m_widgetMix = nullptr;
};

#endif // FORMPLOTHISTORY_H
