#include "threadworker.h"
#include <QDir>
#include <QLineSeries>
#include <QPointF>
#include "funcdef.h"

ThreadWorker::ThreadWorker(QObject *parent)
    : QObject(parent)
{
    m_offset31 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET31, "0").toInt();
    m_offset33 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET33, "0").toInt();
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

void ThreadWorker::setAlgorithm(const QString &algorithm)
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

void ThreadWorker::processDataF30(const QByteArray &data31,
                                  const QByteArray &data33,
                                  const double &temperature)
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
    CURVE curve31;
    CURVE curve33;
    for (int i = 0; i < raw31.size(); ++i) {
        curve31.raw.data.push_back({static_cast<double>(i), raw31.at(i)});
        curve31.raw.y_min = std::min(curve31.raw.y_min, raw31.at(i));
        curve31.raw.y_max = std::max(curve31.raw.y_max, raw31.at(i));
    }
    for (int i = 0; i < v_voltage31.size(); ++i) {
        curve31.data.push_back({static_cast<double>(i), v_voltage31.at(i)});
        curve31.y_min = std::min(curve31.y_min, v_voltage31.at(i));
        curve31.y_max = std::max(curve31.y_max, v_voltage31.at(i));
    }
    curve31.x_min = 0;
    curve31.x_max = v_voltage31.size();
    curve31.raw.x_min = 0;
    curve31.raw.x_max = raw31.size();
    for (int i = 0; i < raw33.size(); ++i) {
        curve33.raw.data.push_back({static_cast<double>(i), raw33.at(i)});
        curve33.raw.y_min = std::min(curve33.raw.y_min, raw33.at(i));
        curve33.raw.y_max = std::max(curve33.raw.y_max, raw33.at(i));
    }
    for (int i = 0; i < v_voltage33.size(); ++i) {
        curve33.data.push_back({static_cast<double>(i), v_voltage33.at(i)});
        curve33.y_min = std::min(curve33.y_min, v_voltage33.at(i));
        curve33.y_max = std::max(curve33.y_max, v_voltage33.at(i));
    }
    curve33.x_min = 0;
    curve33.x_max = v_voltage33.size();
    curve33.raw.x_min = 0;
    curve33.raw.x_max = raw33.size();

    if (!m_autoupdate_threshold) {
        applyThreshold(m_threshold, raw31, raw33, temperature);
    } else {
        calculateArcSinThreshold(temperature);
        applyThreshold(m_threshold, raw31, raw33, temperature);
    }

    emit plotReady4k(curve31, curve33, temperature);
    emit dataReady4k(v_voltage31, v_voltage33, raw31, raw33);
    if (m_collection_fitting_points.m_enable) {
        QString path = QString("%1/%2").arg(m_collection_fitting_points.dir,
                                            m_collection_fitting_points.file);
        QDir dir(m_collection_fitting_points.dir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QFile file(path);
        QIODevice::OpenMode mode;

        if (m_collection_fitting_points.current_idx == 0) {
            mode = QIODevice::WriteOnly | QIODevice::Truncate;
        } else {
            mode = QIODevice::WriteOnly | QIODevice::Append;
        }

        if (!file.open(mode)) {
            m_collection_fitting_points.m_enable = false;
            emit collectionFitingPointsFinish(false);
            return;
        }

        QByteArray hex31 = data31.toHex(' ').toUpper();
        QByteArray hex33 = data33.toHex(' ').toUpper();
        file.write(hex31);
        file.write("\n");
        file.write(hex33);
        file.write("\n");
        file.flush();
        file.close();

        ++m_collection_fitting_points.current_idx;
        if (m_collection_fitting_points.current_idx >= m_collection_fitting_points.count) {
            m_collection_fitting_points.m_enable = false;
            emit collectionFitingPointsFinish(true);
        }
    }
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
    int numPointsRaw = 0;

    if (m_algorithm == "F15_curves") {
        processF15Curve31(data31, v_voltage31, raw31, yMin, yMax);
        processF15Curve33(data33, v_voltage33, raw33, yMin, yMax);
    } else if (m_algorithm == "F15_single") {
        processF15Curve31(data31, v_voltage31, raw31, yMin, yMax);
    }

    CURVE curve31;
    CURVE curve33;
    if (m_algorithm == "F15_single") {
        for (int i = 0; i < v_voltage31.size(); ++i) {
            curve31.data.push_back({static_cast<double>(i), v_voltage31[i]});
            curve31.y_min = std::min(curve31.y_min, v_voltage31[i]);
            curve31.y_max = std::max(curve31.y_max, v_voltage31[i]);
        }
        curve31.x_min = 0;
        curve31.x_max = v_voltage31.size();
        for (int i = 0; i < raw31.size(); ++i) {
            curve31.raw.data.push_back({static_cast<double>(i), raw31[i]});
            curve31.raw.y_min = std::min(curve31.raw.y_min, raw31[i]);
            curve31.raw.y_max = std::max(curve31.raw.y_max, raw31[i]);
        }
        curve31.raw.x_min = 0;
        curve31.raw.x_max = raw31.size();
    } else {
        numPoints = std::min(v_voltage31.size(), v_voltage33.size());
        for (int i = 0; i < numPoints; ++i) {
            curve31.data.push_back({static_cast<double>(i), v_voltage31[i]});
            curve31.y_min = std::min(curve31.y_min, v_voltage31[i]);
            curve31.y_max = std::max(curve31.y_max, v_voltage31[i]);
            curve33.data.push_back({static_cast<double>(i), v_voltage33[i]});
            curve33.y_min = std::min(curve33.y_min, v_voltage33[i]);
            curve33.y_max = std::max(curve33.y_max, v_voltage33[i]);
        }
        curve31.x_min = 0;
        curve31.x_max = numPoints;
        curve33.x_min = 0;
        curve33.x_max = numPoints;

        numPointsRaw = std::min(raw31.size(), raw33.size());
        for (int i = 0; i < numPointsRaw; ++i) {
            curve31.raw.data.push_back({static_cast<double>(i), raw31[i]});
            curve31.raw.y_min = std::min(curve31.raw.y_min, raw31[i]);
            curve31.raw.y_max = std::max(curve31.raw.y_max, raw31[i]);
            curve33.raw.data.push_back({static_cast<double>(i), raw33[i]});
            curve33.raw.y_min = std::min(curve33.raw.y_min, raw33[i]);
            curve33.raw.y_max = std::max(curve33.raw.y_max, raw33[i]);
        }
        curve31.raw.x_min = 0;
        curve31.raw.x_max = numPointsRaw;
        curve33.raw.x_min = 0;
        curve33.raw.x_max = numPointsRaw;
    }

    if (!m_autoupdate_threshold) {
        applyThreshold(m_threshold, raw31, raw33, temperature);
    }

    emit plotReady4k(curve31, curve33, temperature);
    emit dataReady4k(v_voltage31, v_voltage33, raw31, raw33);
}

