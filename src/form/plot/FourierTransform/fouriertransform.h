#ifndef FOURIERTRANSFORM_H
#define FOURIERTRANSFORM_H

#include <QWidget>
#include <QtCharts>

class DraggableLine;
class MyChartView;

QT_BEGIN_NAMESPACE
namespace Ui {
class FourierTransform;
}
QT_END_NAMESPACE

class FourierTransform : public QWidget
{
    Q_OBJECT

public:
    explicit FourierTransform(QWidget *parent = nullptr);
    ~FourierTransform();

    void transform(const QList<QPointF> &data);
    void setSampleRate(double rate);

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_tBtnRange_clicked();
    void on_spinBoxSampleRate_valueChanged(int rate);
    void on_tBtnAxis_clicked();
    void on_spinBoxAxisXStart_valueChanged(int val);
    void on_spinBoxAxisXEnd_valueChanged(int val);

private:
    void init();

    QList<QPointF> fftAmplitude(const QList<QPointF> &data, double sampleRate);

    QList<QPointF> ifftBandLimited(const QList<QPointF> &data,
                                   double sampleRate,
                                   double freqStart,
                                   double freqEnd);

    void updateIFFT();

private:
    Ui::FourierTransform *ui;

    QChart *m_chartFFT = nullptr;
    MyChartView *m_chartViewFFT = nullptr;
    QLineSeries *m_lineFFT = nullptr;
    QValueAxis *m_axisXFFT = nullptr;
    QValueAxis *m_axisYFFT = nullptr;

    QChart *m_chartIFFT = nullptr;
    MyChartView *m_chartViewIFFT = nullptr;
    QLineSeries *m_lineIFFT = nullptr;
    QValueAxis *m_axisXIFFT = nullptr;
    QValueAxis *m_axisYIFFT = nullptr;

    DraggableLine *m_lineLeft = nullptr;
    DraggableLine *m_lineRight = nullptr;

    bool m_enableRange = false;
    double m_start = 0.0;
    double m_end = 1000.0;
    double m_sampleRate = 1000.0;

    QList<QPointF> m_currentData;

    bool m_enableAxisX = false;
    int m_axisXStart = 0;
    int m_axisXEnd = 0;
};

#endif // FOURIERTRANSFORM_H
