#include "formserial.h"
#include "ThreadParser/threadparser.h"
#include "keydef.h"
#include "ui_formserial.h"

#include <QFile>
#include <QFileDialog>
#include <QThread>

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QToolButton>
#include "LineSend/linesend.h"
#include "funcdef.h"

FormSerial::FormSerial(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormSerial)
{
    ui->setupUi(this);
    init();
}

FormSerial::~FormSerial()
{
    SETTING_CONFIG_SYNC();
    if(m_workerThread)
    {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    delete ui;
}

void FormSerial::retranslateUI()
{
    ui->retranslateUi(this);
}

enum STEP_EASY_CONNECT {
    CONNECT_PORT = 1,
    HANDSHAKE,
    SET_INTEGRATION_TIME,
    DATA_REQUEST,
    FRAME_CHECK,
    FINISH
};

bool FormSerial::startProduceConnect() {
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    QStringList ports;
    for (const auto &port : availablePorts) {
        ports.append(port.portName());
    }
#ifdef QT_DEBUG
    QString debug_port = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_DEBUG_PORT);
    ports.push_front(debug_port);
#endif
    for (const auto &port : ports) {
        LOG_INFO("test port: {}", port);
        statusReport(100 * CONNECT_PORT / FINISH, QString("connect to port: %1").arg(port));
        closeSerial();
        m_serial = new QSerialPort(this);
        m_serial->setPortName(port);
        m_serial->setBaudRate(115200);
        m_serial->setDataBits(static_cast<QSerialPort::DataBits>(8));
        m_serial->setParity(QSerialPort::NoParity);
        m_serial->setStopBits(QSerialPort::OneStop);
        if (!m_serial->open(QIODevice::ReadWrite)) {
            LOG_ERROR("error: {}", m_serial->errorString());
            continue;
        }
        connect(m_serial,
                &QSerialPort::readyRead,
                this,
                &FormSerial::onReadyRead,
                Qt::UniqueConnection);
        sendEasyData("DD3C000310CDFF");
        if (m_serial->bytesAvailable() > 0 || m_serial->waitForReadyRead(500)) {
            QByteArray response_handshake = m_serial->readAll();
            QByteArray expected_handshake = QByteArray::fromHex("DE3A000311CEFF");
            if(response_handshake.contains(expected_handshake)) {
                sendEasyData("DD3C000340CDFF");
                m_serial->readAll();
                m_serial->clear(QSerialPort::Input);
                m_ready = true;
                return true;
            }
        }
    }
    return false;
}

bool FormSerial::startEasyConnect(const QString& F30_shown_mode)
{
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    QStringList ports;
    for (const auto &port : availablePorts) {
        ports.append(port.portName());
    }
#ifdef QT_DEBUG
    QString debug_port = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_DEBUG_PORT);
    ports.push_front(debug_port);
#endif
    for (const auto &port : ports) {
        LOG_INFO("test port: {}", port);
        statusReport(100 * CONNECT_PORT / FINISH, QString("connect to port: %1").arg(port));
        closeSerial();
        m_serial = new QSerialPort(this);
        m_serial->setPortName(port);
        m_serial->setBaudRate(115200);
        m_serial->setDataBits(static_cast<QSerialPort::DataBits>(8));
        m_serial->setParity(QSerialPort::NoParity);
        m_serial->setStopBits(QSerialPort::OneStop);
        if (!m_serial->open(QIODevice::ReadWrite)) {
            LOG_ERROR("error: {}", m_serial->errorString());
            continue;
        }
        connect(m_serial,
                &QSerialPort::readyRead,
                this,
                &FormSerial::onReadyRead,
                Qt::UniqueConnection);
        statusReport(100 * HANDSHAKE / FINISH, QString("%1 step [handshake]: start").arg(port));
        m_serial->clear(QSerialPort::Input);
        sendEasyData("DD3C000310CDFF");
        statusReport(100 * HANDSHAKE / FINISH,
                     QString("%1 step [handshake]: wait response").arg(port));
        if (m_serial->bytesAvailable() > 0 || m_serial->waitForReadyRead(500)) {
            QByteArray response_handshake = m_serial->readAll();
            QByteArray expected_handshake = QByteArray::fromHex("DE3A000311CEFF");
            LOG_INFO("cmd: DD3C000310CDFF -> {}", response_handshake.toHex().toUpper());
            statusReport(100 * HANDSHAKE / FINISH,
                         QString("%1 step [handshake]: check excepted").arg(port));
            if (response_handshake.contains(expected_handshake)) {
                if(F30_shown_mode == CFG_F30_MODE_DOUBLE) {
                    statusReport(100 * DATA_REQUEST / FINISH,
                                 QString("%1 step [data 40 request]: start").arg(port));
                    {
                        sendEasyData("DD3C000368CDFF");
                        QByteArray response;
                        response += m_serial->readAll();
                        while (m_serial->waitForReadyRead(50)) {
                            response += m_serial->readAll();
                        }
                        doThresholdExtra(response);
                    }
                    {
                        sendEasyData("DD3C000370CDFF");
                        QByteArray response;
                        response += m_serial->readAll();
                        while (m_serial->waitForReadyRead(50)) {
                            response += m_serial->readAll();
                        }
                        doBaselineExtra(response);
                    }
                    sendEasyData("DD3C000340CDFF");
                }
                else if(F30_shown_mode == CFG_F30_MODE_SINGLE) {
                    statusReport(100 * SET_INTEGRATION_TIME / FINISH,
                                 QString("%1 step [set integration time]: check excepted")
                                     .arg(port));
                    sendEasyData("DD3C000622000005CDFF");
                    m_serial->waitForReadyRead(500);
                    QByteArray response = m_serial->readAll();
                    QByteArray except = QByteArray::fromHex("DE3A000323CEFF");
                    LOG_INFO("cmd: DD3C000622000005CDFF -> {}", response.toHex().toUpper());
                    if (response == except) {
                        statusReport(100 * DATA_REQUEST / FINISH,
                                     QString("%1 step [data 30 request]: start").arg(port));
                        sendEasyData("DD3C000330CDFF");
                    }
                }

                m_serial->readAll();
                m_serial->clear(QSerialPort::Input);
                m_ready = true;
                return true;
            } else {
                statusReport(100 * FRAME_CHECK / FINISH,
                             QString("%1 step [frame check]: start").arg(port));
                if (doFrameExtra(response_handshake)) {
                    statusReport(100 * FRAME_CHECK / FINISH,
                                 QString("%1 step [frame check]: success.").arg(port));
                    m_serial->readAll();
                    m_serial->clear(QSerialPort::Input);
                    m_ready = true;
                    return true;
                } else {
                    statusReport(100 * FRAME_CHECK / FINISH,
                                 QString("%1 step [frame check]: fail!").arg(port));
                }
            }
        } else {
            statusReport(100 * HANDSHAKE / FINISH,
                         QString("%1 step [handshake]: no response").arg(port));
        }
    }
    return false;
}

