#include "formserial.h"
#include "keydef.h"
#include "ui_formserial.h"

#include <QFile>
#include <QFileDialog>

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QToolButton>
#include "../plot/plot_algorithm.h"
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
    delete ui;
}

void FormSerial::retranslateUI()
{
    ui->retranslateUi(this);
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

    QStringList groups = SETTING_FRAME_GROUPS();
    if (!groups.empty()) {
        for (const auto &g : groups) {
            FrameType frame;
            frame.name = g;
            frame.header = QByteArray::fromHex(SETTING_FRAME_GET(g, FRAME_HEADER).toUtf8());
            frame.footer = QByteArray::fromHex(SETTING_FRAME_GET(g, FRAME_FOOTER).toUtf8());
            frame.length = SETTING_FRAME_GET(g, FRAME_LENGTH).toInt();
            m_frameTypes.push_back(frame);
        }
    } else {
        m_frameTypes = {
            {"curve_24bit", QByteArray::fromHex("DE3A096631"), QByteArray::fromHex("CEFF"), 1990},
            {"curve_14bit", QByteArray::fromHex("DE3A096633"), QByteArray::fromHex("CEFF"), 0},
        };
        for (const auto &frame : m_frameTypes) {
            SETTING_FRAME_SET(frame.name, FRAME_HEADER, frame.header.toHex().toUpper());
            SETTING_FRAME_SET(frame.name, FRAME_FOOTER, frame.footer.toHex().toUpper());
            SETTING_FRAME_SET(frame.name, FRAME_LENGTH, QString::number(frame.length));
        }
    }
    int current_algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
    m_algorithm = current_algorithm;
    if (current_algorithm == static_cast<int>(SHOW_ALGORITHM::NUM_660)) {
        m_frameTypes = {
            {"curve_24bit", QByteArray::fromHex("DE3A096631"), QByteArray::fromHex("CEFF"), 1990},
        };
    } else if (current_algorithm == static_cast<int>(SHOW_ALGORITHM::PLAY_MPU6050)) {
        m_frameTypes = {
            {"MPU6050", "<MPU>", "<END>"},
        };
    }

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
        SHOW_AUTO_CLOSE_MSGBOX(this, "Error", "Serial not open!");
        return;
    }
    auto res = m_serial->write(bytes);
    if (res == -1) {
        LOG_WARN("Write failed: {}", m_serial->errorString());
    }
}

void FormSerial::onChangeFrameType(int index)
{
    if (index == static_cast<int>(SHOW_ALGORITHM::NUM_660)) {
        m_frameTypes = {
            {"curve_24bit", QByteArray::fromHex("DE3A096631"), QByteArray::fromHex("CEFF"), 1990},
        };
    } else if (index == static_cast<int>(SHOW_ALGORITHM::MAX_NEG_95)
               || index == static_cast<int>(SHOW_ALGORITHM::NORMAL)) {
        m_frameTypes = {
            {"curve_24bit", QByteArray::fromHex("DE3A096631"), QByteArray::fromHex("CEFF"), 1990},
            {"curve_14bit", QByteArray::fromHex("DE3A096633"), QByteArray::fromHex("CEFF"), 0},
        };
    } else if (index == static_cast<int>(SHOW_ALGORITHM::PLAY_MPU6050)) {
        m_frameTypes = {
            {"MPU6050", "<MPU>", "<END>"},
        };
    }
    m_algorithm = index;
}

void FormSerial::initMultSend()
{
    QStringList cmds;
    for (int i = 0; i < 5; ++i) {
        cmds.push_back(
            SETTING_CONFIG_GET(CFG_GROUP_HISTROY, QString("%1_%2").arg(CFG_HISTORY_MULT).arg(i)));
    }
    for (int i = 0; i < 5; ++i) {
        QString lineEditName = QString("lineEdit_cmd_%1").arg(i);
        QLineEdit *lineEdit = findChild<QLineEdit *>(lineEditName);
        if (lineEdit) {
            lineEdit->setText(cmds.at(i));
            connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit, i, &cmds]() {
                SETTING_CONFIG_SET(CFG_GROUP_HISTROY,
                                   QString("%1_%2").arg(CFG_HISTORY_MULT).arg(i),
                                   lineEdit->text());
            });
        }
    }

    QStringList labels;
    for (int i = 0; i < 5; ++i) {
        labels.push_back(SETTING_CONFIG_GET(CFG_GROUP_HISTROY,
                                            QString("%1_%2").arg(CFG_MULT_LABEL).arg(i),
                                            QString("cmd_%1").arg(i)));
    }

    for (int i = 0; i < 5; ++i) {
        QString lineEditName = QString("lineEdit_label_%1").arg(i);
        QLineEdit *lineEdit = findChild<QLineEdit *>(lineEditName);
        if (lineEdit) {
            lineEdit->setPlaceholderText(labels.at(i));
            connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit, i]() {
                SETTING_CONFIG_SET(CFG_GROUP_HISTROY,
                                   QString("%1_%2").arg(CFG_MULT_LABEL).arg(i),
                                   lineEdit->text());
            });
        }
    }

    for (int i = 0; i < 5; ++i) {
        QString tBtnName = QString("tBtn_%1").arg(i);
        QToolButton *tBtn = findChild<QToolButton *>(tBtnName);
        if (tBtn) {
            connect(tBtn, &QToolButton::clicked, this, [this, tBtn, i]() {
                QLineEdit *lineEdit = findChild<QLineEdit *>(QString("lineEdit_cmd_%1").arg(i));
                QString text = lineEdit->text().trimmed();
                if (!text.isEmpty()) {
                    send(text);
                }
            });
        }
    }
}

