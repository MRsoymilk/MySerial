#include "handlemodeproduce.h"
#include "funcdef.h"
#include <QThread>
#include <QTimer>
#include <QSerialPortInfo>

HandleModeProduce::HandleModeProduce(QObject *parent) : QObject(parent) {
    init();
}

void HandleModeProduce::init() {
    m_timer_produce = new QTimer(this);
    m_timer_produce->setSingleShot(true);
    connect(m_timer_produce, &QTimer::timeout, this, &HandleModeProduce::onProduceModeTimeout);

    m_port_index = 0;
    m_wait_next_port = false;
    m_wait_next_cmd = false;
    m_establish = false;
}

void HandleModeProduce::setFrameType(QList<FrameType> type)
{
    m_frameTypes = type;
}

void HandleModeProduce::doConnect(const QStringList &ports)
{
    m_ports = ports;
    m_port_index = 0;
    tryNextPort();
}

void HandleModeProduce::stopConnect()
{
    sendCMD("DD3C000360CDFF");
    m_serial->waitForBytesWritten(200);
    m_serial->flush();
    m_timer_produce->stop();
    if(m_serial) {
        m_serial->close();
        m_serial->deleteLater();
        m_serial = nullptr;
    }
    m_wait_next_cmd = false;
    m_wait_next_port = false;
    m_produce_buffer.clear();
}

void HandleModeProduce::tryNextPort()
{
    if (m_port_index >= m_ports.size()) {
        qDebug() << "All ports tried, handshake failed";
        emit redoConnect();
        return;
    }

    if(m_serial) {
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
        LOG_WARN("open failed: {}", m_serial->errorString());
        ++m_port_index;
        QTimer::singleShot(10, this, &HandleModeProduce::tryNextPort);
        return;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &HandleModeProduce::onProduceModeReadyRead, Qt::UniqueConnection);

    m_step = PRODUCE_HANDSHAKE;
    m_produce_buffer.clear();
    m_timer_produce->start(60 * 1000);
    m_timer_elapsed.start();
    m_wait_next_cmd = false;

    qDebug() << "Opened port:" << portName;
    processProduceConnect("");
}

void HandleModeProduce::onProduceModeReadyRead()
{
    if(!m_serial) return;

    QByteArray data = m_serial->readAll();
    if(m_establish) {
        emit dataReady(data);
    }
    else {
        processProduceConnect(data);
    }
}

void HandleModeProduce::processProduceConnect(const QByteArray &data)
{
    m_produce_buffer.append(data);

    switch(m_step) {
        case PRODUCE_HANDSHAKE:
        {
            sendCMD("DD3C000310CDFF");
            QByteArray expected_handshake = QByteArray::fromHex("DE3A000311CEFF");
            if(m_produce_buffer.contains(expected_handshake)) {
                m_step = PRODUCE_DATA_REQUEST;
                m_wait_next_cmd = false;
                m_produce_buffer.clear();
                sendCMD("DD3C000340CDFF");
            }
        }
        break;
        case PRODUCE_DATA_REQUEST:
        {
            if(doProduceFrameExtra()) {
                m_step = PRODUCE_FINISH;
            }
        }
        break;
        case PRODUCE_FINISH:
            m_establish = true;
            emit connectEstablished();
            break;
        default:
            break;
    }
}

void HandleModeProduce::sendCMD(const QString &text)
{
    if(!m_serial || m_wait_next_cmd) return;

    m_serial->write(QByteArray::fromHex(text.toUtf8()));
    m_wait_next_cmd = true;

    // 1秒后允许发送下一条命令
    QTimer::singleShot(1000, [this]() { m_wait_next_cmd = false; });
}

bool HandleModeProduce::doProduceFrameExtra() {
    while (true) {
        if (m_frameTypes.isEmpty()) {
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
            if (m_produce_buffer.size() > 10 * 1024) {
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

void HandleModeProduce::onProduceModeTimeout()
{
    qDebug() << "Port timeout, trying next port";
    ++m_port_index;
    tryNextPort();
}
