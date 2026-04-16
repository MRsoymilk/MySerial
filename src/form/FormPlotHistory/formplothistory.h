#ifndef FORMPLOTHISTORY_H
#define FORMPLOTHISTORY_H

#include <QWidget>

#include "MyChartView/mychartview.h"
#include "global.h"

class FormPlotData;
class ShowData;

namespace Ui {
class FormPlotHistory;
}

class FormPlotHistory : public QWidget {
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
    void sendToPlot(const MY_DATA &data, bool record = false);
public slots:
    void onHistoryRecv(const MY_DATA &my_data);
    void onTemperature(double temperature);

protected:
    void closeEvent(QCloseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void on_tBtnNext_clicked();
    void on_tBtnPrev_clicked();
    void on_lineEditGo_editingFinished();
    void on_tBtnDumpPlot_clicked();
    void on_tBtnDumpData_clicked();
    void on_tBtnToPlot_clicked();
    void onMenuRemove();
    void onMenuClear();
    void on_tBtnToPlotWith_clicked();
    void on_tBtnToVoltage_clicked();
    void on_tBtnShowData_clicked();
    void on_tBtnDumpRaw_clicked();

private:
    void updatePlot();
    void init();
    void toPlot();

private:
    Ui::FormPlotHistory *ui;
    QList<MY_DATA> m_data;
    int m_index = 0;

    QList<QString> m_frames;
    MyChartView *m_chartView = nullptr;
    QLineSeries *m_line31 = nullptr;
    QLineSeries *m_line33 = nullptr;
    QChart *m_chart = nullptr;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    bool m_enableToVoltage = false;
    ShowData *m_showData;
    bool m_enableShowData = false;
};

#endif  // FORMPLOTHISTORY_H
