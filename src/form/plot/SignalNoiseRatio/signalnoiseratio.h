#ifndef SIGNALNOISERATIO_H
#define SIGNALNOISERATIO_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class MyChartView;

namespace Ui {
class SignalNoiseRatio;
}

class SignalNoiseRatio : public QWidget
{
    Q_OBJECT
public:
    enum IDX_SNR {
        MODE_SIGNAL_NOISE,
        MODE_SIGNAL_SIGNAL
    };
public:
    explicit SignalNoiseRatio(QWidget *parent = nullptr);
    ~SignalNoiseRatio();
    void calculate(const QList<QPointF> &data);

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_checkBoxUseIdx_checkStateChanged(const Qt::CheckState &arg1);
    void contextMenuEvent(QContextMenuEvent *event) override;
    void on_tabWidget_currentChanged(int index);
    void on_spinBoxExcludeRadius_textChanged(const QString &arg1);

private:
    void init();
    void clearChart();
    void exportChart();

private:
    Ui::SignalNoiseRatio *ui;
    int m_idx;
    bool m_enableIdx = false;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_line;
    QVector<double> m_vSignal = {};
    QVector<double> m_vNoiseStd = {};
    IDX_SNR m_tab_idx = MODE_SIGNAL_SIGNAL;
};

#endif // SIGNALNOISERATIO_H