void FormSerial::doBaselineExtra(const QByteArray &data) {
    const QByteArray header = QByteArray::fromHex("DE3A000671");
    const QByteArray tail   = QByteArray::fromHex("CEFF");
    int headPos = data.indexOf(header);
    if (headPos < 0) {
        LOG_WARN("Baseline: not found header: DE3A000671");
        return;
    }

    int tailPos = data.indexOf(tail, headPos + header.size());
    if (tailPos < 0) {
        LOG_WARN("Baseline: not found tail: CEFF");
        return;
    }

    int payloadStart = headPos + header.size();
    int payloadLen = tailPos - payloadStart;

    QByteArray payload = data.mid(payloadStart, payloadLen);
    int value = (static_cast<quint8>(payload[0]) << 16) |
                static_cast<quint8>(payload[1]) << 8 |
                static_cast<quint8>(payload[2]);
    emit sendOption({{"baseline", value}});
}

void FormSerial::doThresholdExtra(const QByteArray &data)
{
    const QByteArray header = QByteArray::fromHex("DE3A064569");
    const QByteArray tail   = QByteArray::fromHex("CEFF");

    int headPos = data.indexOf(header);
    if (headPos < 0) {
        LOG_WARN("Threshold: not found header: DE3A064569");
        return;
    }

    int tailPos = data.indexOf(tail, headPos + header.size());
    if (tailPos < 0) {
        LOG_WARN("Threshold: not found tail: CEFF");
        return;
    }


    int payloadStart = headPos + header.size();
    int payloadLen = tailPos - payloadStart;

    QByteArray payload = data.mid(payloadStart, payloadLen);

    QList<double> values;
    for (int i = 0; i + 1 < payload.size(); i += 2) {
        qint16 v = (static_cast<quint8>(payload[i]) << 8) |
                    static_cast<quint8>(payload[i + 1]);
        values.append(v);
    }
    emit sendThreshold(true, values);
}

void FormSerial::stopFSeriesConnect()
{
    m_ready = false;

    if (m_serial) {
        disconnect(m_serial, &QSerialPort::readyRead,
                   this, &FormSerial::onReadyRead);
        m_serial->clear(QSerialPort::AllDirections);
        QThread::msleep(50);
        sendExpertData("DD3C000360CDFF");
        if (!m_serial->waitForBytesWritten(500)) {
            LOG_WARN("stop Easy connect: write timeout");
        }
        m_serial->flush();
        QThread::msleep(200);
    }

    closeSerial();
}

void FormSerial::sendEasyData(const QString &text)
{
    LOG_INFO("serial send: {}", text);
    if (!(m_serial && m_serial->isOpen())) {
        SHOW_AUTO_CLOSE_MSGBOX(this, TITLE_WARNING, tr("serial not open!"));
        LOG_ERROR("Serial not open!");
        return;
    }

    QByteArray data;
    QString cleaned = text;
    cleaned.remove(QRegularExpression("[^0-9A-Fa-f\\s]"));

    QStringList byteStrings;
    if (cleaned.contains(QRegularExpression("\\s+"))) {
        byteStrings = cleaned.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    } else {
        for (int i = 0; i + 1 < cleaned.length(); i += 2) {
            byteStrings << cleaned.mid(i, 2);
        }
    }

    for (const QString &byteStr : byteStrings) {
        bool ok;
        int byte = byteStr.toInt(&ok, 16);
        if (ok) {
            data.append(static_cast<char>(byte));
        } else {
            LOG_WARN("illegal hex: {}", byteStr);
        }
    }
    m_serial->write(data);
}

