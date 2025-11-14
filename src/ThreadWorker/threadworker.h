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
    void processDataF30(const QByteArray &data31, const QByteArray &data33);
    void processData4k(const QByteArray &data14,
                       const QByteArray &data24,
                       const double &temperature);
    void onEnableCorrection(bool enable, const QJsonObject &params);
    void onUseLoadedThreshold(bool isUse, QVector<double> threshold);
    void onUseLoadedThreadsholdOption(const double &offset, const double &step);
signals:
    void pointsReady4k(const QVector<double> &v14,
                       const QVector<double> &v24,
                       const QVector<double> &raw14,
                       const QVector<double> &raw24);
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
    void processF30Curve31(const QByteArray &data31,
                           QVector<double> &v_voltage31,
                           QVector<double> &raw31,
                           double &yMin,
                           double &yMax,
                           double &yMax31);
    void processF30Curve33(const QByteArray &data33,
                           QVector<double> &v_voltage33,
                           QVector<double> &raw33,
                           double &yMin,
                           double &yMax);
    void processCurve14(const QByteArray &data14,
                        QVector<double> &v_voltage14,
                        QVector<double> &raw14,
                        double &yMin,
                        double &yMax,
                        double &yMax14);
    void processCurve24(const QByteArray &data24,
                        QVector<double> &v_voltage24,
                        QVector<double> &raw24,
                        double &yMin,
                        double &yMax);
    QVector<double> generateThreshold(const double &temperature);
    void applyThreshold(const QVector<double> &threshold,
                        const QVector<double> &raw14,
                        const QVector<double> &raw24,
                        const double &temperature);

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
    QString m_formula;
    struct CORRECTION_SIN
    {
        double k1, b1, k2, b2, y0, A, xc, w, T;
    } m_correction_sin;
    struct CORRECTION_ARCSIN
    {
        double k1, b1, k2, b2;
        double l_k, l_b, l_d, l_alpha;
        double r_k, r_b, r_d, r_alpha;
        double T;
    } m_correction_arcsin;
    QVector<double> m_threshold;
    bool m_use_loaded_threshold = false;
};

#endif // THREADWORKER_H
