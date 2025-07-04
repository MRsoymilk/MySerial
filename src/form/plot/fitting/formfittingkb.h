#ifndef FORMFITTINGKB_H
#define FORMFITTINGKB_H

#include <QChart>
#include <QChartView>
#include <QLayoutItem>
#include <QLineSeries>
#include <QValueAxis>
#include <QWidget>

namespace Ui {
class FormFittingKB;
}

class FormFittingKB : public QWidget
{
    Q_OBJECT

public:
    explicit FormFittingKB(QWidget *parent = nullptr);
    ~FormFittingKB();
    int getRound();

signals:
    void sendKB(const QByteArray &kb);

private:
    void drawKB(const float &k, const float &b);
    QVector<double> smoothCenteredMovingAverage(const QVector<double> &data, int window);
    QByteArray wrapKB(const float &k, const float &b);
    bool findPeak(const int &idx_min,
                  const int &idx_max,
                  const QVector<double> &smoothed,
                  const int &min_distance,
                  const double &min_prominence,
                  const QVector<double> &v14,
                  const QVector<double> &v24);
    void fittingKB(float &avg_k, float &avg_b);
    void findV14_MaxMinIdx(const QVector<double> &v14, int &idx_min, int &idx_max);

public slots:
    void doCorrection(const QVector<double> &v14, const QVector<double> &v24);
    void on_spinBoxRound_valueChanged(int val);

private:
    Ui::FormFittingKB *ui;
    int m_current_round;
    bool m_start;
    int m_round;
    QVector<QVector<QPointF>> m_v14;
};

#endif // FORMFITTINGKB_H