void FormSerial::updateFrameTypes(const QString &algorithm)
{
    m_frameTypes.clear();
    if (m_algorithm == "F15_curves") {
        QStringList groups = SETTING_FRAME_F15Curves_GROUPS();
        if (!groups.empty()) {
            for (const auto &g : groups) {
                FrameType frame;
                frame.name = g;
                frame.header = QByteArray::fromHex(
                    SETTING_FRAME_F15Curves_GET(g, FRAME_HEADER).toUtf8());
                frame.footer = QByteArray::fromHex(
                    SETTING_FRAME_F15Curves_GET(g, FRAME_FOOTER).toUtf8());
                frame.length = SETTING_FRAME_F15Curves_GET(g, FRAME_LENGTH).toInt();
                m_frameTypes.push_back(frame);
            }
        } else {
            m_frameTypes = {
                            {"F15_31", QByteArray::fromHex("DE3A096631"), QByteArray::fromHex("CEFF"), 6007},
                            {"F15_33", QByteArray::fromHex("DE3A096633"), QByteArray::fromHex("CEFF"), 6007},
                            };
            for (const auto &frame : m_frameTypes) {
                SETTING_FRAME_F15Curves_SET(frame.name,
                                            FRAME_HEADER,
                                            frame.header.toHex().toUpper());
                SETTING_FRAME_F15Curves_SET(frame.name,
                                            FRAME_FOOTER,
                                            frame.footer.toHex().toUpper());
                SETTING_FRAME_F15Curves_SET(frame.name, FRAME_LENGTH, QString::number(frame.length));
            }
        }
    } else if (m_algorithm == "F15_single") {
        QStringList groups = SETTING_FRAME_F15Single_GROUPS();
        if (!groups.empty()) {
            for (const auto &g : groups) {
                FrameType frame;
                frame.name = g;
                frame.header = QByteArray::fromHex(
                    SETTING_FRAME_F15Single_GET(g, FRAME_HEADER).toUtf8());
                frame.footer = QByteArray::fromHex(
                    SETTING_FRAME_F15Single_GET(g, FRAME_FOOTER).toUtf8());
                frame.length = SETTING_FRAME_F15Single_GET(g, FRAME_LENGTH).toInt();
                m_frameTypes.push_back(frame);
            }
        } else {
            m_frameTypes = {
                            {"F15_31", QByteArray::fromHex("DE3A096631"), QByteArray::fromHex("CEFF"), 1612},
                            };
            for (const auto &frame : m_frameTypes) {
                SETTING_FRAME_F15Single_SET(frame.name,
                                            FRAME_HEADER,
                                            frame.header.toHex().toUpper());
                SETTING_FRAME_F15Single_SET(frame.name,
                                            FRAME_FOOTER,
                                            frame.footer.toHex().toUpper());
                SETTING_FRAME_F15Single_SET(frame.name, FRAME_LENGTH, QString::number(frame.length));
            }
        }
    } else if (m_algorithm == "Play_mpu6050") {
        QStringList groups = SETTING_FRAME_PLAY_MPU6050_GROUPS();
        if (!groups.empty()) {
            for (const auto &g : groups) {
                FrameType frame;
                frame.name = g;
                frame.header = QByteArray::fromHex(
                    SETTING_FRAME_PLAY_MPU6050_GET(g, FRAME_HEADER).toUtf8());
                frame.footer = QByteArray::fromHex(
                    SETTING_FRAME_PLAY_MPU6050_GET(g, FRAME_FOOTER).toUtf8());
                frame.length = SETTING_FRAME_PLAY_MPU6050_GET(g, FRAME_LENGTH).toInt();
                m_frameTypes.push_back(frame);
            }
        } else {
            m_frameTypes = {
                            {"MPU6050", QByteArray::fromHex("DE3A177331"), QByteArray::fromHex("CEFF"), 6007},
                            };
            for (const auto &frame : m_frameTypes) {
                SETTING_FRAME_PLAY_MPU6050_SET(frame.name,
                                               FRAME_HEADER,
                                               frame.header.toHex().toUpper());
                SETTING_FRAME_PLAY_MPU6050_SET(frame.name,
                                               FRAME_FOOTER,
                                               frame.footer.toHex().toUpper());
                SETTING_FRAME_PLAY_MPU6050_SET(frame.name,
                                               FRAME_LENGTH,
                                               QString::number(frame.length));
            }
        }
    } else if (m_algorithm == "F30_curves") {
        QStringList groups = SETTING_FRAME_F30Curves_GROUPS();
        if (!groups.empty()) {
            for (const auto &g : groups) {
                FrameType frame;
                frame.name = g;
                frame.header = QByteArray::fromHex(
                    SETTING_FRAME_F30Curves_GET(g, FRAME_HEADER).toUtf8());
                frame.footer = QByteArray::fromHex(
                    SETTING_FRAME_F30Curves_GET(g, FRAME_FOOTER).toUtf8());
                frame.length = SETTING_FRAME_F30Curves_GET(g, FRAME_LENGTH).toInt();
                m_frameTypes.push_back(frame);
            }
        } else {
            m_frameTypes = {
                            {"F30_31", QByteArray::fromHex("DE3A177331"), QByteArray::fromHex("CEFF"), 6007},
                            {"F30_33", QByteArray::fromHex("DE3A177333"), QByteArray::fromHex("CEFF"), 6007},
                            };
            for (const auto &frame : m_frameTypes) {
                SETTING_FRAME_F30Curves_SET(frame.name,
                                            FRAME_HEADER,
                                            frame.header.toHex().toUpper());
                SETTING_FRAME_F30Curves_SET(frame.name,
                                            FRAME_FOOTER,
                                            frame.footer.toHex().toUpper());
                SETTING_FRAME_F30Curves_SET(frame.name, FRAME_LENGTH, QString::number(frame.length));
            }
        }
    } else if (m_algorithm == "F30_single") {
        QStringList groups = SETTING_FRAME_F30Single_GROUPS();
        if (!groups.empty()) {
            for (const auto &g : groups) {
                FrameType frame;
                frame.name = g;
                frame.header = QByteArray::fromHex(
                    SETTING_FRAME_F30Single_GET(g, FRAME_HEADER).toUtf8());
                frame.footer = QByteArray::fromHex(
                    SETTING_FRAME_F30Single_GET(g, FRAME_FOOTER).toUtf8());
                frame.length = SETTING_FRAME_F30Single_GET(g, FRAME_LENGTH).toInt();
                m_frameTypes.push_back(frame);
            }
        } else {
            m_frameTypes = {
                            {"F30_31", QByteArray::fromHex("DE3A064331"), QByteArray::fromHex("CEFF"), 1607},
                            };
            for (const auto &frame : m_frameTypes) {
                SETTING_FRAME_F30Single_SET(frame.name,
                                            FRAME_HEADER,
                                            frame.header.toHex().toUpper());
                SETTING_FRAME_F30Single_SET(frame.name,
                                            FRAME_FOOTER,
                                            frame.footer.toHex().toUpper());
                SETTING_FRAME_F30Single_SET(frame.name, FRAME_LENGTH, QString::number(frame.length));
            }
        }
    } else if (m_algorithm == "LLC_curves") {
        QStringList groups = SETTING_FRAME_LLC_GROUPS();
        for (const auto &g : groups) {
            FrameType frame;
            frame.name = g;
            frame.header = QByteArray::fromHex(SETTING_FRAME_LLC_GET(g, FRAME_HEADER).toUtf8());
            frame.footer = QByteArray::fromHex(SETTING_FRAME_LLC_GET(g, FRAME_FOOTER).toUtf8());
            frame.length = SETTING_FRAME_LLC_GET(g, FRAME_LENGTH).toInt();
            m_frameTypes.push_back(frame);
        }
    } else {
        m_frameTypes = {};
    }
    QMetaObject::invokeMethod(
        m_parser,
        [this]()
        {
            m_parser->setFrameTypes(m_frameTypes);
        },
        Qt::QueuedConnection);
}

