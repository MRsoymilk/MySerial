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
    SETTING_SYNC();
    delete ui;
}

void FormSerial::getINI()
{
    m_ini.port_name = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT_NAME);
    m_ini.baud_rate = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_BAUD_RATE);
    m_ini.send_format = SETTING_GET(CFG_GROUP_SERIAL,
                                    CFG_SERIAL_SEND_FORMAT,
                                    VAL_SERIAL_SEND_NORMAL);
    m_ini.show_send = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_SHOW_SEND, VAL_ENABLE) == VAL_ENABLE
                          ? true
                          : false;
    m_ini.hex_display = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_HEX_DISPLAY, VAL_DISABLE)
                                == VAL_ENABLE
                            ? true
                            : false;
    QString cycle = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_CYCLE, "1000");
    QString send_page = SETTING_GET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_PAGE, VAL_PAGE_SINGLE);

    ui->cBoxPortName->setCurrentText(m_ini.port_name);
    ui->cBoxBaudRate->setCurrentText(m_ini.baud_rate);
    on_cBoxSendFormat_currentTextChanged(m_ini.send_format);
    ui->checkBoxShowSend->setChecked(m_ini.show_send);
    ui->checkBoxHexDisplay->setChecked(m_ini.hex_display);
    ui->lineEditCycle->setText(cycle);
    if (send_page == VAL_PAGE_SINGLE) {
        ui->tabWidget->setCurrentWidget(ui->tabSingle);
    } else {
        ui->tabWidget->setCurrentWidget(ui->tabMultipe);
    }
    QString single_send = SETTING_GET(CFG_GROUP_HISTROY, CFG_HISTORY_SINGLE_SEND);
    QString mult_0 = SETTING_GET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_0);
    QString mult_1 = SETTING_GET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_1);
    QString mult_2 = SETTING_GET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_2);
    QString mult_3 = SETTING_GET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_3);
    QString mult_4 = SETTING_GET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_4);
    ui->txtSend->setPlainText(single_send);
    ui->lineEdit_cmd_0->setText(mult_0);
    ui->lineEdit_cmd_1->setText(mult_1);
    ui->lineEdit_cmd_2->setText(mult_2);
    ui->lineEdit_cmd_3->setText(mult_3);
    ui->lineEdit_cmd_4->setText(mult_4);
}

void FormSerial::init()
{
    // init port
    QList<QSerialPortInfo> list_port = QSerialPortInfo::availablePorts();
    if (list_port.isEmpty()) {
        qDebug() << "No available serial port found!";
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
    m_switch = false;
    ui->btnSerialSwitch->setText("To Open");
#ifdef QT_DEBUG
    ui->cBoxPortName->addItem("/dev/pts/9");
#endif
    ui->cBoxPortName->addItems(port_names);
    on_cBoxPortName_activated(0);

    ui->cBoxDataBit->addItems({"8", "7", "6", "5"});
    ui->cBoxCheckBit->addItems({"None", "Even", "Mark", "Odd"});
    ui->cBoxStopBit->addItems({"1", "1.5", "2"});
    // init send
    ui->btnSend->setIcon(QIcon(":/res/icons/send-on.png"));
    ui->btnSend->setIconSize(QSize(32, 32));

    // TODO
    ui->groupBoxEnhancement->hide();

    ui->cBoxSendFormat->addItems(
        {VAL_SERIAL_SEND_NORMAL, VAL_SERIAL_SEND_HEX, VAL_SERIAL_SEND_HEX_TRANSLATE});

    // ini
    getINI();
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [=]() {
        if (m_serial->isOpen()) {
            m_serial->close();
        }
    });
    m_send_timer = new QTimer(this);
    connect(m_send_timer, &QTimer::timeout, this, &FormSerial::onAutoSend);
}

void FormSerial::send(const QString &text)
{
    LOG_INFO("serial send: {}", text);
    if (!(m_serial && m_serial->isOpen())) {
        qDebug() << "Serial port not open.";
        SHOW_AUTO_CLOSE_MSGBOX(this, "warning", "serial not open!");
        return;
    }

    if (text.isEmpty()) {
        qDebug() << "Send text is empty.";
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
                qDebug() << "illegal hex: " << byteStr;
            }
        }
        qDebug() << "send (hex):" << data;
        to_show = data;
    } else if (m_ini.send_format == VAL_SERIAL_SEND_HEX_TRANSLATE) {
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
    m_serial->write(data);
    if (m_ini.show_send) {
        ui->txtRecv->appendPlainText("[TX] " + to_show);
    }
}

