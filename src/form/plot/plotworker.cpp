#include "plotworker.h"

#include <QLineSeries>
#include <QPointF>

PlotWorker::PlotWorker(QObject *parent)
    : QObject(parent)
{}

PlotWorker::~PlotWorker() {}

void PlotWorker::processData(const QByteArray &data, const QString &name)
{
    QByteArray payload = data.mid(5, data.size() - 7);
    qDebug() << "payload: " << payload.size();
    if (payload.size() % 3 != 0) {
        qWarning() << "Invalid data length";
        return;
    }
    QByteArray filteredPayload;
    for (int i = 0; i < payload.size(); i += 3) {
        quint32 value = (static_cast<quint8>(payload[i]) << 16)
                        | (static_cast<quint8>(payload[i + 1]) << 8)
                        | static_cast<quint8>(payload[i + 2]);

        if (value != 0) {
            filteredPayload.append(payload.mid(i, 3));
        }
    }
    payload = filteredPayload;

    int numPoints = payload.size() / 3;

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    QLineSeries *series = new QLineSeries;
    for (int i = 0; i < numPoints; ++i) {
        int idx = i * 3;
        quint32 raw = (quint8) payload[idx] << 16 | (quint8) payload[idx + 1] << 8
                      | (quint8) payload[idx + 2];

        qint32 signedRaw = static_cast<qint32>(raw);
        double voltage;

        if (name == "curve_24bit") {
            if (signedRaw & 0x800000) {
                signedRaw |= 0xFF000000;
            }
            voltage = signedRaw / double(1 << 23) * 2.5;
        } else if (name == "curve_14bit") {
            signedRaw &= 0x3FFF;
            if (signedRaw & 0x2000) {
                signedRaw |= 0xFFFFC000;
            }
            voltage = signedRaw / double(1 << 13) * 3.3;
        }

        if (voltage < yMin)
            yMin = voltage;
        if (voltage > yMax)
            yMax = voltage;

        // series->append(m_time + m_T * i, voltage);
        series->append(i, voltage);
    }
    emit pointsReady(series->points());

    // double xMin = m_time;
    // double xMax = m_time + numPoints * m_T;

    double xMin = 1;
    double xMax = 2000;

    // m_time = xMax;
    emit dataReady(name, series, numPoints, yMin, yMax, xMin, xMax);
}
