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

        // 生成阈值表
        QVector<qint32> threshold;
        for (int j = 0; j < m_correction_num; ++j) {
            double x = j * m_correction_step + m_correction_offset;
            double v1 = (m_correction_sin.k1 * m_correction_sin.T + m_correction_sin.b1) / 8.5
                        / 1000;
            double v2 = m_correction_sin.y0
                        + m_correction_sin.A
                              * std::sin(3.14159 * (x - m_correction_sin.xc) / m_correction_sin.w);
            double v3 = (m_correction_sin.k2 * m_correction_sin.T + m_correction_sin.b2) / 1000;
            double y = v1 * v2 + v3;
            threshold.push_back(qRound(y / 3.3 * (1 << 13)));
        }

        // int idx_max = 0;
        // int idx_min = 0;
        // int raw_max = INT_MIN;
        // int raw_min = INT_MAX;
        // for (int i = 0; i < raw14.size(); ++i) {
        //     if (raw_max < raw14[i]) {
        //         raw_max = raw14[i];
        //         idx_max = i;
        //     }
        //     if (raw_min > raw14[i]) {
        //         raw_min = raw14[i];
        //         idx_min = i;
        //     }
        // }

        // double x_max_correction = m_correction_offset;
        // double y_min_correction = std::numeric_limits<double>::max();
        // double y_max_correction = std::numeric_limits<double>::lowest();

        // int distance = INT_MAX;
        // int idx_threshold = 0;
        // for (int i = idx_max; i <= (idx_min - 1) && idx_threshold < 535; ++i) {
        //     if ((threshold[idx_threshold] < raw14[i])
        //         && (threshold[idx_threshold] >= raw14[i + 1])) {
        //         double x = m_correction_offset + idx_threshold * m_correction_step;
        //         if (i > raw24.size()) {
        //             break;
        //         }
        //         double y = raw24[i];
        //         out_correction.push_back(QPointF(x, y));
        //         x_max_correction = std::max(x_max_correction, x);
        //         y_min_correction = std::min(y, y_min_correction);
        //         y_max_correction = std::max(y, y_max_correction);
        //         idx_threshold++;
        //     }
        // }

        // emit showCorrectionCurve(out_correction,
        //                          m_correction_offset,
        //                          x_max_correction,
        //                          y_min_correction,
        //                          y_max_correction,
        //                          temperature);
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

    //     if (m_correction_enable) {
    //         QList<QPointF> out_correction;
    //         QVector<qint32> threshold;

    //         for (int j = 0; j < m_correction_num; ++j) {
    //             double x = j * m_correction_step + m_correction_offset;
    //             double v1 = (m_correction_sin.k1 * temperature + m_correction_sin.b1) / 8.5 / 1000;
    //             double v2 = m_correction_sin.y0
    //                         + m_correction_sin.A
    //                               * std::sin(3.14159 * (x - m_correction_sin.xc) / m_correction_sin.w);
    //             double v3 = (m_correction_sin.k2 * temperature + m_correction_sin.b2) / 1000;
    //             double y = v1 * v2 + v3;
    //             threshold.push_back(qRound(y / 3.3 * (1 << 13))); // 转换为与 raw14 相同的 14 位 ADC 值
    //         }

    //         double x_max_correction = m_correction_offset;
    //         double y_min_correction = std::numeric_limits<double>::max();
    //         double y_max_correction = std::numeric_limits<double>::lowest();

    //         const int SEND_POINT_NUM = 535; // 目标点数
    //         int select_count = 0;           // 已选点计数
    //         int raw14_index = 0;            // 当前 raw14 索引
    //         int judge_ok = 0;               // 是否找到起始点

    //         for (int i = 0; i < SEND_POINT_NUM && raw14_index < raw14.size(); ++i) {
    //             if (judge_ok == 0) { // 寻找第一个点
    //                 while (raw14_index < raw14.size()) {
    //                     qint32 bit14 = raw14[raw14_index];
    //                     qint32 consin_wave_val = threshold[i];

    //                     // 第一个点条件：bit14 < consin_wave_val
    //                     if (bit14 < consin_wave_val) {
    //                         if (raw14_index == 0) {
    //                             // 第一个点未找到，返回空点集
    //                             emit showCorrectionCurve({},
    //                                                      m_correction_offset,
    //                                                      m_correction_offset,
    //                                                      0,
    //                                                      0,
    //                                                      temperature);
    //                             goto remain;
    //                         }
    //                         double x = m_correction_offset + i * m_correction_step;
    //                         double y = raw24[raw14_index]; // 直接使用 raw24 的原始值
    //                         out_correction.push_back(QPointF(x, y));
    //                         x_max_correction = std::max(x_max_correction, x);
    //                         y_min_correction = std::min(y, y_min_correction);
    //                         y_max_correction = std::max(y, y_max_correction);
    //                         select_count++;
    //                         judge_ok = 1;
    //                         raw14_index++;
    //                         break;
    //                     } else {
    //                         raw14_index++;
    //                         if (raw14_index >= raw14.size()) {
    //                             // 未找到起始点
    //                             emit showCorrectionCurve({},
    //                                                      m_correction_offset,
    //                                                      m_correction_offset,
    //                                                      0,
    //                                                      0,
    //                                                      temperature);
    //                             goto remain;
    //                         }
    //                     }
    //                 }
    //             } else { // 寻找后续点
    //                 while (raw14_index < raw14.size()) {
    //                     qint32 bit14 = raw14[raw14_index];
    //                     qint32 consin_wave_val = threshold[i];

    //                     if (bit14 < consin_wave_val) {
    //                         double x = m_correction_offset + i * m_correction_step;
    //                         double y = raw24[raw14_index]; // 直接使用 raw24 的原始值
    //                         out_correction.push_back(QPointF(x, y));
    //                         x_max_correction = std::max(x_max_correction, x);
    //                         y_min_correction = std::min(y, y_min_correction);
    //                         y_max_correction = std::max(y, y_max_correction);
    //                         select_count++;
    //                         raw14_index++;
    //                         break;
    //                     } else {
    //                         raw14_index++;
    //                         if (raw14_index >= raw14.size()) {
    //                             // 未找到足够点
    //                             emit showCorrectionCurve(out_correction,
    //                                                      m_correction_offset,
    //                                                      x_max_correction,
    //                                                      y_min_correction,
    //                                                      y_max_correction,
    //                                                      temperature);
    //                             goto remain;
    //                         }
    //                     }
    //                 }
    //             }

    //             // 成功收集 535 个点
    //             if (select_count == SEND_POINT_NUM) {
    //                 emit showCorrectionCurve(out_correction,
    //                                          m_correction_offset,
    //                                          x_max_correction,
    //                                          y_min_correction,
    //                                          y_max_correction,
    //                                          temperature);
    //                 goto remain;
    //             }
    //         }

    //         // 未收集到 535 个点，返回部分结果
    //         emit showCorrectionCurve(out_correction,
    //                                  m_correction_offset,
    //                                  x_max_correction,
    //                                  y_min_correction,
    //                                  y_max_correction,
    //                                  temperature);
    //     }
    // remain:
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
        m_correction_sin.T = params["T"].toDouble();
    }
}