void FormSerial::getINI()
{
    m_ini.port_name = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT_NAME);
    m_ini.baud_rate = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_BAUD_RATE);
    m_ini.send_format = SETTING_CONFIG_GET(CFG_GROUP_SERIAL,
                                           CFG_SERIAL_SEND_FORMAT,
                                           VAL_SERIAL_SEND_NORMAL);
    m_ini.show_send = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_SHOW_SEND, VAL_ENABLE)
                              == VAL_ENABLE
                          ? true
                          : false;
    m_ini.hex_display = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_HEX_DISPLAY, VAL_DISABLE)
                                == VAL_ENABLE
                            ? true
                            : false;
    m_ini.debug_port = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_DEBUG_PORT);
    m_ini.cycle = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_CYCLE, "1000");
    m_ini.send_page = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_PAGE, VAL_PAGE_SINGLE);
    m_ini.single_send = SETTING_CONFIG_GET(CFG_GROUP_HISTROY, CFG_HISTORY_SINGLE_SEND);

    initMultSend();
}

void FormSerial::setINI()
{
#ifdef QT_DEBUG
    ui->cBoxPortName->addItem(m_ini.debug_port);
    on_cBoxPortName_activated(0);
    ui->cBoxPortName->setCurrentText(m_ini.debug_port);
#else
    ui->cBoxPortName->setCurrentText(m_ini.port_name);
    on_cBoxPortName_activated(0);
#endif
    ui->cBoxBaudRate->setCurrentText(m_ini.baud_rate);
    on_cBoxSendFormat_currentTextChanged(m_ini.send_format);
    ui->checkBoxShowSend->setChecked(m_ini.show_send);
    ui->checkBoxHexDisplay->setChecked(m_ini.hex_display);
    ui->lineEditCycle->setText(m_ini.cycle);
    if (m_ini.send_page == VAL_PAGE_SINGLE) {
        ui->tabWidget->setCurrentWidget(ui->tabSingle);
    } else {
        ui->tabWidget->setCurrentWidget(ui->tabMultipe);
    }
    ui->txtSend->setPlainText(m_ini.single_send);
}

void FormSerial::sendRaw(const QByteArray &bytes)
{
    if (!m_serial) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error"), tr("Serial not open!"));
        return;
    }
    auto res = m_serial->write(bytes);
    if (res == -1) {
        LOG_WARN("Write failed: {}", m_serial->errorString());
    }
}

void FormSerial::onChangeFrameType(const QString &algorithm)
{
    m_algorithm = algorithm;
    updateFrameTypes(m_algorithm);
}