void FormSerial::init()
{
    // ini
    getINI();
    // init port
    QList<QSerialPortInfo> list_port = QSerialPortInfo::availablePorts();
    if (list_port.isEmpty()) {
        LOG_WARN("No available serial port found!");
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

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [=]() {
        if (m_serial && m_serial->isOpen()) {
            QMetaObject::invokeMethod(m_serial, "close", Qt::QueuedConnection);
        }
    });
    m_send_timer = new QTimer(this);
    connect(m_send_timer, &QTimer::timeout, this, &FormSerial::onAutoSend);
    ui->txtRecv->setMaximumBlockCount(1000);
    setINI();
    m_recv_count = 0;
}

void FormSerial::send(const QString &text)
{
    LOG_INFO("serial send: {}", text);
    if (!(m_serial && m_serial->isOpen())) {
        SHOW_AUTO_CLOSE_MSGBOX(this, "warning", tr("serial not open!"));
        LOG_ERROR("Serial not open!");
        return;
    }

    if (text.isEmpty()) {
        LOG_WARN("Send txt is empty.");
        return;
    }

    QByteArray data;
    QString to_show;
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
        LOG_INFO("send (hex): {}", data);
        to_show = data;
    } else if (m_ini.send_format == VAL_SERIAL_SEND_HEX_TRANSLATE) {
        QByteArray byteArray = text.toUtf8();

        QString hexString;
        for (int i = 0; i < byteArray.length(); ++i) {
            hexString.append(
                QString("%1 ").arg((unsigned char) byteArray[i], 2, 16, QChar('0')).toUpper());
        }
        LOG_INFO("send (hex translate): {}", hexString);
        data = byteArray;
        to_show = hexString;
    } else {
        data = text.toUtf8();
        LOG_INFO("send (normal): {}", data);
        to_show = data;
    }
    auto res = m_serial->write(data);
    if (res == -1) {
        LOG_WARN("Write failed: {}", m_serial->errorString());
    }
    if (m_ini.show_send) {
        ui->txtRecv->appendPlainText("[TX] " + to_show);
    }
}

void FormSerial::on_btnSend_clicked()
{
    QString text = ui->txtSend->toPlainText().trimmed();
    LOG_INFO("serial send: {}", text);
    send(text);
    SETTING_CONFIG_SET(CFG_GROUP_HISTROY, CFG_HISTORY_SINGLE_SEND, text);
}

void FormSerial::on_btnSerialSwitch_clicked()
{
    m_switch = !m_switch;
    if (m_switch) {
        // open serial
        openSerial();
    } else {
        // close serial
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
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
    }
    delete m_serial;
    m_serial = nullptr;

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
}

void FormSerial::handleFrame(const QString &type, const QByteArray &data)
{
    if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::MAX_NEG_95)
        || m_algorithm == static_cast<int>(SHOW_ALGORITHM::NORMAL)) {
        if (type == "curve_24bit") {
            m_frame.bit24 = data;
        } else if (type == "curve_14bit") {
            m_frame.bit14 = data;
            if (!m_frame.bit24.isEmpty()) {
                emit recv2Data4k(m_frame.bit14, m_frame.bit24);
                emit recv2Plot4k(m_frame.bit14, m_frame.bit24);
                m_frame.bit24.clear();
            }
        }
    } else if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::NUM_660)) {
        if (type == "curve_24bit") {
            m_frame.bit24 = data;
            emit recv2Data4k(m_frame.bit14, m_frame.bit24);
            emit recv2Plot4k(m_frame.bit14, m_frame.bit24);
        }
    } else if (m_algorithm == static_cast<int>(SHOW_ALGORITHM::PLAY_MPU6050)) {
        emit recv2MPU(data);
    }
}

void FormSerial::onReadyRead()
{
    QByteArray data = m_serial->readAll();
    m_recv_count += data.size();
    ui->labelRecvCount->setText(QString("recv: %1").arg(m_recv_count));

    QString to_show = data;
    if (m_ini.hex_display) {
        to_show.clear();
        for (int i = 0; i < data.length(); ++i) {
            to_show.append(QString("%1 ").arg((unsigned char) data[i], 2, 16, QChar('0')).toUpper());
        }
    }
    ui->txtRecv->appendPlainText("[RX] " + to_show);
    m_buffer.append(data);

    while (true) {
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

        } else {
            // 长度不固定：查找 footer 位置
            int footerIdx = m_buffer.indexOf(current_frame.footer, current_frame.header.size());
            if (footerIdx == -1) {
                // 没找到帧尾，等待更多数据
                break;
            }

            int frame_len = footerIdx + current_frame.footer.size();
            QByteArray frame_candidate = m_buffer.left(frame_len);
            LOG_INFO("Variable-length frame matched: {}, size = {}",
                     current_frame.name.toStdString(),
                     frame_len);
            handleFrame(current_frame.name, frame_candidate);
            m_buffer.remove(0, frame_len);
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
            QMessageBox::warning(this, "Warning", "Please open serial port!");
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
            QMessageBox::warning(this, "Warning", "Please set valid time(ms)!");
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
            send(msg);
        }
    } else {
        for (int i = 0; i <= 4; ++i) {
            QString msg = findChild<QLineEdit *>(QString("lineEdit_cmd_%1").arg(i))->text();
            if (!msg.isEmpty()) {
                send(msg);
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
    for (const auto &port : ports) {
        currentPorts << port.portName();
    }

    if (currentPorts != m_lastPortList) {
        LOG_INFO("Serial ports changed: {}", currentPorts.join(", ").toStdString());
        m_lastPortList = currentPorts;

        ui->cBoxPortName->clear();
        ui->cBoxPortName->addItems(currentPorts);

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
    }
}
