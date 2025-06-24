#ifndef FORMPLOTCORRECTION_H
#define FORMPLOTCORRECTION_H

#include <QWidget>

namespace Ui {
class FormPlotCorrection;
}

class FormPlotCorrection : public QWidget
{
    Q_OBJECT
public:
    struct INI_CORRECTION
    {
        int round;
    };

public:
    explicit FormPlotCorrection(QWidget *parent = nullptr);
    ~FormPlotCorrection();

signals:
    void windowClose();
    void sendKB(const QByteArray &bytes);

protected:
    void closeEvent(QCloseEvent *event);
public slots:
    void onEpochCorrection(const QVector<double> &v14, const QVector<double> &v24);
private slots:
    void on_btnStart_clicked();
    void on_spinBoxRound_valueChanged(int val);

private:
    void init();
    void drawKB(const float &k = .0, const float &b = .0);
    QVector<double> smoothCenteredMovingAverage(const QVector<double> &data, int window);
    QByteArray wrapKB(const float &k, const float &b);
    void fittingKB(float &avg_k, float &avg_b);
    void findV14_MaxMinIdx(const QVector<double> &v14, int &idx_min, int &idx_max);
    bool findPeak(const int &idx_min,
                  const int &idx_max,
                  const QVector<double> &smoothed,
                  const int &min_distance,
                  const double &min_prominence,
                  const QVector<double> &v14,
                  const QVector<double> &v24);

private:
    Ui::FormPlotCorrection *ui;
    INI_CORRECTION m_ini;
    int m_current_round;
    bool m_start;
    QVector<QVector<QPointF>> m_v14;
};

#endif // FORMPLOTCORRECTION_H