bool FormSerial::doFrameExtra(const QByteArray &data)
{
    m_recv_count += data.size();
    ui->labelRecvCount->setText(QString("recv: %1").arg(m_recv_count));
    m_buffer.append(data);

    QString to_show = data;
    if (m_ini.hex_display) {
        to_show.clear();
        for (int i = 0; i < data.length(); ++i) {
            to_show.append(QString("%1 ").arg((unsigned char) data[i], 2, 16, QChar('0')).toUpper());
        }
        ui->txtRecv->setUpdatesEnabled(false);
        ui->txtRecv->appendPlainText(to_show);
        ui->txtRecv->setUpdatesEnabled(true);
        if (ui->txtRecv->document()->characterCount() > 200000)
        {
            ui->txtRecv->clear();
        }
    }

    while (true) {
        if (m_frameTypes.isEmpty()) {
            m_buffer.clear();
            return false;
        }
        int firstHeaderIdx = -1;
        FrameType current_frame;

        // 查找所有已知帧头
        for (const auto &type : m_frameTypes) {
            int idx = m_buffer.indexOf(type.header);
            if (idx != -1 && (firstHeaderIdx == -1 || idx < firstHeaderIdx)) {
                firstHeaderIdx = idx;
                current_frame = type;
            }
        }

        // 没有帧头，清理或等待
        if (firstHeaderIdx == -1) {
            if (m_buffer.size() > 10 * 1024) {
                LOG_WARN("Buffer overflow, clearing");
                m_buffer.clear();
            }
            break;
        }

        // 丢弃无效数据
        if (firstHeaderIdx > 0) {
            LOG_WARN("Dropping invalid data before header: {} bytes", firstHeaderIdx);
            m_buffer.remove(0, firstHeaderIdx);
        }

        if (current_frame.length != 0) {
            // 长度固定帧
            if (m_buffer.size() < current_frame.length)
                break;

            QByteArray frame_candidate = m_buffer.left(current_frame.length);
            if (!frame_candidate.endsWith(current_frame.footer)) {
                LOG_WARN("Invalid footer (fixed length), removing header only");
                m_buffer.remove(0, current_frame.header.size());
                continue;
            }

            LOG_INFO("Fixed-length frame matched: {}", current_frame.name.toStdString());
            handleFrame(current_frame.name, frame_candidate);
            m_buffer.remove(0, current_frame.length);
            return true;
        } else {
            // 长度不固定：查找 footer 位置
            int footerIdx = m_buffer.indexOf(current_frame.footer, current_frame.header.size());
            if (footerIdx == -1) {
                // 没找到帧尾，等待更多数据
                break;
            }

            int frame_len = footerIdx + current_frame.footer.size();
            QByteArray frame_candidate;
            frame_candidate = m_buffer.left(frame_len);

            LOG_INFO("Variable-length frame matched: {}, size = {}",
                     current_frame.name.toStdString(),
                     frame_len);
            handleFrame(current_frame.name, frame_candidate);

            m_buffer.remove(0, frame_len);
            return true;
        }
    }
    return false;
}

void FormSerial::onSimulateRecv(const QByteArray &bytes)
{
    doFrameExtra(bytes);
}

void FormSerial::clearData()
{
    m_buffer.clear();
}

void FormSerial::closeEvent(QCloseEvent *event)
{
    closeSerial();
    event->accept();
}

void FormSerial::initMultSend()
{
    int total = 30;
    m_lineSends.reserve(total);
    for (int i = 0; i < total; ++i) {
        auto lineSend = new LineSend(i, ui->vLayMultipe->widget());
        lineSend->setBtn(QString::number(i));

        QString cmd = SETTING_CONFIG_GET(CFG_GROUP_HISTROY,
                                         QString("%1_%2").arg(CFG_HISTORY_MULT).arg(i));
        lineSend->setCmd(cmd);

        QString label = SETTING_CONFIG_GET(CFG_GROUP_HISTROY,
                                           QString("%1_%2").arg(CFG_MULT_LABEL).arg(i),
                                           QString("cmd_%1").arg(i));
        lineSend->setLabel(label);

        connect(lineSend, &LineSend::cmdSendClicked, this, [this](int index, const QString &cmd) {
            if (!cmd.isEmpty()) {
                sendExpertData(cmd);
            }
        });

        connect(lineSend, &LineSend::cmdEdited, this, [this](int index, const QString &cmd) {
            SETTING_CONFIG_SET(CFG_GROUP_HISTROY,
                               QString("%1_%2").arg(CFG_HISTORY_MULT).arg(index),
                               cmd);
        });

        connect(lineSend, &LineSend::labelEdited, this, [this](int index, const QString &label) {
            SETTING_CONFIG_SET(CFG_GROUP_HISTROY,
                               QString("%1_%2").arg(CFG_MULT_LABEL).arg(index),
                               label);
        });

        m_lineSends.append(lineSend);
    }

    loadPage(0);

    ui->labelPage->setText("1 / 6");
}

void FormSerial::init()
{
    // ini
    getINI();
    // init port
    QList<QSerialPortInfo> list_port = QSerialPortInfo::availablePorts();
    if (list_port.isEmpty()) {
        LOG_WARN("No available serial port found!");
        SHOW_AUTO_CLOSE_MSGBOX(this, TITLE_WARNING, tr("No available serial port found!"));
        return;
    }
    QStringList port_names;
    for (const auto &port : list_port) {
        SERIAL serial;
        serial.SerialNumber = port.serialNumber();
        serial.Description = port.description();
        serial.Manufacturer = port.manufacturer();
        serial.StandardBaudRates = port.standardBaudRates();
        serial.SystemLocation = port.systemLocation();
        m_mapSerial.insert(port.portName(), serial);
        port_names.push_back(port.portName());
    }

    ui->tBtnRefresh->setObjectName("refresh");

    m_switch = false;
    ui->btnSerialSwitch->setText("To Open");
    ui->cBoxPortName->addItems(port_names);

    ui->cBoxDataBit->addItems({"8", "7", "6", "5"});
    ui->cBoxCheckBit->addItems({"None", "Even", "Mark", "Odd"});
    ui->cBoxStopBit->addItems({"1", "1.5", "2"});
    // init send
    ui->btnSend->setIcon(QIcon(":/res/icons/send-on.png"));
    ui->btnSend->setIconSize(QSize(32, 32));

    // TODO
    ui->groupBoxEnhancement->hide();

    QSignalBlocker blocker(ui->cBoxSendFormat);
    ui->cBoxSendFormat->addItems(
        {VAL_SERIAL_SEND_NORMAL, VAL_SERIAL_SEND_HEX, VAL_SERIAL_SEND_HEX_TRANSLATE});

    m_send_timer = new QTimer(this);
    connect(m_send_timer, &QTimer::timeout, this, &FormSerial::onAutoSend);
    ui->txtRecv->document()->setMaximumBlockCount(1000);
    ui->txtRecv->setUndoRedoEnabled(false);
    setINI();
    m_recv_count = 0;

    ui->tBtnNext->setObjectName("go-next");
    ui->tBtnPrev->setObjectName("go-prev");

    m_workerThread = new QThread(this);
    m_parser = new ThreadParser();

    m_parser->moveToThread(m_workerThread);

    connect(this,
            &FormSerial::pushParserData,
            m_parser,
            &ThreadParser::pushData,
            Qt::QueuedConnection);

    connect(m_parser,
            &ThreadParser::frameParsed,
            this,
            &FormSerial::handleFrame,
            Qt::QueuedConnection);

    m_workerThread->start();
}

