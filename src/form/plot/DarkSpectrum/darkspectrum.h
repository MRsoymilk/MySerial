#ifndef DARKSPECTRUM_H
#define DARKSPECTRUM_H

#include <QWidget>

namespace Ui {
class DarkSpectrum;
}

class DarkSpectrum : public QWidget {
    Q_OBJECT

public:
    explicit DarkSpectrum(QWidget *parent = nullptr);
    ~DarkSpectrum();
    void calculate(const QVector<double> &v);

signals:
    void windowClose();
    void doCalculate(bool status);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_btnRefresh_clicked();

private:
    Ui::DarkSpectrum *ui;

private:
    QVector<double> m_lastDark;
};

#endif  // DARKSPECTRUM_H
