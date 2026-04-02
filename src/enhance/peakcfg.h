#ifndef PEAKCFG_H
#define PEAKCFG_H

#include <QWidget>

namespace Ui {
class PeakCfg;
}

class PeakCfg : public QWidget {
    Q_OBJECT

public:
    explicit PeakCfg(QWidget *parent = nullptr);
    ~PeakCfg();
    QVector<double> getCfg();

private slots:
    void on_spinBoxWindow_valueChanged(int val);
    void on_doubleSpinBoxThreshold_valueChanged(double val);
    void on_doubleSpinBoxMinDist_valueChanged(double val);

private:
    void init();

private:
    Ui::PeakCfg *ui;
    int m_window = 3;
    double m_threshold = 1.0;
    double m_min_dist = 5.0;
};

#endif  // PEAKCFG_H