void FormSerial::on_btnSend_clicked()
{
    QString text = ui->txtSend->toPlainText().trimmed();
    LOG_INFO("serial send: {}", text);
    send(text);
    SETTING_SET(CFG_GROUP_HISTROY, CFG_HISTORY_SINGLE_SEND, text);
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
    m_serial->setBaudRate(ui->cBoxBaudRate->currentText().toInt());
    m_serial->setDataBits(
        static_cast<QSerialPort::DataBits>(ui->cBoxDataBit->currentText().toInt()));

    QString check = ui->cBoxCheckBit->currentText();
    if (check == "None")
        m_serial->setParity(QSerialPort::NoParity);
    else if (check == "Even")
        m_serial->setParity(QSerialPort::EvenParity);
    else if (check == "Odd")
        m_serial->setParity(QSerialPort::OddParity);
    else if (check == "Mark")
        m_serial->setParity(QSerialPort::MarkParity);

    QString stopBitText = ui->cBoxStopBit->currentText();
    if (stopBitText == "1")
        m_serial->setStopBits(QSerialPort::OneStop);
    else if (stopBitText == "1.5")
        m_serial->setStopBits(QSerialPort::OneAndHalfStop);
    else if (stopBitText == "2")
        m_serial->setStopBits(QSerialPort::TwoStop);

    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open serial port!";
        qDebug() << m_serial->errorString();
        LOG_WARN("Failed to open serial port: {}", m_serial->errorString());
        delete m_serial;
        m_serial = nullptr;
        m_switch = false;
        return;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &FormSerial::onReadyRead);

    ui->btnSerialSwitch->setText("To Close");
    ui->btnSerialSwitch->setStyleSheet("background-color: green; color: white;");

    SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT_NAME, ui->cBoxPortName->currentText());
    SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_BAUD_RATE, ui->cBoxBaudRate->currentText());
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

void FormSerial::onReadyRead()
{
    QByteArray data = m_serial->readAll();
    LOG_INFO("serial recv: {}", data);
    QString to_show = data;
    if (m_ini.hex_display) {
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
        emit recv2Data(frame);
        emit recv2Plot(frame); // 发给 FormPlot

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
        m_ini.show_send = true;
        SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SHOW_SEND, VAL_ENABLE);
    } else {
        m_ini.show_send = false;
        SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SHOW_SEND, VAL_DISABLE);
    }
}

void FormSerial::on_cBoxSendFormat_currentTextChanged(const QString &format)
{
    if (!m_ini.send_format.isEmpty()) {
        SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_FORMAT, format);
    }
    m_ini.send_format = format;
    ui->cBoxSendFormat->setCurrentText(format);
}

void FormSerial::on_checkBoxHexDisplay_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::CheckState::Checked) {
        m_ini.hex_display = true;
        SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_HEX_DISPLAY, VAL_ENABLE);
    } else {
        m_ini.hex_display = false;
        SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_HEX_DISPLAY, VAL_DISABLE);
    }
}

void FormSerial::on_tBtn_0_clicked()
{
    QString text = ui->lineEdit_cmd_0->text().trimmed();
    send(text);
    SETTING_SET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_0, text);
}

void FormSerial::on_tBtn_1_clicked()
{
    QString text = ui->lineEdit_cmd_1->text().trimmed();
    send(text);
    SETTING_SET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_1, text);
}

void FormSerial::on_tBtn_2_clicked()
{
    QString text = ui->lineEdit_cmd_2->text().trimmed();
    send(text);
    SETTING_SET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_2, text);
}

void FormSerial::on_tBtn_3_clicked()
{
    QString text = ui->lineEdit_cmd_3->text().trimmed();
    send(text);
    SETTING_SET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_3, text);
}

void FormSerial::on_tBtn_4_clicked()
{
    QString text = ui->lineEdit_cmd_4->text().trimmed();
    send(text);
    SETTING_SET(CFG_GROUP_HISTROY, CFG_HISTORY_MULT_4, text);
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
    SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_CYCLE, ui->lineEditCycle->text());
}

void FormSerial::on_tabWidget_currentChanged(int index)
{
    if (index == 0) {
        SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_PAGE, VAL_PAGE_SINGLE);
    } else {
        SETTING_SET(CFG_GROUP_SERIAL, CFG_SERIAL_SEND_PAGE, VAL_PAGE_MULTIPE);
    }
}