void FormSerial::sendExpertData(const QString &text)
{
    LOG_INFO("serial send: {}", text);
    if (!(m_serial && m_serial->isOpen())) {
        SHOW_AUTO_CLOSE_MSGBOX(this, TITLE_WARNING, tr("serial not open!"));
        LOG_ERROR("Serial not open!");
        return;
    }

    if (text.isEmpty()) {
        LOG_WARN("Send txt is empty.");
        return;
    }

    QByteArray data;
    QString to_show;
    QString flag = "";
    if (m_ini.send_format == VAL_SERIAL_SEND_HEX) {
        QString cleaned = text;
        cleaned.remove(QRegularExpression("[^0-9A-Fa-f\\s]"));

        QStringList byteStrings;
        if (cleaned.contains(QRegularExpression("\\s+"))) {
            byteStrings = cleaned.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        } else {
            for (int i = 0; i + 1 < cleaned.length(); i += 2) {
                byteStrings << cleaned.mid(i, 2);
            }
        }

        for (const QString &byteStr : byteStrings) {
            bool ok;
            int byte = byteStr.toInt(&ok, 16);
            if (ok) {
                data.append(static_cast<char>(byte));
            } else {
                LOG_WARN("illegal hex: {}", byteStr);
            }
        }

        flag = "send (hex)";
    } else if (m_ini.send_format == VAL_SERIAL_SEND_HEX_TRANSLATE) {
        QByteArray byteArray = text.toUtf8();

        QString hexString;
        for (int i = 0; i < byteArray.length(); ++i) {
            hexString.append(
                QString("%1 ").arg((unsigned char) byteArray[i], 2, 16, QChar('0')).toUpper());
        }
        data = byteArray;
        flag = "send (hex translate)";
    } else {
        data = text.toUtf8();
        flag = "send (normal)";
    }
    auto res = m_serial->write(data);
    if (res == -1) {
        LOG_WARN("Write failed: {}", m_serial->errorString());
    }
    QStringList hexList;
    for (char byte : data) {
        hexList << QString("%1")
        .arg(static_cast<unsigned char>(byte), 2, 16, QLatin1Char('0'))
            .toUpper();
    }
    to_show = hexList.join(" ");
    if (m_ini.show_send) {
        ui->txtRecv->setUpdatesEnabled(false);
        ui->txtRecv->appendPlainText(QString("[TX] %1:\n%2").arg(TIMESTAMP_0()).arg(to_show));
        ui->txtRecv->setUpdatesEnabled(true);
    }
    LOG_INFO("{}: {}", flag, to_show);
}

void FormSerial::sendProduceData(const QString &text, std::function<bool(const QByteArray &)> func) {
    LOG_INFO("serial send: {}", text);
    if (!(m_serial && m_serial->isOpen())) {
        SHOW_AUTO_CLOSE_MSGBOX(this, TITLE_WARNING, tr("serial not open!"));
        LOG_ERROR("Serial not open!");
        return;
    }

    QByteArray data;
    QString cleaned = text;
    cleaned.remove(QRegularExpression("[^0-9A-Fa-f\\s]"));

    QStringList byteStrings;
    if (cleaned.contains(QRegularExpression("\\s+"))) {
        byteStrings = cleaned.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    } else {
        for (int i = 0; i + 1 < cleaned.length(); i += 2) {
            byteStrings << cleaned.mid(i, 2);
        }
    }

    for (const QString &byteStr : byteStrings) {
        bool ok;
        int byte = byteStr.toInt(&ok, 16);
        if (ok) {
            data.append(static_cast<char>(byte));
        } else {
            LOG_WARN("illegal hex: {}", byteStr);
        }
    }
    m_call_produce_func = func;
    m_serial->write(data);
}

void FormSerial::on_btnSend_clicked()
{
    QString text = ui->txtSend->toPlainText().trimmed();
    LOG_INFO("serial send: {}", text);
    sendExpertData(text);
    SETTING_CONFIG_SET(CFG_GROUP_HISTROY, CFG_HISTORY_SINGLE_SEND, text);
}

void FormSerial::on_btnSerialSwitch_clicked()
{
    m_switch = !m_switch;
    if (m_switch) {
        // open serial
        m_ready = true;
        openSerial();
    } else {
        // close serial
        m_ready = false;
        closeSerial();
        ui->checkBoxScheduledDelivery->setChecked(false);
        on_checkBoxScheduledDelivery_clicked();
    }
}

