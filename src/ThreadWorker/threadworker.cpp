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
                                  QVector<double> &raw14,
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
                                  QVector<double> &raw24,
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
    processF30Curve31(data31, v_voltage31, raw31, yMin, yMax, yMax31);
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
    emit dataReady4k(out33, out31, xMin, xMax, yMin, yMax);
    emit pointsReady4k(v_voltage33, v_voltage31, raw33, raw31);
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
    QVector<double> raw14;
    QVector<double> raw24;
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
        // 生成阈值表
        QVector<double> threshold;
        if (m_use_loaded_threshold) {
            threshold = m_threshold;
        } else {
            threshold = generateThreshold(temperature);
        }
        applyThreshold(threshold, raw14, raw24, temperature);
    }

    emit pointsReady4k(v_voltage14, v_voltage24, raw14, raw24);
    emit dataReady4k(out14, out24, xMin, xMax, yMin, yMax, temperature);
}

QVector<double> ThreadWorker::generateThreshold(const double &temperature)
{
    QVector<double> threshold;
    double T = temperature;
    if (m_formula == "sin") {
        if (qAbs(temperature) < 1e-9) {
            T = m_correction_sin.T;
        }
        for (int j = 0; j < m_correction_num; ++j) {
            double x = j * m_correction_step + m_correction_offset;
            double v1 = (m_correction_sin.k1 * T + m_correction_sin.b1) / 8.5 / 1000;
            double v2 = m_correction_sin.y0
                        + m_correction_sin.A
                              * std::sin(3.14159 * (x - m_correction_sin.xc) / m_correction_sin.w);
            double v3 = (m_correction_sin.k2 * T + m_correction_sin.b2) / 1000;
            double y = v1 * v2 + v3;
            threshold.push_back(qRound(y / 3.3 * (1 << 13)));
        }
    } else if (m_formula == "arcsin") {
        if (qAbs(temperature) < 1e-9) {
            T = m_correction_arcsin.T;
        }
        for (int j = 0; j < m_correction_num; ++j) {
            double x = j * m_correction_step + m_correction_offset;
            double y_lambda = 0.0;
            if (x <= 1300) {
                y_lambda = m_correction_arcsin.l_k
                               * (qAsin(x / 1000.0
                                        / (2 * m_correction_arcsin.l_d
                                           * qCos(M_PI / 180.0 * m_correction_arcsin.l_alpha / 2)))
                                  - m_correction_arcsin.l_alpha / 2)
                           + m_correction_arcsin.l_b;
            } else {
                y_lambda = m_correction_arcsin.r_k
                               * (qAsin(x / 1000.0
                                        / (2 * m_correction_arcsin.r_d
                                           * qCos(M_PI / 180.0 * m_correction_arcsin.r_alpha / 2)))
                                  - m_correction_arcsin.r_alpha / 2)
                           + m_correction_arcsin.r_b;
            }

            double y = (m_correction_arcsin.k1 * T + m_correction_arcsin.b1) / 8.5 / 1000 * y_lambda
                       + (m_correction_arcsin.k2 * T + m_correction_arcsin.b2) / 1000;
            threshold.push_back(qRound(y));
        }
    }
    return threshold;
}

