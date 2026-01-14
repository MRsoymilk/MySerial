#ifndef FORMEXPERT_H
#define FORMEXPERT_H

#include <QWidget>

class FormSerial;
class FormPlot;
class FormData;
class FormLog;
class FormSetting;
class FormPlayMPU6050;
class AutoUpdate;
class ThreadWorker;
class FormPlotSimulate;
class FormPlotData;
class FormPlotHistory;
class FormPlotCorrection;
class FormExternal;

namespace Ui {
class FormExpert;
}

class FormExpert : public QWidget
{
    Q_OBJECT

public:
    explicit FormExpert(QWidget *parent = nullptr);
    ~FormExpert();
    void retranslateUI();
    void setAlgorithm(const QString &algorithm);

private slots:
    void on_btnSerial_clicked();
    void on_btnPlot_clicked();
    void on_btnData_clicked();
    void on_btnLog_clicked();
    void on_btnSetting_clicked();
    void on_btnUpdate_clicked();
    void on_btnExternal_clicked();

    void plotDataClose();
    void plotHistoryClose();
    void plotSimulateClose();
    void plotCorrectionClose();

    void on_tBtnData_clicked();
    void on_tBtnHistory_clicked();
    void on_tBtnSimulate_clicked();
    void on_tBtnCorrection_clicked();

private:
    Ui::FormExpert *ui;
    FormSerial *formSerial;
    FormPlot *formPlot;
    FormData *formData;
    FormLog *formLog;
    FormSetting *formSetting;
    FormPlayMPU6050 *playMPU6050;
    FormExternal *formExternal;
    AutoUpdate *formAutoUpdate;
    QThread *m_workerThread;
    ThreadWorker *m_worker;
    FormPlotSimulate *m_plotSimulate = nullptr;
    FormPlotData *m_plotData = nullptr;
    FormPlotHistory *m_plotHistory = nullptr;
    FormPlotCorrection *m_plotCorrection = nullptr;

    int m_currentPageIndex = 0;

    bool m_showData = false;
    bool m_showHistory = false;
    bool m_showSimulate = false;
    bool m_showCorrection = false;

    // QWidget interface
    void initToolbar();
    void initStackWidget();
    void init();

protected:
    void closeEvent(QCloseEvent *event) override;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // FORMEXPERT_H