void FormSerial::openSerial()
{
    LOG_INFO("open serial");
    m_serial = new QSerialPort(this);
    QString port_name = ui->cBoxPortName->currentText();
    m_serial->setPortName(port_name);
    int baud_rate = ui->cBoxBaudRate->currentText().toInt();
    m_serial->setBaudRate(baud_rate);
    int data_bits = ui->cBoxDataBit->currentText().toInt();
    m_serial->setDataBits(static_cast<QSerialPort::DataBits>(data_bits));

    QString check = ui->cBoxCheckBit->currentText();
    if (check == "None")
        m_serial->setParity(QSerialPort::NoParity);
    else if (check == "Even")
        m_serial->setParity(QSerialPort::EvenParity);
    else if (check == "Odd")
        m_serial->setParity(QSerialPort::OddParity);
    else if (check == "Mark")
        m_serial->setParity(QSerialPort::MarkParity);

    QString stop_bits = ui->cBoxStopBit->currentText();
    if (stop_bits == "1")
        m_serial->setStopBits(QSerialPort::OneStop);
    else if (stop_bits == "1.5")
        m_serial->setStopBits(QSerialPort::OneAndHalfStop);
    else if (stop_bits == "2")
        m_serial->setStopBits(QSerialPort::TwoStop);

    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        LOG_WARN("Failed to open serial port: {}", m_serial->errorString());
        QMessageBox::warning(this,
                             "Warning",
                             QString("Failed to open serial port: %1").arg(m_serial->errorString()));
        delete m_serial;
        m_serial = nullptr;
        m_switch = false;
        return;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &FormSerial::onReadyRead);

    ui->btnSerialSwitch->setText("To Close");
    ui->btnSerialSwitch->setStyleSheet("background-color: green; color: white;");

    SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT_NAME, ui->cBoxPortName->currentText());
    SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_BAUD_RATE, ui->cBoxBaudRate->currentText());
}

void FormSerial::closeSerial()
{
    LOG_INFO("close serial");
    if (m_serial) {
        m_serial->disconnect(this);

        if (m_serial->isOpen()) {
            m_serial->close();
        }
        delete m_serial;
        m_serial = nullptr;
    }

    ui->btnSerialSwitch->setText("To Open");
    ui->btnSerialSwitch->setStyleSheet("background-color: white; color: black;");
}

void FormSerial::on_cBoxPortName_activated(int index)
{
    // clear
    ui->cBoxBaudRate->clear();
    // change
    QString name = m_mapSerial.firstKey();
    QStringList list_txt_bauds;
    for (qint32 rate : m_mapSerial[name].StandardBaudRates) {
        list_txt_bauds << QString::number(rate);
    }
    ui->cBoxBaudRate->addItems(list_txt_bauds);
    if (!m_ini.baud_rate.isEmpty() && list_txt_bauds.contains(m_ini.baud_rate)) {
        ui->cBoxBaudRate->setCurrentText(m_ini.baud_rate);
    }
}

void FormSerial::handleFrame(const QString &type, const QByteArray &data, const QByteArray &temp)
{
    if (m_algorithm == "F30_curves") {
        if (type == "F30_31") {
            m_frame.data31 = data;
        } else if (type == "F30_33") {
            if (!m_frame.data31.isEmpty()) {
                double temperature = 0;
                QByteArray frame_temperature = "";

                if (m_acceptTemperature) {
                    QByteArray frame_data = data.left(data.size() - (2 + 4));
                    m_frame.data33 = frame_data;
                    frame_temperature = data.right(2 + 4);
                    if (frame_temperature.size() >= 4) {
                        long long raw_temperature = (static_cast<quint8>(frame_temperature[0])
                                                     << 8 * 3)
                                                    | (static_cast<quint8>(frame_temperature[1])
                                                       << 8 * 2)
                                                    | (static_cast<quint8>(frame_temperature[2])
                                                       << 8)
                                                    | (static_cast<quint8>(frame_temperature[3]));

                        temperature = raw_temperature / 10000.0;
                    }
                } else {
                    m_frame.data33 = data;
                }
                m_frame.temperature = frame_temperature;
                emit recv2PlotF30(m_frame, temperature);
                m_frame.Clear();
            }
        }
    } else if (m_algorithm == "F30_single") {
        emit recv2PlotF30(m_frame, {});
    } else if (m_algorithm == "F15_curves") {
        if (type == "F15_31") {
            m_frame.data31 = data;
        } else if (type == "F15_33") {
            m_frame.data33 = data;
            if (!m_frame.data31.isEmpty()) {
                emit recv2PlotF15(m_frame);
                m_frame.Clear();
            }
        }
    } else if (m_algorithm == "F15_single") {
        if (type == "F15_31") {
            emit recv2PlotF15(m_frame, {});
        }
    } else if (m_algorithm == "Play_mpu6050") {
        emit recv2MPU(data);
    } else if (m_algorithm == "LLC_curves") {
        if (type == "F30_31") {
            m_frame.data31 = data;
        } else if (type == "F30_33") {
            m_frame.data33 = data;
            if (!m_frame.data31.isEmpty()) {
                emit recv2PlotLLC(m_frame);
                m_frame.Clear();
            }
        }
    }
}

void FormSerial::loadPage(int page)
{
    m_currentPage = page;

    auto *layout = ui->vLayMultipe;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget())
            w->setParent(nullptr);
        delete item;
    }

    int start = page * m_pageSize;
    int end = qMin(start + m_pageSize, m_lineSends.size());
    for (int i = start; i < end; ++i) {
        layout->addWidget(m_lineSends.at(i));
    }

    ui->lineEditPageName->setText(SETTING_CONFIG_GET(CFG_GROUP_HISTROY,
                                                     QString("%1_%2").arg(CFG_MULT_PAGE).arg(page),
                                                     QString("page_%1").arg(page)));
}

