#ifndef FORMPLOTHISTORY_H
#define FORMPLOTHISTORY_H

#include <QWidget>
#include "mychartview.h"

namespace Ui {
class FormPlotHistory;
}

class FormPlotHistory : public QWidget
{
    Q_OBJECT
public:
    const int INDEX_14 = 1;
    const int INDEX_24 = 2;

public:
    explicit FormPlotHistory(QWidget *parent = nullptr);
    ~FormPlotHistory();
    void retranslateUI();

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void onHistoryRecv(const QList<QPointF> &data14, const QList<QPointF> &data24);
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
    void on_tBtnFittingSin_clicked();
    void on_tBtnFittingKB_clicked();

private:
    void updatePlot14();
    void updatePlot24();
    void updatePlot(int index = 0);
    void init();
    void clearFitting();
    void drawFitting();
    void getFittingChart();

private:
    Ui::FormPlotHistory *ui;
    QList<QList<QPointF>> m_p14, m_p24;
    int m_index_14, m_index_24;
    MyChartView *m_chartView14Split = nullptr;
    MyChartView *m_chartView24Split = nullptr;
    MyChartView *m_chartMix = nullptr;
    QChart *m_chart = nullptr;
    QWidget *m_widgetSplit = nullptr;
    QWidget *m_widgetMix = nullptr;
    bool m_fitting;
    void drawFittingKB();
    void drawFittingSin();
};

#endif // FORMPLOTHISTORY_H
