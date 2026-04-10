#include "handlemodeproduce.h"

#include <QSerialPortInfo>
#include <QThread>
#include <QTimer>

#include "funcdef.h"
#include "../../mode/FormProduce/formproduce.h"

HandleModeProduce::HandleModeProduce(QObject *parent) : QObject(parent) { init(); }

void HandleModeProduce::init() {
    m_timer_produce = new QTimer(this);
    m_timer_produce->setSingleShot(true);
    connect(m_timer_produce, &QTimer::timeout, this, &HandleModeProduce::onProduceModeTimeout);

    m_port_index = 0;
    m_wait_next_port = false;
    m_wait_next_cmd = false;
    m_establish = false;
}

void HandleModeProduce::setFrameType(QList<FrameType> type) { m_frameTypes = type; }

void HandleModeProduce::doOpt(int id, const QString &msg)
{
    m_call_step = id;
    switch (id) {
        case FormProduce::PRODUCE_WRITE_DEVICE_SERIAL:
            sendCMD(msg);
            m_wait_next_cmd = false;
            break;
        case FormProduce::PRODUCE_QUERY_DEVICE_SERIAL:
            m_wait_call = true;
            sendCMD("DD3C000312CDFF");
            m_wait_next_cmd = false;
            break;
        case FormProduce::PRODUCE_WRITE_BASEINE:
            sendCMD(msg);
            m_wait_next_cmd = false;
            break;
        case FormProduce::PRODUCE_QUERY_BASEINE:
            m_wait_call = true;
            sendCMD("DD3C000370CDFF");
            m_wait_next_cmd = false;
            break;
        case FormProduce::PRODUCE_WRITE_THRESHOLD:
            sendCMD(msg);
            m_wait_next_cmd = false;
            break;
        case FormProduce::PRODUCE_SELF_CHECK:
            m_wait_call = true;
            sendCMD("DD3C000350CDFF");
            m_wait_next_cmd = false;
            break;
    }
}

void HandleModeProduce::doConnect(const QStringList &ports) {
    m_ports = ports;
    m_port_index = 0;
    tryNextPort();
}

void HandleModeProduce::stopConnect() {
    sendCMD("DD3C000360CDFF");
    m_timer_produce->stop();
    if (m_serial) {
        m_serial->close();
        m_serial->deleteLater();
        m_serial = nullptr;
    }
    m_establish = false;
    m_wait_next_cmd = false;
    m_wait_next_port = false;
    m_produce_buffer.clear();
    LOG_INFO("Produce mode stop connect");
}

void HandleModeProduce::tryNextPort() {
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
        emit statusReport(PRODUCE_NONE, tr("[%1] open failed: %2").arg(portName).arg(m_serial->errorString()));

        ++m_port_index;
        m_timer_produce->stop();
        tryNextPort();
        return;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &HandleModeProduce::onProduceModeReadyRead, Qt::UniqueConnection);

    emit statusReport(PRODUCE_NONE, tr("[%1] start.").arg(m_ports[m_port_index]));

    m_step = PRODUCE_NONE;
    m_produce_buffer.clear();
    m_timer_produce->start(10 * 1000);
    m_timer_elapsed.start();
    m_wait_next_cmd = false;

    qDebug() << "Opened port:" << portName;

    sendCMD("DD3C000360CDFF");
    m_wait_next_cmd = false;

    sendCMD("DD3C000310CDFF");
    m_step = PRODUCE_HANDSHAKE;
    emit statusReport(PRODUCE_HANDSHAKE, tr("[%1] start handshake.").arg(m_ports[m_port_index]));
}

void HandleModeProduce::onProduceModeReadyRead() {
    if (!m_serial) return;

    QByteArray data = m_serial->readAll();
    if(m_wait_call) {
        processProduceCall(data);
    }
    if (m_establish) {
        emit dataReady(data);
    } else {
        processProduceConnect(data);
    }
}

