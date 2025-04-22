#include "formserial.h"
#include "keydef.h"
#include "ui_formserial.h"

#include <QFile>
#include <QFileDialog>

#include <QSerialPort>
#include <QSerialPortInfo>

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
    closeSerial();
    delete ui;
}

void FormSerial::setINI()
{
    m_ini.port_name = ui->cBoxPortName->currentText();
    m_ini.baud_rate = ui->cBoxBaudRate->currentText();
    SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT_NAME, m_ini.port_name);
    SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_BAUD_RATE, m_ini.baud_rate);
}

void FormSerial::getINI()
{
    m_ini.port_name = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT_NAME);
    m_ini.baud_rate = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_BAUD_RATE);
    m_ini.test_port_name = SETTING_GET(CFG_GROUP_TEST, CFG_TEST_PORT_NAME);
    m_ini.test_baud_rate = SETTING_GET(CFG_GROUP_TEST, CFG_TEST_BAUD_RATE);
    QString format = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_FORMAT, VAL_SERIAL_SEND_NORMAL);
    on_cBoxSendFormat_currentTextChanged(format);
    m_show_send = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_SHOW_SEND, VAL_ENABLE) == VAL_ENABLE
                      ? true
                      : false;
}

void FormSerial::init()
{
    // init port
    QList<QSerialPortInfo> list_port = QSerialPortInfo::availablePorts();
    if (list_port.isEmpty()) {
        qDebug() << "No available serial port found!";
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
    m_switch = false;
    ui->btnSerialSwitch->setText("To Open");
#ifdef QT_DEBUG
    // ui->cBoxPortName->addItem(m_ini.test_port_name);
    ui->cBoxPortName->addItem("/dev/pts/10");
#endif
    ui->cBoxPortName->addItems(port_names);
    on_cBoxPortName_activated(0);

    ui->cBoxDataBit->addItems({"8", "7", "6", "5"});
    ui->cBoxCheckBit->addItems({"None", "Even", "Mark", "Odd"});
    ui->cBoxStopBit->addItems({"1", "1.5", "2"});
    // init send
    ui->btnSend->setIcon(QIcon(":/res/icons/send-on.png"));
    ui->btnSend->setIconSize(QSize(32, 32));

    // current set
    ui->cBoxBaudRate->setCurrentText("115200");

    // TODO
    ui->groupBoxEnhancement->hide();

    m_show_send = false;
    ui->cBoxSendFormat->addItems(
        {VAL_SERIAL_SEND_NORMAL, VAL_SERIAL_SEND_HEX, VAL_SERIAL_SEND_HEX_TRANSLATE});
    // ini
    getINI();
    ini2UI();
}

void FormSerial::ini2UI()
{
    ui->checkBoxShowSend->setChecked(m_show_send);
    ui->cBoxBaudRate->setCurrentText(m_ini.baud_rate);
}

void FormSerial::on_btnSend_clicked()
{
    QString text = ui->txtSend->toPlainText().trimmed();
    LOG_INFO("serial send: {}", text);

    if (!(m_serialPort && m_serialPort->isOpen())) {
        qDebug() << "Serial port not open.";
        SHOW_AUTO_CLOSE_MSGBOX(this, "warning", "串口未打开！");
        return;
    }

    if (text.isEmpty()) {
        qDebug() << "Send text is empty.";
        return;
    }

    QByteArray data;
    QString to_show;
    if (m_send_format == SEND_FORMAT::HEX) {
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
                qDebug() << "illegal hex: " << byteStr;
            }
        }
        qDebug() << "send (hex):" << data;
        to_show = data;
    } else if (m_send_format == SEND_FORMAT::HEX_TRANSLATE) {
        QByteArray byteArray = text.toUtf8();

        QString hexString;
        for (int i = 0; i < byteArray.length(); ++i) {
            hexString.append(
                QString("%1 ").arg((unsigned char) byteArray[i], 2, 16, QChar('0')).toUpper());
        }
        qDebug() << "send (hex translate):" << hexString;
        data = byteArray;
        to_show = hexString;
    } else {
        data = text.toUtf8();
        qDebug() << "send (normal):" << data;
        to_show = data;
    }
    m_serialPort->write(data);
    if (m_show_send) {
        ui->txtRecv->appendPlainText("[TX] " + to_show);
    }
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
    }
}

