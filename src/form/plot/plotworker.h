#ifndef PLOTWORKER_H
#define PLOTWORKER_H

#include <QLineSeries>
#include <QObject>

class PlotWorker : public QObject
{
    Q_OBJECT
public:
    explicit PlotWorker(QObject *parent = nullptr);
    ~PlotWorker();
    void setOffset14(const int &offset);
    void setOffset24(const int &offset);
    void setAlgorithm(int);

public slots:
    void processData4k(const QByteArray &data14, const QByteArray &data24);

signals:
    void pointsReady4k(const QVector<double> &v14,
                       const QVector<double> &v24,
                       const QVector<quint32> &raw14,
                       const QVector<quint32> &raw24);
    void dataReady4k(const QList<QPointF> &v14,
                     const QList<QPointF> &v24,
                     const double &xMin,
                     const double &xMax,
                     const double &yMin,
                     const double &yMax);

private:
    void processCurve14(const QByteArray &data14,
                        QVector<double> &v_voltage14,
                        QVector<quint32> &raw14,
                        double &yMin,
                        double &yMax,
                        double &yMax14);
    void processCurve24(const QByteArray &data24,
                        QVector<double> &v_voltage24,
                        QVector<quint32> &raw24,
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
};

#endif // PLOTWORKER_H
