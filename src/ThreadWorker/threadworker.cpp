#include "threadworker.h"
#include <QLineSeries>
#include <QPointF>
#include "funcdef.h"
#include "plot_algorithm.h"

ThreadWorker::ThreadWorker(QObject *parent)
    : QObject(parent)
{
    m_offset14 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET14, "0").toInt();
    m_offset24 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET24, "0").toInt();
    m_algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
}

ThreadWorker::~ThreadWorker() {}

void ThreadWorker::setOffset14(const int &offset)
{
    m_offset14 = offset;
}

void ThreadWorker::setOffset24(const int &offset)
{
    m_offset24 = offset;
}

void ThreadWorker::setAlgorithm(int algorithm)
{
    m_algorithm = algorithm;
}

void ThreadWorker::processCurve14(const QByteArray &data14,
                                QVector<double> &v_voltage14,
                                QVector<qint32> &raw14,
                                double &yMin,
                                double &yMax,
                                double &yMax14)
{
    {
        QByteArray payload = data14.mid(5, data14.size() - 7);
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
            raw14.push_back(signedRaw);
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
}
static int debug_count = 0;
void ThreadWorker::processCurve24(const QByteArray &data24,
                                QVector<double> &v_voltage24,
                                QVector<qint32> &raw24,
                                double &yMin,
                                double &yMax)
{
    {
        // QByteArray payload = data24.mid(5, data24.size() - 7);
        // if (payload.size() % 3 != 0) {
        //     LOG_WARN("Invalid data length: {}", payload.size());
        //     return;
        // }
        QByteArray payload = data24.mid(5, data24.size() - 7);
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
            raw24.push_back(signedRaw);
            voltage = signedRaw / double(1 << 23) * 2.5;

            if (voltage < yMin)
                yMin = voltage;
            if (voltage > yMax)
                yMax = voltage;

            v_voltage24.push_back(voltage);
        }
    }
}

void ThreadWorker::processData4k(const QByteArray &data14,
                                 const QByteArray &data24,
                                 const double &temperature)
{
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    double yMax14 = std::numeric_limits<double>::lowest();
    QVector<double> v_voltage14;
    QVector<double> v_voltage24;
    QVector<qint32> raw14;
    QVector<qint32> raw24;
    int numPoints = 0;

    if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::MAX_NEG_95)
        || m_algorithm == static_cast<int>(SHOW_ALGORITHM::NORMAL)) {
        processCurve14(data14, v_voltage14, raw14, yMin, yMax, yMax14);
        processCurve24(data24, v_voltage24, raw24, yMin, yMax);
    } else if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::NUM_660)) {
        processCurve24(data24, v_voltage24, raw24, yMin, yMax);
    }

    double xMin = 0.0;
    double xMax = 1999.0;

    QList<QPointF> out14, out24;

    if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::MAX_NEG_95)) {
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
    } else if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::NUM_660)) {
        xMin = 980;
        xMax = std::min(xMin + v_voltage24.size(), xMin + 660);
        out14.clear();
        v_voltage14.clear();
        for (int i = 0; i < v_voltage24.size(); ++i) {
            out24.push_back({static_cast<double>(i + xMin), v_voltage24[i]});
        }
    } else {
        numPoints = std::min(v_voltage14.size(), v_voltage24.size());
        for (int i = 0; i < numPoints; ++i) {
            out14.push_back({static_cast<double>(i), v_voltage14[i]});
            out24.push_back({static_cast<double>(i), v_voltage24[i]});
        }
        xMax = numPoints;
    }

    if (m_correction_enable) {
        QList<QPointF> out_correction;
        // generate threshold
        QVector<qint32> threshold;
        for (int j = 0; j < m_correction_num; ++j) {
            double x = j * m_correction_step + m_correction_offset;
            double v1 = (m_correction_sin.k1 * temperature + m_correction_sin.b1) / 8.5 / 1000;
            double v2 = m_correction_sin.y0
                        + m_correction_sin.A
                              * std::sin(3.14159 * (x - m_correction_sin.xc) / m_correction_sin.w);
            double v3 = (m_correction_sin.k2 * temperature + m_correction_sin.b2) / 1000;
            double y = v1 * v2 + v3;
            threshold.push_back(qRound(y / 3.3 * (1 << 13)));
        }

        double x_max_correction = m_correction_offset;
        double y_min_correction = std::numeric_limits<double>::max();
        double y_max_correction = std::numeric_limits<double>::lowest();

        // 遍历 raw14，找到最接近的阈值点
        for (int i = 0; i < raw14.size(); ++i) {
            int best_j = -1;
            int min_diff = std::numeric_limits<int>::max();
            for (int j = 0; j < threshold.size(); ++j) {
                int diff = std::abs(raw14[i] - threshold[j]);
                if (diff < min_diff) {
                    min_diff = diff;
                    best_j = j;
                }
            }

            if (best_j >= 0) {
                double x = m_correction_offset + best_j * m_correction_step;
                if (x > raw24.size()) {
                    break;
                }
                double y = raw24[i];
                out_correction.push_back(QPointF(x, y));

                x_max_correction = std::max(x_max_correction, x);
                y_min_correction = std::min(y, y_min_correction);
                y_max_correction = std::max(y, y_max_correction);
            }
        }

        emit showCorrectionCurve(out_correction,
                                 m_correction_offset,
                                 x_max_correction,
                                 y_min_correction,
                                 y_max_correction,
                                 temperature);
    }

    emit pointsReady4k(v_voltage14, v_voltage24, raw14, raw24);
    emit dataReady4k(out14, out24, xMin, xMax, yMin, yMax, temperature);
}

void ThreadWorker::onEnableCorrection(bool enable, const QJsonObject &params)
{
    m_correction_enable = enable;
    if (enable) {
        m_correction_offset = params["offset"].toDouble();
        m_correction_step = params["step"].toDouble();
        m_correction_sin.k1 = params["k1"].toDouble();
        m_correction_sin.b1 = params["b1"].toDouble();
        m_correction_sin.y0 = params["y0"].toDouble();
        m_correction_sin.A = params["A"].toDouble();
        m_correction_sin.xc = params["xc"].toDouble();
        m_correction_sin.w = params["w"].toDouble();
        m_correction_sin.k2 = params["k2"].toDouble();
        m_correction_sin.b2 = params["b2"].toDouble();
    }
}