void ThreadWorker::processDataLLC(const QByteArray &data31,
                                  const QByteArray &data33,
                                  const double &temperature)
{
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    double yMax31 = std::numeric_limits<double>::lowest();
    QVector<double> v_voltage31;
    QVector<double> v_voltage33;
    QVector<double> raw31;
    QVector<double> raw33;
    int numPoints = 0;
    // 31
    {
        // 长度字段（2字节，大端序）
        QByteArray payload = data31.mid(5, data31.size() - 7);
        if (payload.size() % 4 != 0) {
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
        for (int i = 0; i + 4 < payload.size(); i += 4) {
            // big-endian 高字节在前
            quint16 raw = (static_cast<quint8>(payload[i + 1]))
                          | (static_cast<quint8>(payload[i + 2]) << 8);

            double voltage = static_cast<double>(raw) * 50.358 / 1000000.0;
            // double voltage = raw;

            if (voltage < yMin)
                yMin = voltage;
            if (voltage > yMax)
                yMax = voltage;

            v_voltage31.push_back(voltage);
            raw31.push_back(raw);
        }
    }
    // 33
    {
        QByteArray payload = data33.mid(5, data33.size() - 7);
        if (payload.size() % 4 != 0) {
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

        for (int i = 0; i + 4 < payload.size(); i += 4) {
            quint16 raw = (static_cast<quint8>(payload[i + 1]))
                          | (static_cast<quint8>(payload[i + 2]) << 8);

            qint16 signedRaw = *reinterpret_cast<qint16 *>(&raw);
            double voltage = static_cast<double>(signedRaw) / 0x8000 * 3.3;
            // double voltage = signedRaw;

            if (voltage < yMin)
                yMin = voltage;
            if (voltage > yMax)
                yMax = voltage;

            v_voltage33.push_back(voltage);
            raw33.push_back(signedRaw);
        }
    }
    QList<QPointF> out31, out33;
    CURVE curve31;
    CURVE curve33;
    for (int i = 0; i < v_voltage31.size(); ++i) {
        curve31.data.push_back({static_cast<double>(i), v_voltage31.at(i)});
        curve31.y_min = std::min(curve31.y_min, v_voltage31.at(i));
        curve31.y_max = std::max(curve31.y_max, v_voltage31.at(i));
    }
    curve31.x_min = 0;
    curve31.x_max = v_voltage31.size();
    for (int i = 0; i < v_voltage33.size(); ++i) {
        curve33.data.push_back({static_cast<double>(i), v_voltage33.at(i)});
        curve33.y_min = std::min(curve33.y_min, v_voltage33.at(i));
        curve33.y_max = std::max(curve33.y_max, v_voltage33.at(i));
    }
    curve33.x_min = 0;
    curve33.x_max = v_voltage33.size();

    if (!m_autoupdate_threshold) {
        applyThreshold(m_threshold, raw31, raw33, temperature);
    } else {
        calculateArcSinThreshold(temperature);
        applyThreshold(m_threshold, raw31, raw33, temperature);
    }

    emit plotReady4k(curve31, curve33, temperature);
    emit dataReady4k(v_voltage31, v_voltage33, raw31, raw33);
}

void ThreadWorker::applyThreshold(const QVector<double> &threshold,
                                  const QVector<double> &raw31,
                                  const QVector<double> &raw33,
                                  const double &temperature)
{
    QList<QPointF> out_correction;
    int idx_max = 0;
    int idx_min = 0;
    int raw_max = INT_MIN;
    int raw_min = INT_MAX;
    for (int i = 0; i < raw33.size(); ++i) {
        // if (raw_max < raw33[i]) {
        //     raw_max = raw33[i];
        //     idx_max = i;
        // }
        if (raw_min > raw33[i]) {
            raw_min = raw33[i];
            idx_min = i;
        }
    }
    if (idx_min < idx_max) {
        std::swap(idx_min, idx_max);
    }

    double x_max_correction = m_correction_offset;
    double y_min_correction = std::numeric_limits<double>::max();
    double y_max_correction = std::numeric_limits<double>::lowest();
    int step = 1000;
    for (int idx_threshold = 0; idx_threshold < threshold.size(); ++idx_threshold) {
        double min_distance = std::numeric_limits<double>::max();
        int target_idx = 0;
        double x = m_correction_offset + idx_threshold * m_correction_step;
        double current_threshold = threshold[idx_threshold];
        int end_idx = std::min(idx_min, idx_max + step);
        for (int j = idx_max; j < end_idx; ++j) {
            double _distance = std::abs(current_threshold - raw33[j]);
            if (min_distance > _distance) {
                min_distance = _distance;
                target_idx = j;
                idx_max = j;
            }
        }
        out_correction.push_back(QPointF(x, raw31[target_idx]));
        x_max_correction = std::max(x_max_correction, x);
        y_min_correction = std::min(y_min_correction, raw31[target_idx]);
        y_max_correction = std::max(y_max_correction, raw31[target_idx]);
    }
    if (m_enable_interpolation) {
        int n = out_correction.size();
        if (n < 2)
            return;

        QList<QPointF> dense;
        for (int i = 0; i < n - 1; ++i) {
            const QPointF &p0 = out_correction[i];
            const QPointF &p1 = out_correction[i + 1];
            dense.append(p0);
            double midX = (p0.x() + p1.x()) * 0.5;
            double midY = (p0.y() + p1.y()) * 0.5;
            dense.append(QPointF(midX, midY));
        }
        dense.append(out_correction.last());

        QList<QPointF> out_interpolation;
        int m = dense.size();
        for (int i = 0; i < m;) {
            int start = i;
            double y0 = dense[i].y();

            // 找平台
            while (i + 1 < m && qFuzzyCompare(dense[i + 1].y(), y0)) {
                i++;
            }

            int end = i;

            // 非平台
            if (start == end) {
                out_interpolation.append(dense[start]);
                i++;
                continue;
            }

            // 平台段
            double yStart = dense[start].y();
            double yEnd = (end + 1 < m) ? dense[end + 1].y() : yStart;

            int count = end - start + 1;

            for (int k = 0; k < count; ++k) {
                double t = double(k + 1) / (count + 1);
                double y = yStart + t * (yEnd - yStart);
                out_interpolation.append(QPointF(dense[start + k].x(), y));
            }

            i = end + 1;
        }
        emit showCorrectionCurve(out_interpolation,
                                 m_correction_offset,
                                 x_max_correction,
                                 y_min_correction,
                                 y_max_correction,
                                 temperature);
    } else {
        emit showCorrectionCurve(out_correction,
                                 m_correction_offset,
                                 x_max_correction,
                                 y_min_correction,
                                 y_max_correction,
                                 temperature);
    }
}

void ThreadWorker::calculateArcSinThreshold(const double &temperature)
{
    QVector<double> threshod;
    for (int i = 0; i < m_correction_count; ++i) {
        double idx = i * m_correction_step + m_correction_offset;
        double y_lambda = 0;
        if (idx <= 1300) {
            y_lambda = m_params_arcsin.l_k
                           * (qAsin(idx / 1000.0
                                    / (2 * m_params_arcsin.l_d
                                       * qCos(M_PI / 180.0 * m_params_arcsin.l_alpha / 2)))
                              - m_params_arcsin.l_alpha / 2)
                       + m_params_arcsin.l_b;
        } else {
            y_lambda = m_params_arcsin.r_k
                           * (qAsin(idx / 1000.0
                                    / (2 * m_params_arcsin.r_d
                                       * qCos(M_PI / 180.0 * m_params_arcsin.r_alpha / 2)))
                              - m_params_arcsin.r_alpha / 2)
                       + m_params_arcsin.r_b;
        }
        double y = (m_params_arcsin.t_k1 * temperature + m_params_arcsin.t_b1) / 8.5 / 1000
                       * y_lambda
                   + (m_params_arcsin.t_k2 * temperature + m_params_arcsin.t_b2) / 1000;
        threshod.push_back(y);
    }

    m_threshold = threshod;
}

void ThreadWorker::onUseLoadedThreshold(bool isUse, QVector<double> threshold)
{
    m_autoupdate_threshold = !isUse;
    m_threshold = threshold;
    emit changeThresholdStatus("use loaded threshold");
}

void ThreadWorker::onUseLoadedThreadsholdOption(const QJsonObject &option)
{
    if (option.contains("offset")) {
        m_correction_offset = option["offset"].toDouble();
    }
    if (option.contains("step")) {
        m_correction_step = option["step"].toDouble();
    }
    if (option.contains("count")) {
        m_correction_count = option["count"].toDouble();
    }
    if (option.contains("interpolation")) {
        m_enable_interpolation = option["interpolation"].toBool();
    }
}

void ThreadWorker::onParamsArcSin(const PARAMS_ARCSIN &params)
{
    m_params_arcsin = params;
    m_autoupdate_threshold = true;
    emit changeThresholdStatus("use auto update threshold");
}

void ThreadWorker::onCollectionFittingPoints(const QString &dir,
                                             const QString &file,
                                             const int &count)
{
    m_collection_fitting_points.count = count;
    m_collection_fitting_points.current_idx = 0;
    m_collection_fitting_points.dir = dir;
    m_collection_fitting_points.file = file;
    m_collection_fitting_points.m_enable = true;
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

        double voltage = static_cast<double>(raw) * 50.358 / 1000000.0;

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
        double voltage = static_cast<double>(signedRaw) / 0x8000 * 3.3;

        if (voltage < yMin)
            yMin = voltage;
        if (voltage > yMax)
            yMax = voltage;

        v_voltage33.push_back(voltage);
        raw33.push_back(signedRaw);
    }
}