void FormSerial::openSerial()
{
    LOG_INFO("open serial");
    m_serialPort = new QSerialPort(this);
    QString portName = ui->cBoxPortName->currentText();
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(ui->cBoxBaudRate->currentText().toInt());
    m_serialPort->setDataBits(
        static_cast<QSerialPort::DataBits>(ui->cBoxDataBit->currentText().toInt()));

    QString check = ui->cBoxCheckBit->currentText();
    if (check == "None")
        m_serialPort->setParity(QSerialPort::NoParity);
    else if (check == "Even")
        m_serialPort->setParity(QSerialPort::EvenParity);
    else if (check == "Odd")
        m_serialPort->setParity(QSerialPort::OddParity);
    else if (check == "Mark")
        m_serialPort->setParity(QSerialPort::MarkParity);

    QString stopBitText = ui->cBoxStopBit->currentText();
    if (stopBitText == "1")
        m_serialPort->setStopBits(QSerialPort::OneStop);
    else if (stopBitText == "1.5")
        m_serialPort->setStopBits(QSerialPort::OneAndHalfStop);
    else if (stopBitText == "2")
        m_serialPort->setStopBits(QSerialPort::TwoStop);

    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open serial port!";
        qDebug() << m_serialPort->errorString();
        delete m_serialPort;
        m_serialPort = nullptr;
        m_switch = false;
        return;
    }

    connect(m_serialPort, &QSerialPort::readyRead, this, &FormSerial::onReadyRead);

    ui->btnSerialSwitch->setText("To Close");
    ui->btnSerialSwitch->setStyleSheet("background-color: green; color: white;");
}

void FormSerial::closeSerial()
{
    LOG_INFO("close serial");
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    delete m_serialPort;
    m_serialPort = nullptr;

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

void FormSerial::onReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    LOG_INFO("serial recv: {}", data);
    QString to_show = data;

    if (m_hex_display) {
        to_show.clear();
        for (int i = 0; i < data.length(); ++i) {
            to_show.append(QString("%1 ").arg((unsigned char) data[i], 2, 16, QChar('0')).toUpper());
        }
    }
    ui->txtRecv->appendPlainText("[RX] " + to_show);
    m_buffer.append(data);

    const QByteArray header = QByteArray::fromHex("DE3A096631");
    const QByteArray footer = QByteArray::fromHex("CEFF");

    while (true) {
        int startIdx = m_buffer.indexOf(header);
        if (startIdx == -1) {
            // 没找到头，清除无用数据
            if (m_buffer.size() > 1024)
                m_buffer.clear();
            break;
        }

        int endIdx = m_buffer.indexOf(footer, startIdx);
        if (endIdx == -1) {
            // 没找到尾，等待更多数据
            break;
        }

        int frameLen = endIdx + footer.size() - startIdx;
        QByteArray frame = m_buffer.mid(startIdx, frameLen);

        // 处理完整帧数据
        emit dataReceived(frame); // 发给 FormPlot

        // 移除已处理数据
        m_buffer.remove(0, endIdx + footer.size());
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
        m_show_send = true;
    } else {
        m_show_send = false;
    }
}

void FormSerial::on_cBoxSendFormat_currentTextChanged(const QString &format)
{
    if (format == VAL_SERIAL_SEND_HEX) {
        m_send_format = SEND_FORMAT::HEX;
    } else if (format == VAL_SERIAL_SEND_HEX_TRANSLATE) {
        m_send_format = SEND_FORMAT::HEX_TRANSLATE;
    } else {
        m_send_format = SEND_FORMAT::NORMAL;
    }
}

void FormSerial::on_checkBoxHexDisplay_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::CheckState::Checked) {
        m_hex_display = true;
    } else {
        m_hex_display = false;
    }
}
