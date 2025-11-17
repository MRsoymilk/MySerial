#include "threadworker.h"
#include <QLineSeries>
#include <QPointF>
#include "funcdef.h"
#include "plot_algorithm.h"

ThreadWorker::ThreadWorker(QObject *parent)
    : QObject(parent)
{
    m_offset31 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET31, "0").toInt();
    m_offset33 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET33, "0").toInt();
    m_algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
}

ThreadWorker::~ThreadWorker() {}

void ThreadWorker::setOffset31(const int &offset)
{
    m_offset31 = offset;
}

void ThreadWorker::setOffset33(const int &offset)
{
    m_offset33 = offset;
}

void ThreadWorker::setAlgorithm(int algorithm)
{
    m_algorithm = algorithm;
}

void ThreadWorker::processF15Curve31(const QByteArray &data31,
                                     QVector<double> &v_voltage31,
                                     QVector<double> &raw31,
                                     double &yMin,
                                     double &yMax)
{
    QByteArray payload = data31.mid(5, data31.size() - 7);
    int remainder = payload.size() % 3;

    if (remainder != 0) {
        int padding = 3 - remainder;
        LOG_WARN("Invalid data length: {}, padding {} zero byte(s)", payload.size(), padding);
        payload.append(QByteArray(padding, '\0'));
    }
    QByteArray filteredPayload;
    for (int i = 0; i < payload.size(); i += 3) {
        quint32 value = (static_cast<quint8>(payload[i]) << 16)
                        | (static_cast<quint8>(payload[i + 1]) << 8)
                        | static_cast<quint8>(payload[i + 2]);
        // if (value != 0) {
        //     filteredPayload.append(payload.mid(i, 3));
        // }
        filteredPayload.append(payload.mid(i, 3));
    }

    payload = filteredPayload;

    if (m_offset31 > 0) {
        payload = QByteArray(m_offset31, 0) + filteredPayload;
    } else if (m_offset31 < 0) {
        int skip = -m_offset31;
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
        raw31.push_back(signedRaw);
        voltage = signedRaw / double(1 << 23) * 2.5;

        if (voltage < yMin)
            yMin = voltage;
        if (voltage > yMax)
            yMax = voltage;

        v_voltage31.push_back(voltage);
    }
}

void ThreadWorker::processF15Curve33(const QByteArray &data33,
                                     QVector<double> &v_voltage33,
                                     QVector<double> &raw33,
                                     double &yMin,
                                     double &yMax)
{
    {
        QByteArray payload = data33.mid(5, data33.size() - 7);
        if (payload.size() % 3 != 0) {
            LOG_WARN("Invalid data length: {}", payload.size());
            return;
        }
        QByteArray filteredPayload;
        for (int i = 0; i < payload.size(); i += 3) {
            quint32 value = (static_cast<quint8>(payload[i]) << 16)
                            | (static_cast<quint8>(payload[i + 1]) << 8)
                            | static_cast<quint8>(payload[i + 2]);
            // if (value != 0) {
            //     filteredPayload.append(payload.mid(i, 3));
            // }
            filteredPayload.append(payload.mid(i, 3));
        }

        payload = filteredPayload;
        if (m_offset33 > 0) {
            payload = QByteArray(m_offset33, 0) + filteredPayload;
        } else if (m_offset33 < 0) {
            int skip = -m_offset33;
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
            raw33.push_back(signedRaw);
            voltage = signedRaw / double(1 << 13) * 3.3;

            if (voltage < yMin) {
                yMin = voltage;
            }
            if (voltage > yMax) {
                yMax = voltage;
            }

            v_voltage33.push_back(voltage);
        }
    }
}

void ThreadWorker::processDataF30(const QByteArray &data31, const QByteArray &data33)
{
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    double yMax31 = std::numeric_limits<double>::lowest();
    QVector<double> v_voltage31;
    QVector<double> v_voltage33;
    QVector<double> raw31;
    QVector<double> raw33;
    int numPoints = 0;
    processF30Curve31(data31, v_voltage31, raw31, yMin, yMax);
    processF30Curve33(data33, v_voltage33, raw33, yMin, yMax);
    QList<QPointF> out31, out33;
    for (int i = 0; i < v_voltage31.size(); ++i) {
        out31.push_back({static_cast<double>(i), v_voltage31.at(i)});
    }
    for (int i = 0; i < v_voltage33.size(); ++i) {
        out33.push_back({static_cast<double>(i), v_voltage33.at(i)});
    }
    double xMin, xMax;
    xMin = 0;
    xMax = std::max(v_voltage31.size(), v_voltage33.size());
    if (m_use_loaded_threshold) {
        applyThreshold(m_threshold, raw31, raw33, 0);
    }
    emit plotReady4k(out31, out33, xMin, xMax, yMin, yMax);
    emit dataReady4k(v_voltage31, v_voltage33, raw31, raw33);
}

