#ifndef THREADWORKER_H
#define THREADWORKER_H

#include <QLineSeries>
#include <QObject>

class ThreadWorker : public QObject
{
    Q_OBJECT
public:
    explicit ThreadWorker(QObject *parent = nullptr);
    ~ThreadWorker();
    void setOffset14(const int &offset);
    void setOffset24(const int &offset);
    void setAlgorithm(int);

public slots:
    void processData4k(const QByteArray &data14,
                       const QByteArray &data24,
                       const double &temperature);
    void onEnableCorrection(bool enable, const QJsonObject &params);

signals:
    void pointsReady4k(const QVector<double> &v14,
                       const QVector<double> &v24,
                       const QVector<qint32> &raw14,
                       const QVector<qint32> &raw24);
    void dataReady4k(const QList<QPointF> &v14,
                     const QList<QPointF> &v24,
                     const double &xMin,
                     const double &xMax,
                     const double &yMin,
                     const double &yMax,
                     const double &temperature = 0.0,
                     bool record = true);
    void showCorrectionCurve(const QList<QPointF> &data,
                             const double &xMin,
                             const double &xMax,
                             const double &yMin,
                             const double &yMax,
                             const double &temperature);

private:
    void processCurve14(const QByteArray &data14,
                        QVector<double> &v_voltage14,
                        QVector<qint32> &raw14,
                        double &yMin,
                        double &yMax,
                        double &yMax14);
    void processCurve24(const QByteArray &data24,
                        QVector<double> &v_voltage24,
                        QVector<qint32> &raw24,
                        double &yMin,
                        double &yMax);

private:
    double m_time = 0.0;
    const double m_fs = 3600.0;
    const double m_T = 1.0 / m_fs;
    int m_offset14 = 0;
    int m_offset24 = 0;
    int m_algorithm = 0;
    int m_index_algorithm_neg_max95 = 0;
    QLineSeries *m_series;
    bool m_correction_enable = false;
    double m_correction_offset = 900;
    double m_correction_step = 1.5;
    double m_correction_num = 535;
    struct CORRECTION_SIN
    {
        double k1, b1, k2, b2, y0, A, xc, w;
    } m_correction_sin;
};

#endif // THREADWORKER_H
