#include "plotworker.h"

#include <QLineSeries>
#include <QPointF>

PlotWorker::PlotWorker(QObject *parent)
    : QObject(parent)
{}

PlotWorker::~PlotWorker() {}

void PlotWorker::setOffset14(const int &offset)
{
    m_offset14 = offset;
}

void PlotWorker::setOffset24(const int &offset)
{
    m_offset24 = offset;
}

void PlotWorker::setAlgorithm(int algorithm)
{
    m_algorithm = algorithm;
}

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
    if (name == "curve_14bit") {
        if (m_offset14 > 0) {
            payload = QByteArray(m_offset14, 0) + filteredPayload;
        } else if (m_offset14 < 0) {
            int skip = -m_offset14;
            payload = filteredPayload.mid(skip);
            payload.append(QByteArray(skip, 0));
        }
    } else if (name == "curve_24bit") {
        if (m_offset24 > 0) {
            payload = QByteArray(m_offset24, 0) + filteredPayload;
        } else if (m_offset24 < 0) {
            int skip = -m_offset24;
            payload = filteredPayload.mid(skip);
            payload.append(QByteArray(skip, 0));
        }
    }

    int numPoints = payload.size() / 3;

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    m_series = new QLineSeries(this);
    QVector<double> v_voltage;
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

        v_voltage.push_back(voltage);
    }

    // algorithm
    if (m_algorithm == 1) {
        if (name == "curve_14bit") {
            double val95 = yMax * 0.95;
            int index = 0;
            for (; index < numPoints; ++index) {
                if (v_voltage[index] > val95) {
                    m_index_algorithm_neg_max95 = index;
                    break;
                }
            }
        }
        int endIndex = std::min(m_index_algorithm_neg_max95 + 1000, numPoints);

        int indexNew = 0;
        for (int i = m_index_algorithm_neg_max95; i < endIndex; ++i) {
            m_series->append(indexNew++, v_voltage[i]);
        }
        numPoints = 1000;
    } else {
        for (int i = 0; i < numPoints; ++i) {
            m_series->append(i, v_voltage[i]);
        }
    }

    emit pointsReady(m_series);

    double xMin = 1;
    double xMax = numPoints;

    emit dataReady(name, m_series, numPoints, yMin, yMax, xMin, xMax);
}

void PlotWorker::processData4k(const QByteArray &data14, const QByteArray &data24)
{
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    double yMax14 = std::numeric_limits<double>::lowest();
    QVector<double> v_voltage14;
    QVector<double> v_voltage24;
    int numPoints = 0;
    {
        QByteArray payload = data14.mid(5, data14.size() - 7);
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
        if (m_offset14 > 0) {
            payload = QByteArray(m_offset14, 0) + filteredPayload;
        } else if (m_offset14 < 0) {
            int skip = -m_offset14;
            payload = filteredPayload.mid(skip);
            payload.append(QByteArray(skip, 0));
        }

        int numPoints = payload.size() / 3;

        for (int i = 0; i < numPoints; ++i) {
            int idx = i * 3;
            quint32 raw = (quint8) payload[idx] << 16 | (quint8) payload[idx + 1] << 8
                          | (quint8) payload[idx + 2];

            qint32 signedRaw = static_cast<qint32>(raw);
            double voltage;

            signedRaw &= 0x3FFF;
            if (signedRaw & 0x2000) {
                signedRaw |= 0xFFFFC000;
            }
            voltage = signedRaw / double(1 << 13) * 3.3;

            if (voltage < yMin) {
                yMin = voltage;
            }
            if (voltage > yMax) {
                yMax = voltage;
            }
            if (voltage > yMax14) {
                yMax14 = voltage;
            }

            v_voltage14.push_back(voltage);
        }
    }

    {
        QByteArray payload = data24.mid(5, data24.size() - 7);
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

        if (m_offset24 > 0) {
            payload = QByteArray(m_offset24, 0) + filteredPayload;
        } else if (m_offset24 < 0) {
            int skip = -m_offset24;
            payload = filteredPayload.mid(skip);
            payload.append(QByteArray(skip, 0));
        }

        int numPoints = payload.size() / 3;

        for (int i = 0; i < numPoints; ++i) {
            int idx = i * 3;
            quint32 raw = (quint8) payload[idx] << 16 | (quint8) payload[idx + 1] << 8
                          | (quint8) payload[idx + 2];

            qint32 signedRaw = static_cast<qint32>(raw);
            double voltage;

            if (signedRaw & 0x800000) {
                signedRaw |= 0xFF000000;
            }
            voltage = signedRaw / double(1 << 23) * 2.5;

            if (voltage < yMin)
                yMin = voltage;
            if (voltage > yMax)
                yMax = voltage;

            v_voltage24.push_back(voltage);
        }
    }
    double xMin = 0.0;
    double xMax = 1999.0;

    QList<QPointF> out14, out24;

    if (m_algorithm) {
        double val95 = yMax14 * 0.95;
        for (int index = 0; index < v_voltage14.size(); ++index) {
            if (v_voltage14[index] > val95) {
                m_index_algorithm_neg_max95 = index;
                break;
            }
        }
        int index = 0;
        numPoints = std::min(1000 + m_index_algorithm_neg_max95,
                             static_cast<int>(std::min(v_voltage14.size(), v_voltage24.size())));
        for (int i = m_index_algorithm_neg_max95; i < numPoints; ++i) {
            out14.push_back({static_cast<double>(index), v_voltage14[i]});
            out24.push_back({static_cast<double>(index), v_voltage24[i]});
            ++index;
        }
        xMax = std::min(out14.size(), out24.size());
    } else {
        numPoints = std::min(v_voltage14.size(), v_voltage24.size());
        for (int i = 0; i < numPoints; ++i) {
            out14.push_back({static_cast<double>(i), v_voltage14[i]});
            out24.push_back({static_cast<double>(i), v_voltage24[i]});
        }
        xMax = numPoints;
    }

    emit pointsReady4k(v_voltage14, v_voltage24);
    emit dataReady4k(out14, out24, xMin, xMax, yMin, yMax);
}
