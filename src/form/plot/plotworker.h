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
    void processData(const QByteArray &data, const QString &name);

signals:
    void dataReady(const QString &name,
                   QLineSeries *line,
                   int numPoints,
                   double minY,
                   double maxY,
                   double minX,
                   double maxX);
    void pointsReady(QLineSeries *line);

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
