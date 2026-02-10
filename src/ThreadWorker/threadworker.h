#ifndef THREADWORKER_H
#define THREADWORKER_H

#include <QLineSeries>
#include <QObject>
#include "global.h"

class ThreadWorker : public QObject
{
    Q_OBJECT
public:
    explicit ThreadWorker(QObject *parent = nullptr);
    ~ThreadWorker();
    void setOffset31(const int &offset);
    void setOffset33(const int &offset);
    void setAlgorithm(const QString &algorithm);

public slots:
    void processDataF30(const QByteArray &data31,
                        const QByteArray &data33,
                        const double &temperature);
    void processDataF15(const QByteArray &data31,
                        const QByteArray &data33,
                        const double &temperature);
    void processDataLLC(const QByteArray &data31,
                        const QByteArray &data33,
                        const double &temperature);
    void onUseLoadedThreshold(bool isUse, QVector<double> threshold);
    void onUseLoadedThreadsholdOption(const QJsonObject &option);
    void onParamsArcSin(const PARAMS_ARCSIN &params);
    void onCollectionFittingPoints(const QString &dir, const QString &file, const int &count);

signals:
    void changeThresholdStatus(const QString &status);
    void dataReady4k(const QVector<double> &v31,
                     const QVector<double> &v33,
                     const QVector<double> &raw31,
                     const QVector<double> &raw33);
    void plotReady4k(const CURVE &curve31,
                     const CURVE &curve33,
                     const double &temperature = 0.0,
                     bool record = true);
    void showCorrectionCurve(const QList<QPointF> &data,
                             const double &xMin,
                             const double &xMax,
                             const double &yMin,
                             const double &yMax,
                             const double &temperature);
    void collectionFitingPointsFinish(bool status);

private:
    void processF30Curve31(const QByteArray &data31,
                           QVector<double> &v_voltage31,
                           QVector<double> &raw31,
                           double &yMin,
                           double &yMax);
    void processF30Curve33(const QByteArray &data33,
                           QVector<double> &v_voltage33,
                           QVector<double> &raw33,
                           double &yMin,
                           double &yMax);
    void processF15Curve31(const QByteArray &data31,
                           QVector<double> &v_voltage31,
                           QVector<double> &raw31,
                           double &yMin,
                           double &yMax);
    void processF15Curve33(const QByteArray &data33,
                           QVector<double> &v_voltage33,
                           QVector<double> &raw33,
                           double &yMin,
                           double &yMax);
    void applyThreshold(const QVector<double> &threshold,
                        const QVector<double> &raw31,
                        const QVector<double> &raw33,
                        const double &temperature);
    void calculateArcSinThreshold(const double &temperature);

private:
    int m_offset33 = 0;
    int m_offset31 = 0;
    QString m_algorithm = "";
    QLineSeries *m_series;
    double m_correction_offset = 900;
    double m_correction_step = 1;
    int m_correction_count = 800;
    QVector<double> m_threshold;
    bool m_autoupdate_threshold = false;
    bool m_enable_interpolation = false;
    PARAMS_ARCSIN m_params_arcsin;
    COLLECTION_FITTING_POINTS m_collection_fitting_points;
};

#endif // THREADWORKER_H
