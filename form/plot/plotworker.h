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

public slots:
    void processData(const QByteArray &data);

signals:
    void dataReady(
        QLineSeries *line, int numPoints, double minY, double maxY, double minX, double maxX);

private:
    double m_time = 0.0;
    const double m_fs = 3600.0;
    const double m_T = 1.0 / m_fs;
};

#endif // PLOTWORKER_H
