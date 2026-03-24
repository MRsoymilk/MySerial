#ifndef FORMPRODUCE_H
#define FORMPRODUCE_H

#include <QChart>
#include <QLabel>
#include <QLineSeries>
#include <QValueAxis>
#include <QWidget>

#include "global.h"

class MyChartView;
class FormSerial;
class ThreadWorker;
class FormFittingPoints;

namespace Ui {
class FormProduce;
}

class FormProduce : public QWidget {
    Q_OBJECT

public:
    explicit FormProduce(QWidget *parent = nullptr);
    ~FormProduce();
    void retranslateUI();
    void setAlgorithm(const QString &algorithm);

public slots:
    void updatePlot4k(const MY_DATA &my_data, bool record = true);

private slots:
    void on_tBtnSwitch_clicked();
    void on_btnWriteDeviceSerial_clicked();
    void on_btnQueryDeviceSerial_clicked();
    void on_tBtnDoneDeviceSerial_clicked();
    void on_btnWriteBaseline_clicked();
    void on_btnQueryBaseline_clicked();
    void on_tBtnDoneBaseline_clicked();
    void on_btnStartCorrection_clicked();
    void on_tBtnDoneCorrection_clicked();
    void on_btnStartSelfCheck_clicked();
    void on_tBtnDoneSelfCheck_clicked();
    void on_tBtnToVoltage_clicked();
    void on_tBtnPause_clicked();

private:
    void init();
    bool connectProduceMode();
    void closeProduceMode();
    void updatePlot2d(const QList<QPointF> &data31, const QList<QPointF> &data33);
    void updateAxis();

private:
    Ui::FormProduce *ui;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_series33;
    QLineSeries *m_series31;
    FormSerial *formSerial;
    QThread *m_workerThread;
    ThreadWorker *m_worker;
    FormFittingPoints *m_formFittingPoints = nullptr;
    bool m_isPlaying = false;
    bool m_enableVoltage = false;
    bool m_enableFitting = false;

    // QWidget interface
    void makeTabTodo();
    QVector<bool> m_tabDone;
    void updateTabStyle();
    void initTabUI();
    QVector<QWidget *> m_tabWidgets;
    QVector<QLabel *> m_tabLabels;
    bool m_pause = false;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif  // FORMPRODUCE_H
