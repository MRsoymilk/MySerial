#ifndef FORMPLOTHISTORY_H
#define FORMPLOTHISTORY_H

#include <QWidget>
#include "MyChartView/mychartview.h"
#include "global.h"

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
    void sendToPlot(const CURVE &curve31,
                    const CURVE &curve33,
                    const double &temperature,
                    bool record = false);

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void onHistoryRecv(const CURVE &curve31, const CURVE &curve33, const double &temperature);
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
    QList<CURVE> m_p31, m_p33;
    QList<double> m_temperature;
    int m_index_31 = 0, m_index_33 = 0;
    MyChartView *m_chartView31Split = nullptr;
    MyChartView *m_chartView33Split = nullptr;
    MyChartView *m_chartViewMix = nullptr;
    QLineSeries *m_lineMix31 = nullptr;
    QLineSeries *m_lineMix33 = nullptr;
    QLineSeries *m_splitLine31 = nullptr;
    QLineSeries *m_splitLine33 = nullptr;
    QChart *m_chart = nullptr;
    QChart *m_chartMix = nullptr;
    QChart *m_chart31 = nullptr;
    QChart *m_chart33 = nullptr;
    QValueAxis *m_axisMixX;
    QValueAxis *m_axisMixY;
    QValueAxis *m_axisSplit31X;
    QValueAxis *m_axisSplit31Y;
    QValueAxis *m_axisSplit33X;
    QValueAxis *m_axisSplit33Y;
};

#endif // FORMPLOTHISTORY_H