void HandleModeProduce::processProduceCall(const QByteArray &data) {
    if(m_call_buffer.size() > 10 * 1024) {
        m_wait_call = false;
    }
    m_call_buffer.append(data);

    switch(m_call_step) {
        case FormProduce::PRODUCE_QUERY_DEVICE_SERIAL:
        {
            const QByteArray header = QByteArray::fromHex("DE3A000913");
            const QByteArray tail = QByteArray::fromHex("CEFF");
            int headPos = m_call_buffer.indexOf(header);
            if (headPos < 0) {
                LOG_WARN("DeviceSerial: not found header: DE3A000913");
                return;
            }
            int tailPos = m_call_buffer.indexOf(tail, headPos + header.size());
            if (tailPos < 0) {
                LOG_WARN("DeviceSerial: not found tail: CEFF");
                return;
            }
            int start = headPos + header.size();
            int len = tailPos - start;
            if (len <= 0) {
                LOG_WARN("DeviceSerial: invalid length");
                return;
            }
            QByteArray load = m_call_buffer.mid(start, len);
            emit optReturn(m_call_step, load.toHex(' ').toUpper());
            m_call_buffer.clear();
            m_wait_call = false;
        }
            break;
        case FormProduce::PRODUCE_QUERY_BASEINE:
        {
            const QByteArray header = QByteArray::fromHex("DE3A000671");
            const QByteArray tail = QByteArray::fromHex("CEFF");
            int headPos = m_call_buffer.indexOf(header);
            if (headPos < 0) {
                LOG_WARN("Baseline: not found header: DE3A000671");
                return;
            }

            int tailPos = m_call_buffer.indexOf(tail, headPos + header.size());
            if (tailPos < 0) {
                LOG_WARN("Baseline: not found tail: CEFF");
                return;
            }

            int start = headPos + header.size();
            int len = tailPos - start;

            if (len <= 0) {
                LOG_WARN("Baseline: invalid length");
                return;
            }

            QByteArray load  = m_call_buffer.mid(start, len);
            int val =
                (static_cast<quint8>(load[0]) << 16) | (static_cast<quint8>(load[1]) << 8) | static_cast<quint8>(load[2]);
            emit optReturn(m_call_step, QString::number(val));
            m_call_buffer.clear();
            m_wait_call = false;
        }
            break;
        case FormProduce::PRODUCE_SELF_CHECK:
        {
            QByteArray sign_check = QByteArray::fromHex("DE3A000351CEFF");
            int index = m_call_buffer.indexOf(sign_check);
            if (index != -1) {
                m_call_buffer.remove(index, sign_check.size());
                emit optReturn(m_call_step, tr("start self check"));
            }
            const QByteArray header = QByteArray::fromHex("DE3A000453");
            const QByteArray tail = QByteArray::fromHex("CEFF");
            int headPos = m_call_buffer.indexOf(header);
            if (headPos < 0) {
                LOG_WARN("SelfCheck: not found header: DE3A000453");
                return;
            }
            int tailPos = m_call_buffer.indexOf(tail, headPos + header.size());
            if (tailPos < 0) {
                LOG_WARN("SelfCheck: not found tail: CEFF");
                return;
            }
            int start = headPos + header.size();
            int len = tailPos - start;

            if (len <= 0) {
                LOG_WARN("SelfCheck: invalid length");
                return;
            }

            QByteArray data = m_call_buffer.mid(start, len);
            const qint8 code = static_cast<quint8>(data[0]);
            QString msg = "";
            if (code == 0) {
                msg = tr("No fault");
            } else if (code == 1) {
                msg = tr("Module not working");
            } else if (code == 2) {
                msg = tr("Temperature too high (above 50℃)");
            } else if (code == 3) {
                msg = tr("Temperature too low (below -20℃)");
            } else if (code == 4) {
                msg = tr("TEC not working");
            } else if (code == 5) {
                msg = tr("TEC unable to power on");
            } else if (code == 6) {
                msg = tr("Fan not working");
            } else if (code == 7) {
                msg = tr("DAC no output");
            } else if (code == 8) {
                msg = tr("Module unstable");
            }
            emit optReturn(m_call_step, msg);
            m_call_buffer.clear();
            m_wait_call = false;
        }
            break;
    }
}

