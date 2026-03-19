#ifndef FORMEXPERT_H
#define FORMEXPERT_H

#include <QWidget>

class FormSerial;
class FormPlot;
class FormLog;
class FormPlayMPU6050;
class ThreadWorker;
class FormPlotSimulate;
class FormPlotHistory;
class FormPlotCorrection;
class FormExternal;

namespace Ui {
class FormExpert;
}

class FormExpert : public QWidget {
    Q_OBJECT

public:
    explicit FormExpert(QWidget *parent = nullptr);
    ~FormExpert();
    void retranslateUI();
    void setAlgorithm(const QString &algorithm);

private slots:
    void on_btnSerial_clicked();
    void on_btnPlot_clicked();
    void on_btnLog_clicked();
    void on_btnExternal_clicked();

    void plotHistoryClose();
    void plotSimulateClose();
    void plotCorrectionClose();

    void on_tBtnHistory_clicked();
    void on_tBtnSimulate_clicked();
    void on_tBtnCorrection_clicked();

private:
    Ui::FormExpert *ui;
    FormSerial *formSerial;
    FormPlot *formPlot;
    FormLog *formLog;
    FormPlayMPU6050 *playMPU6050;
    FormExternal *formExternal;
    QThread *m_workerThread;
    ThreadWorker *m_worker;
    FormPlotSimulate *m_plotSimulate = nullptr;
    FormPlotHistory *m_plotHistory = nullptr;
    FormPlotCorrection *m_plotCorrection = nullptr;

    int m_currentPageIndex = 0;

    bool m_showHistory = false;
    bool m_showSimulate = false;
    bool m_showCorrection = false;

    void initToolbar();
    void initStackWidget();
    void init();

protected:
    void closeEvent(QCloseEvent *event) override;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif  // FORMEXPERT_H
