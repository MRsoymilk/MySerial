#ifndef FORMPLOTCORRECTION_H
#define FORMPLOTCORRECTION_H

#include <QWidget>

class FormFittingKB;
class FormFittingSin;

namespace Ui {
class FormPlotCorrection;
}

class FormPlotCorrection : public QWidget
{
    Q_OBJECT
public:
    explicit FormPlotCorrection(QWidget *parent = nullptr);
    ~FormPlotCorrection();

signals:
    void windowClose();
    void sendKB(const QByteArray &bytes);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void onEpochCorrection(const QVector<double> &v14, const QVector<double> &v24);

private slots:
    void on_btnStart_clicked();
    void on_comboBoxAlgorithm_currentTextChanged(const QString &arg1);

private:
    void init();
    void findV14_MaxMinIdx(const QVector<double> &v14, int &idx_min, int &idx_max);

private:
    Ui::FormPlotCorrection *ui;
    FormFittingKB *m_formKB;
    FormFittingSin *m_formSin;
    bool m_start;
};

#endif // FORMPLOTCORRECTION_H