void HandleModeProduce::processProduceConnect(const QByteArray &data) {
    m_produce_buffer.append(data);

    switch (m_step) {
        case PRODUCE_NONE:
            break;
        case PRODUCE_HANDSHAKE: {
            QByteArray expected_handshake = QByteArray::fromHex("DE3A000311CEFF");
            if (m_produce_buffer.contains(expected_handshake)) {
                m_step = PRODUCE_DATA_REQUEST;
                m_wait_next_cmd = false;
                m_produce_buffer.clear();
                sendCMD("DD3C000340CDFF");
            }
        } break;
        case PRODUCE_DATA_REQUEST: {
            emit statusReport(PRODUCE_DATA_REQUEST, tr("[%1] start data request.").arg(m_ports[m_port_index]));

            if (doProduceFrameExtra()) {
                m_wait_next_cmd = false;
                m_step = PRODUCE_FINISH;
            }
        } break;
        case PRODUCE_FINISH:
            emit statusReport(PRODUCE_FINISH, tr("[%1] finish.").arg(m_ports[m_port_index]));

            m_wait_next_cmd = false;
            m_timer_produce->stop();
            m_establish = true;
            emit connectEstablished();
            break;
        default:
            break;
    }
}

void HandleModeProduce::sendCMD(const QString &text) {
    if (!m_serial || !m_serial->isOpen() || m_wait_next_cmd) return;

    LOG_INFO("{} send cmd: {}", m_ports[m_port_index], text);
    m_serial->write(QByteArray::fromHex(text.toUtf8()));

    m_serial->waitForBytesWritten(200);
    m_serial->flush();
    QThread::msleep(500);

    m_wait_next_cmd = true;
}

bool HandleModeProduce::doProduceFrameExtra() {
    while (true) {
        if (m_frameTypes.isEmpty()) {
            LOG_WARN("Empty frame type!");
            m_produce_buffer.clear();
            return false;
        }
        int firstHeaderIdx = -1;
        FrameType current_frame;

        // 查找所有已知帧头
        for (const auto &type : m_frameTypes) {
            int idx = m_produce_buffer.indexOf(type.header);
            if (idx != -1 && (firstHeaderIdx == -1 || idx < firstHeaderIdx)) {
                firstHeaderIdx = idx;
                current_frame = type;
            }
        }

        // 没有帧头，清理或等待
        if (firstHeaderIdx == -1) {
            if (m_produce_buffer.size() > 50 * 1024) {
                LOG_WARN("Buffer overflow, clearing");
                m_produce_buffer.clear();
            }
            break;
        }

        // 丢弃无效数据
        if (firstHeaderIdx > 0) {
            LOG_WARN("Dropping invalid data before header: {} bytes", firstHeaderIdx);
            m_produce_buffer.remove(0, firstHeaderIdx);
        }

        if (current_frame.length != 0) {
            // 长度固定帧
            if (m_produce_buffer.size() < current_frame.length) break;

            QByteArray frame_candidate = m_produce_buffer.left(current_frame.length);
            if (!frame_candidate.endsWith(current_frame.footer)) {
                LOG_WARN("Invalid footer (fixed length), removing header only");
                m_produce_buffer.remove(0, current_frame.header.size());
                continue;
            }

            LOG_INFO("Fixed-length frame matched: {}", current_frame.name.toStdString());
            // handleFrame(current_frame.name, frame_candidate);
            m_produce_buffer.remove(0, current_frame.length);
            return true;
        } else {
            // 长度不固定：查找 footer 位置
            int footerIdx = m_produce_buffer.indexOf(current_frame.footer, current_frame.header.size());
            if (footerIdx == -1) {
                // 没找到帧尾，等待更多数据
                break;
            }

            int frame_len = footerIdx + current_frame.footer.size();
            QByteArray frame_candidate;
            frame_candidate = m_produce_buffer.left(frame_len);

            LOG_INFO("Variable-length frame matched: {}, size = {}", current_frame.name.toStdString(), frame_len);
            // handleFrame(current_frame.name, frame_candidate);

            m_produce_buffer.remove(0, frame_len);
            return true;
        }
    }
    return false;
}

void HandleModeProduce::onProduceModeTimeout() {
    qDebug() << "Port timeout, trying next port";
    ++m_port_index;
    tryNextPort();
}
