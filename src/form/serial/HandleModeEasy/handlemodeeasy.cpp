#include "handlemodeeasy.h"

#include <QThread>

#include "funcdef.h"
#include "../../mode/FormEasy/formeasy.h"

void HandleModeEasy::stopConnect() {
    sendCMD("DD3C000360CDFF");
    m_timer_easy->stop();
    if (m_serial) {
        m_serial->close();
        m_serial->deleteLater();
        m_serial = nullptr;
    }
    m_wait_next_cmd = false;
    m_wait_next_port = false;
    m_easy_buffer.clear();
}

HandleModeEasy::HandleModeEasy(QObject *parent) : QObject(parent) { init(); }

void HandleModeEasy::setFrameType(QList<FrameType> type) { m_frameTypes = type; }

void HandleModeEasy::init() {
    m_timer_easy = new QTimer(this);
    m_timer_easy->setSingleShot(true);
    connect(m_timer_easy, &QTimer::timeout, this, &HandleModeEasy::onEasyModeTimeout);

    m_port_index = 0;
    m_wait_next_port = false;
    m_wait_next_cmd = false;
    m_establish = false;
}

void HandleModeEasy::sendCMD(const QString &text) {
    if (!m_serial || !m_serial->isOpen() || m_wait_next_cmd) return;

    LOG_INFO("{} send cmd: {}", m_ports[m_port_index], text);
    m_serial->write(QByteArray::fromHex(text.toUtf8()));

    m_serial->waitForBytesWritten(200);
    m_serial->flush();
    QThread::msleep(500);

    m_wait_next_cmd = true;
}

bool HandleModeEasy::doThresholdExtra(const QByteArray &data) {
    const QByteArray header = QByteArray::fromHex("DE3A064569");
    const QByteArray tail = QByteArray::fromHex("CEFF");

    int headPos = data.indexOf(header);
    if (headPos < 0) {
        LOG_WARN("Threshold: not found header: DE3A064569");
        return false;
    }

    int tailPos = data.indexOf(tail, headPos + header.size());
    if (tailPos < 0) {
        LOG_WARN("Threshold: not found tail: CEFF");
        return false;
    }

    int payloadStart = headPos + header.size();
    int payloadLen = tailPos - payloadStart;

    QByteArray payload = data.mid(payloadStart, payloadLen);

    QList<double> values;
    for (int i = 0; i + 1 < payload.size(); i += 2) {
        qint16 v = (static_cast<quint8>(payload[i]) << 8) | static_cast<quint8>(payload[i + 1]);
        values.append(v);
    }
    emit sendThreshold(true, values);
    return true;
}

bool HandleModeEasy::doBaselineExtra(const QByteArray &data) {
    const QByteArray header = QByteArray::fromHex("DE3A000671");
    const QByteArray tail = QByteArray::fromHex("CEFF");
    int headPos = data.indexOf(header);
    if (headPos < 0) {
        LOG_WARN("Baseline: not found header: DE3A000671");
        return false;
    }

    int tailPos = data.indexOf(tail, headPos + header.size());
    if (tailPos < 0) {
        LOG_WARN("Baseline: not found tail: CEFF");
        return false;
    }

    int payloadStart = headPos + header.size();
    int payloadLen = tailPos - payloadStart;

    QByteArray payload = data.mid(payloadStart, payloadLen);
    int value = (static_cast<quint8>(payload[0]) << 16) | static_cast<quint8>(payload[1]) << 8 |
                static_cast<quint8>(payload[2]);
    emit sendOption({{"baseline", value}});
    return true;
}

void HandleModeEasy::processEasyConnect(const QByteArray &data) {
    m_easy_buffer.append(data);
    switch (m_step) {
        case EASY_HANDSHAKE: {
            QByteArray expected_handshake = QByteArray::fromHex("DE3A000311CEFF");
            if (m_easy_buffer.contains(expected_handshake)) {
                m_wait_next_cmd = false;
                m_easy_buffer.clear();

                if (m_mode == CFG_F30_MODE_SINGLE) {
                    m_step = EASY_SET_INTEGRATION_TIME;
                    sendCMD("DD3C000622000005CDFF");
                } else {
                    m_step = EASY_MODE_DOUBLE_DO_THRESHOLD;
                    sendCMD("DD3C000368CDFF");
                }
            }
        } break;
        case EASY_MODE_DOUBLE_DO_THRESHOLD: {
            emit statusReport(EASY_MODE_DOUBLE_DO_THRESHOLD,
                              tr("[%1] mode double, do threshold.").arg(m_ports[m_port_index]));

            if (doThresholdExtra(m_easy_buffer)) {
                m_easy_buffer.clear();
                m_step = EASY_MODE_DOUBLE_DO_BASELINE;
                m_wait_next_cmd = false;
                sendCMD("DD3C000370CDFF");
            }
        } break;
        case EASY_MODE_DOUBLE_DO_BASELINE: {
            emit statusReport(EASY_MODE_DOUBLE_DO_BASELINE,
                              tr("[%1] mode double, do baseline.").arg(m_ports[m_port_index]));

            if (doBaselineExtra(m_easy_buffer)) {
                m_easy_buffer.clear();
                m_step = EASY_DATA_REQUEST;
                m_wait_next_cmd = false;
                sendCMD("DD3C000340CDFF");
            }
        } break;
        case EASY_SET_INTEGRATION_TIME: {
            emit statusReport(EASY_SET_INTEGRATION_TIME, tr("[%1] set integration time.").arg(m_ports[m_port_index]));

            QByteArray except_integration = QByteArray::fromHex("DE3A000323CEFF");
            if (m_easy_buffer.contains(except_integration)) {
                m_easy_buffer.clear();
                m_wait_next_cmd = false;
                m_step = EASY_DATA_REQUEST;
                sendCMD("DD3C000330CDFF");
            }
        } break;
        case EASY_DATA_REQUEST: {
            emit statusReport(EASY_DATA_REQUEST, tr("[%1] start data request.").arg(m_ports[m_port_index]));

            if (doEasyFrameExtra()) {
                m_wait_next_cmd = false;
                m_step = EASY_FINISH;
            }
        } break;
        case EASY_FINISH: {
            emit statusReport(EASY_FINISH, tr("[%1] finish.").arg(m_ports[m_port_index]));

            m_establish = true;
            m_wait_next_cmd = false;
            m_timer_easy->stop();
            emit connectEstablished();
        } break;
        default:
            break;
    }
}