void FormSerial::onReadyRead()
{
    if (!m_ready) {
        return;
    }
    QByteArray data = m_serial->readAll();
    emit pushParserData(data);
    if(m_call_produce_func) {
        if(!m_call_produce_func(data)){
            m_call_produce_func = nullptr;
        }
    }
    if(m_ini.hex_display) {
        QString text = data.toHex(' ').toUpper();
        ui->txtRecv->setUpdatesEnabled(false);
        QTextCursor cursor = ui->txtRecv->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n[RX]: " + text + "\n");
        ui->txtRecv->setTextCursor(cursor);
        ui->txtRecv->setUpdatesEnabled(true);
        if (ui->txtRecv->document()->blockCount() > 20000)
        {
            QTextCursor c(ui->txtRecv->document());
            c.movePosition(QTextCursor::Start);
            c.select(QTextCursor::BlockUnderCursor);
            c.removeSelectedText();
            c.deleteChar();
        }
    }
}

void FormSerial::on_btnRecvClear_clicked()
{
    ui->txtRecv->clear();
}

void FormSerial::on_btnRecvSave_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save Received Data");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->txtRecv->toPlainText();
            file.close();
        }
    }
}

void FormSerial::on_checkBoxShowSend_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::CheckState::Checked) {
        m_ini.show_send = true;
        SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SHOW_SEND, VAL_ENABLE);
    } else {
        m_ini.show_send = false;
        SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SHOW_SEND, VAL_DISABLE);
    }
}

void FormSerial::on_cBoxSendFormat_currentTextChanged(const QString &format)
{
    m_ini.send_format = format;
    ui->cBoxSendFormat->setCurrentText(format);
    SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_FORMAT, format);
}

void FormSerial::on_checkBoxHexDisplay_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::CheckState::Checked) {
        m_ini.hex_display = true;
        SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_HEX_DISPLAY, VAL_ENABLE);
    } else {
        m_ini.hex_display = false;
        SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_HEX_DISPLAY, VAL_DISABLE);
    }
}

void FormSerial::on_checkBoxScheduledDelivery_clicked()
{
    if (ui->checkBoxScheduledDelivery->isChecked()) {
        if (!m_switch) {
            QMessageBox::warning(this, TITLE_WARNING, tr("Please open serial port!"));
            ui->checkBoxScheduledDelivery->setChecked(false);
            return;
        }
        QString ms_time = ui->lineEditCycle->text();

        bool ok = false;
        int interval = ms_time.toInt(&ok);
        if (ok && interval > 0) {
            m_send_timer->start(interval);
            onAutoSend();
        } else {
            QMessageBox::warning(this, TITLE_WARNING, tr("Please set valid time(ms)!"));
            ui->checkBoxScheduledDelivery->setChecked(false);
        }
    } else {
        if (m_send_timer && m_send_timer->isActive()) {
            m_send_timer->stop();
        }
    }
}

void FormSerial::onAutoSend()
{
    if (ui->tabWidget->currentWidget() == ui->tabSingle) {
        QString msg = ui->txtSend->toPlainText();
        if (!msg.isEmpty()) {
            sendExpertData(msg);
        }
    } else {
        return;
        for (int i = 0; i <= 4; ++i) {
            QString msg = findChild<QLineEdit *>(QString("lineEdit_cmd_%1").arg(i))->text();
            if (!msg.isEmpty()) {
                sendExpertData(msg);
            }
        }
    }
}

void FormSerial::on_lineEditCycle_editingFinished()
{
    SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_CYCLE, ui->lineEditCycle->text());
}

void FormSerial::on_tabWidget_currentChanged(int index)
{
    if (index == 0) {
        SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_PAGE, VAL_PAGE_SINGLE);
    } else {
        SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_PAGE, VAL_PAGE_MULTIPE);
    }
}

void FormSerial::on_tBtnRefresh_clicked()
{
    QStringList currentPorts;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    if (ports.isEmpty()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, TITLE_WARNING, tr("No ports available!"));
    }

    for (const auto &port : ports) {
        currentPorts << port.portName();
    }

    m_mapSerial.clear();
    for (const auto &port : ports) {
        SERIAL serial;
        serial.SerialNumber = port.serialNumber();
        serial.Description = port.description();
        serial.Manufacturer = port.manufacturer();
        serial.StandardBaudRates = port.standardBaudRates();
        serial.SystemLocation = port.systemLocation();
        m_mapSerial.insert(port.portName(), serial);
    }

    ui->cBoxPortName->clear();
    ui->cBoxPortName->addItems(currentPorts);
}

void FormSerial::on_tBtnNext_clicked()
{
    if (m_currentPage + 1 < 6) {
        ++m_currentPage;
    } else {
        m_currentPage = 5;
    }
    loadPage(m_currentPage);
    ui->labelPage->setText(QString("%1 / %2").arg(m_currentPage + 1).arg(6));
}

void FormSerial::on_tBtnPrev_clicked()
{
    if (m_currentPage - 1 >= 0) {
        --m_currentPage;
    } else {
        m_currentPage = 0;
    }
    loadPage(m_currentPage);
    ui->labelPage->setText(QString("%1 / %2").arg(m_currentPage + 1).arg(6));
}

void FormSerial::on_lineEditPageName_editingFinished()
{
    SETTING_CONFIG_SET(CFG_GROUP_HISTROY,
                       QString("%1_%2").arg(CFG_MULT_PAGE).arg(m_currentPage),
                       ui->lineEditPageName->text());
}

void FormSerial::on_checkBoxAcceptTemperature_clicked()
{
    m_acceptTemperature = ui->checkBoxAcceptTemperature->isChecked();
    if (m_acceptTemperature) {
        for (auto &frame : m_frameTypes) {
            if (frame.name == "F30_33") {
                frame.length += 6;
                break;
            }
        }
    } else {
        for (auto &frame : m_frameTypes) {
            if (frame.name == "F30_33") {
                frame.length -= 6;
                break;
            }
        }
    }
}