void ThreadWorker::processDataF15(const QByteArray &data31,
                                  const QByteArray &data33,
                                  const double &temperature)
{
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    QVector<double> v_voltage31;
    QVector<double> v_voltage33;
    QVector<double> raw31;
    QVector<double> raw33;
    int numPoints = 0;

    if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::F15_CURVES)) {
        processF15Curve31(data31, v_voltage31, raw31, yMin, yMax);
        processF15Curve33(data33, v_voltage33, raw33, yMin, yMax);
    } else if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::F15_SINGLE)) {
        processF15Curve33(data33, v_voltage33, raw33, yMin, yMax);
    }

    double xMin = 0.0;
    double xMax = 1999.0;

    QList<QPointF> out14, out24;

    if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::F15_SINGLE)) {
        xMin = 980;
        xMax = std::min(xMin + v_voltage33.size(), xMin + 660);
        out14.clear();
        v_voltage31.clear();
        for (int i = 0; i < v_voltage33.size(); ++i) {
            out24.push_back({static_cast<double>(i + xMin), v_voltage33[i]});
        }
    } else {
        numPoints = std::min(v_voltage31.size(), v_voltage33.size());
        for (int i = 0; i < numPoints; ++i) {
            out14.push_back({static_cast<double>(i), v_voltage31[i]});
            out24.push_back({static_cast<double>(i), v_voltage33[i]});
        }
        xMax = numPoints;
    }

    if (m_use_loaded_threshold) {
        applyThreshold(m_threshold, raw31, raw33, temperature);
    }

    emit dataReady4k(v_voltage31, v_voltage33, raw31, raw33);
    emit plotReady4k(out14, out24, xMin, xMax, yMin, yMax, temperature);
}

void ThreadWorker::applyThreshold(const QVector<double> &threshold,
                                  const QVector<double> &raw31,
                                  const QVector<double> &raw33,
                                  const double &temperature)
{
    QList<QPointF> out_correction;
    int idx_max = 0;
    int raw_max = INT_MIN;
    for (int i = 0; i < raw33.size(); ++i) {
        if (raw_max < raw33[i]) {
            raw_max = raw33[i];
            idx_max = i;
        }
    }

    double x_max_correction = m_correction_offset;
    double y_min_correction = std::numeric_limits<double>::max();
    double y_max_correction = std::numeric_limits<double>::lowest();

    int start_idx = idx_max;
    for (int idx_threshold = 0; idx_threshold < threshold.size(); ++idx_threshold) {
        int best_idx = -1;
        int best_diff = INT_MAX;

        for (int j = start_idx; j < raw33.size(); ++j) {
            int diff = std::abs(raw33[j] - threshold[idx_threshold]);
            if (diff < best_diff) {
                best_diff = diff;
                best_idx = j;
            }
            if (raw33[j] < threshold[idx_threshold] && diff > best_diff) {
                break;
            }
        }

        if (best_idx >= 0 && best_idx < raw31.size()) {
            double x = m_correction_offset + idx_threshold * m_correction_step;
            double y = raw31[best_idx];
            out_correction.push_back(QPointF(x, y));

            x_max_correction = std::max(x_max_correction, x);
            y_min_correction = std::min(y_min_correction, y);
            y_max_correction = std::max(y_max_correction, y);

            start_idx = best_idx;
        }
    }

    emit showCorrectionCurve(out_correction,
                             m_correction_offset,
                             x_max_correction,
                             y_min_correction,
                             y_max_correction,
                             temperature);
}

void ThreadWorker::onUseLoadedThreshold(bool isUse, QVector<double> threshold)
{
    m_use_loaded_threshold = isUse;
    m_threshold = threshold;
}

void ThreadWorker::onUseLoadedThreadsholdOption(const double &offset, const double &step)
{
    m_correction_offset = offset;
    m_correction_step = step;
}

void ThreadWorker::processF30Curve31(const QByteArray &data31,
                                     QVector<double> &v_voltage31,
                                     QVector<double> &raw31,
                                     double &yMin,
                                     double &yMax)
{
    // 长度字段（2字节，大端序）
    QByteArray payload = data31.mid(5, data31.size() - 7);
    if (payload.size() % 2 != 0) {
        LOG_WARN("Invalid data length: {}", payload.size());
        return;
    }

    QByteArray filteredPayload = payload;

    if (m_offset31 > 0) {
        payload = QByteArray(m_offset31, 0) + filteredPayload;
    } else if (m_offset31 < 0) {
        int skip = -m_offset31;
        payload = filteredPayload.mid(skip);
        payload.append(QByteArray(skip, 0));
    }
    // 遍历所有采样点
    for (int i = 0; i + 2 < payload.size(); i += 2) {
        // big-endian 高字节在前
        quint16 raw = (static_cast<quint8>(payload[i]) << 8)
                      | (static_cast<quint8>(payload[i + 1]));

        double voltage = static_cast<double>(raw) * 38.15 / 1000000.0;

        if (voltage < yMin)
            yMin = voltage;
        if (voltage > yMax)
            yMax = voltage;

        v_voltage31.push_back(voltage);
        raw31.push_back(raw);
    }
}

void ThreadWorker::processF30Curve33(const QByteArray &data33,
                                     QVector<double> &v_voltage33,
                                     QVector<double> &raw33,
                                     double &yMin,
                                     double &yMax)
{
    QByteArray payload = data33.mid(5, data33.size() - 7);
    if (payload.size() % 2 != 0) {
        LOG_WARN("Invalid data length: {}", payload.size());
        return;
    }

    QByteArray filteredPayload = payload;

    if (m_offset33 > 0) {
        payload = QByteArray(m_offset33, 0) + filteredPayload;
    } else if (m_offset33 < 0) {
        int skip = -m_offset33;
        payload = filteredPayload.mid(skip);
        payload.append(QByteArray(skip, 0));
    }

    for (int i = 0; i + 2 < payload.size(); i += 2) {
        quint16 raw = (static_cast<quint8>(payload[i]) << 8)
                      | (static_cast<quint8>(payload[i + 1]));

        qint16 signedRaw = *reinterpret_cast<qint16 *>(&raw);
        double voltage = static_cast<double>(signedRaw) / 0x8000 * 2.5;

        if (voltage < yMin)
            yMin = voltage;
        if (voltage > yMax)
            yMax = voltage;

        v_voltage33.push_back(voltage);
        raw33.push_back(signedRaw);
    }
}