bool HandleModeEasy::doEasyFrameExtra() {
    while (true) {
        if (m_frameTypes.isEmpty()) {
            m_easy_buffer.clear();
            return false;
        }
        int firstHeaderIdx = -1;
        FrameType current_frame;

        // 查找所有已知帧头
        for (const auto &type : m_frameTypes) {
            int idx = m_easy_buffer.indexOf(type.header);
            if (idx != -1 && (firstHeaderIdx == -1 || idx < firstHeaderIdx)) {
                firstHeaderIdx = idx;
                current_frame = type;
            }
        }

        // 没有帧头，清理或等待
        if (firstHeaderIdx == -1) {
            if (m_easy_buffer.size() > 50 * 1024) {
                LOG_WARN("Buffer overflow, clearing");
                m_easy_buffer.clear();
            }
            break;
        }

        // 丢弃无效数据
        if (firstHeaderIdx > 0) {
            LOG_WARN("Dropping invalid data before header: {} bytes", firstHeaderIdx);
            m_easy_buffer.remove(0, firstHeaderIdx);
        }

        if (current_frame.length != 0) {
            // 长度固定帧
            if (m_easy_buffer.size() < current_frame.length) break;

            QByteArray frame_candidate = m_easy_buffer.left(current_frame.length);
            if (!frame_candidate.endsWith(current_frame.footer)) {
                LOG_WARN("Invalid footer (fixed length), removing header only");
                m_easy_buffer.remove(0, current_frame.header.size());
                continue;
            }

            LOG_INFO("Fixed-length frame matched: {}", current_frame.name.toStdString());
            m_easy_buffer.remove(0, current_frame.length);
            return true;
        } else {
            // 长度不固定：查找 footer 位置
            int footerIdx = m_easy_buffer.indexOf(current_frame.footer, current_frame.header.size());
            if (footerIdx == -1) {
                // 没找到帧尾，等待更多数据
                break;
            }

            int frame_len = footerIdx + current_frame.footer.size();
            QByteArray frame_candidate;
            frame_candidate = m_easy_buffer.left(frame_len);

            LOG_INFO("Variable-length frame matched: {}, size = {}", current_frame.name.toStdString(), frame_len);

            m_easy_buffer.remove(0, frame_len);
            return true;
        }
    }
    return false;
}

void HandleModeEasy::onEasyModeTimeout() {
    qDebug() << "Port timeout, trying next port";
    ++m_port_index;
    tryNextPort();
}

void HandleModeEasy::onEasyModeReadyRead() {
    if (!m_serial) return;

    QByteArray data = m_serial->readAll();
    if (m_establish) {
        emit dataReady(data);
    } else {
        processEasyConnect(data);
    }
}

void HandleModeEasy::doConnect(const QStringList &ports, const QString &mode) {
    stopConnect();
    m_mode = mode;
    m_ports = ports;
    m_port_index = 0;
    tryNextPort();
}

void HandleModeEasy::doOpt(int id, const QString &msg)
{
    m_call_step = id;
    switch (id) {
        case FormEasy::EASY_SET_INTEGRATION_TIME:
            sendCMD(msg);
            m_wait_next_cmd = false;
            break;
    }
}

void HandleModeEasy::tryNextPort() {
    if (m_port_index >= m_ports.size()) {
        qDebug() << "All ports tried, handshake failed";
        emit redoConnect();
        return;
    }

    if (m_serial) {
        m_serial->close();
        m_serial->deleteLater();
        m_serial = nullptr;
    }

    QString portName = m_ports.at(m_port_index);
    m_serial = new QSerialPort(this);
    m_serial->setPortName(portName);
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        LOG_WARN("{} open failed: {}", portName, m_serial->errorString());
        emit statusReport(EASY_NONE, tr("[%1] open failed: %2").arg(portName).arg(m_serial->errorString()));

        ++m_port_index;
        QTimer::singleShot(10, this, &HandleModeEasy::tryNextPort);
        return;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &HandleModeEasy::onEasyModeReadyRead, Qt::UniqueConnection);

    emit statusReport(EASY_NONE, tr("[%1] start.").arg(m_ports[m_port_index]));

    m_step = EASY_NONE;
    m_easy_buffer.clear();
    m_timer_easy->start(60 * 1000);
    m_timer_elapsed.start();
    m_wait_next_cmd = false;

    qDebug() << "Opened port:" << portName;
    sendCMD("DD3C000310CDFF");
    m_step = EASY_HANDSHAKE;
    emit statusReport(EASY_HANDSHAKE, tr("[%1] start handshake.").arg(m_ports[m_port_index]));
}