void ThreadWorker::applyThreshold(const QVector<double> &threshold,
                                  const QVector<double> &raw14,
                                  const QVector<double> &raw24,
                                  const double &temperature)
{
    QList<QPointF> out_correction;
    int idx_max = 0;
    int raw_max = INT_MIN;
    for (int i = 0; i < raw14.size(); ++i) {
        if (raw_max < raw14[i]) {
            raw_max = raw14[i];
            idx_max = i;
        }
    }

    double x_max_correction = m_correction_offset;
    double y_min_correction = std::numeric_limits<double>::max();
    double y_max_correction = std::numeric_limits<double>::lowest();

    int start_idx = idx_max; // 从 raw14 最大值位置开始
    for (int idx_threshold = 0; idx_threshold < threshold.size(); ++idx_threshold) {
        int best_idx = -1;
        int best_diff = INT_MAX;

        // 只在剩余的 raw14 中寻找最近点
        for (int j = start_idx; j < raw14.size(); ++j) {
            int diff = std::abs(raw14[j] - threshold[idx_threshold]);
            if (diff < best_diff) {
                best_diff = diff;
                best_idx = j;
            }
            // 一旦 raw14[j] 小于 threshold[idx_threshold] 且差值开始变大，可以提前跳出
            if (raw14[j] < threshold[idx_threshold] && diff > best_diff) {
                break;
            }
        }

        if (best_idx >= 0 && best_idx < raw24.size()) {
            double x = m_correction_offset + idx_threshold * m_correction_step;
            double y = raw24[best_idx];
            out_correction.push_back(QPointF(x, y));

            x_max_correction = std::max(x_max_correction, x);
            y_min_correction = std::min(y_min_correction, y);
            y_max_correction = std::max(y_max_correction, y);

            // 下一次匹配从这里开始，保证顺序不回退
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

void ThreadWorker::onEnableCorrection(bool enable, const QJsonObject &params)
{
    m_correction_enable = enable;
    if (enable) {
        QString formula = params["formula"].toString();
        m_formula = formula;
        if (formula == "sin") {
            m_correction_sin.k1 = params["k1"].toDouble();
            m_correction_sin.b1 = params["b1"].toDouble();
            m_correction_sin.y0 = params["y0"].toDouble();
            m_correction_sin.A = params["A"].toDouble();
            m_correction_sin.xc = params["xc"].toDouble();
            m_correction_sin.w = params["w"].toDouble();
            m_correction_sin.k2 = params["k2"].toDouble();
            m_correction_sin.b2 = params["b2"].toDouble();
            m_correction_sin.T = params["T"].toDouble();
        } else if (formula == "arcsin") {
            m_correction_arcsin.k1 = params["k1"].toDouble();
            m_correction_arcsin.b1 = params["b1"].toDouble();
            m_correction_arcsin.k2 = params["k2"].toDouble();
            m_correction_arcsin.b2 = params["b2"].toDouble();
            m_correction_arcsin.l_k = params["l_k"].toDouble();
            m_correction_arcsin.l_b = params["l_b"].toDouble();
            m_correction_arcsin.l_d = params["l_d"].toDouble();
            m_correction_arcsin.l_alpha = params["l_alpha"].toDouble();
            m_correction_arcsin.r_k = params["r_k"].toDouble();
            m_correction_arcsin.r_b = params["r_b"].toDouble();
            m_correction_arcsin.r_d = params["r_d"].toDouble();
            m_correction_arcsin.r_alpha = params["r_alpha"].toDouble();
            m_correction_arcsin.T = params["T"].toDouble();
        }
        m_correction_offset = params["offset"].toDouble();
        m_correction_step = params["step"].toDouble();
    }
}

void ThreadWorker::onUseLoadedThreshold(bool isUse, QVector<double> threshold)
{
    m_use_loaded_threshold = isUse;
    m_threshold = threshold;
}

void ThreadWorker::processF30Curve31(const QByteArray &data31,
                                     QVector<double> &v_voltage31,
                                     QVector<double> &raw31,
                                     double &yMin,
                                     double &yMax,
                                     double &yMax31)
{
    // 长度字段（2字节，大端序）
    QByteArray payload = data31.mid(5, data31.size() - 7);
    if (payload.size() % 2 != 0) {
        LOG_WARN("Invalid data length: {}", payload.size());
        return;
    }

    QByteArray filteredPayload = payload;

    if (m_offset14 > 0) {
        payload = QByteArray(m_offset24, 0) + filteredPayload;
    } else if (m_offset14 < 0) {
        int skip = -m_offset14;
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

    yMax31 = yMax;
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

    if (m_offset24 > 0) {
        payload = QByteArray(m_offset24, 0) + filteredPayload;
    } else if (m_offset24 < 0) {
        int skip = -m_offset24;
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
