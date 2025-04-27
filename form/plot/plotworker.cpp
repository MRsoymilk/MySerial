#include "plotworker.h"

#include <QLineSeries>
#include <QPointF>

PlotWorker::PlotWorker(QObject *parent)
    : QObject(parent)
{}

PlotWorker::~PlotWorker() {}

void PlotWorker::processData(const QByteArray &data)
{
    QByteArray payload = data.mid(5, data.size() - 7);
    qDebug() << "payload: " << payload.size();
    if (payload.size() % 3 != 0) {
        qWarning() << "Invalid data length";
        return;
    }

    int numPoints = payload.size() / 3;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    QLineSeries *series = new QLineSeries;
    for (int i = 0; i < numPoints; ++i) {
        int idx = i * 3;
        quint32 raw = (quint8) payload[idx] << 16 | (quint8) payload[idx + 1] << 8
                      | (quint8) payload[idx + 2];

        qint32 signedRaw = static_cast<qint32>(raw);
        if (signedRaw & 0x800000)
            signedRaw |= 0xFF000000;

        double voltage = (signedRaw / double(1 << 23)) * 2.5;

        if (voltage < minY)
            minY = voltage;
        if (voltage > maxY)
            maxY = voltage;

        series->append(m_time + m_T * i, voltage);
    }

    double min_x = m_time;
    double max_x = m_time + numPoints * m_T;
    m_time = max_x;
    emit dataReady(series, numPoints, minY, maxY, min_x, max_x);
}
