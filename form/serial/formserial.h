#ifndef FORMSERIAL_H
#define FORMSERIAL_H

#include <QMap>
#include <QSerialPort>
#include <QWidget>

namespace Ui {
class FormSerial;
}

class FormSerial : public QWidget
{
    Q_OBJECT

public:
    enum class SEND_FORMAT { NORMAL = 0, HEX, HEX_TRANSLATE };

    struct SERIAL
    {
        QString Description;
        QString Manufacturer;
        QString SerialNumber;
        QString SystemLocation;
        QList<qint32> StandardBaudRates;
    };

    struct INI_SERIAL
    {
        QString port_name;
        QString debug_port;
        QString baud_rate;
        QString send_format;
        bool show_send;
        bool hex_display;
        QString cycle;
        QString send_page;
        QString single_send;
    };
signals:
    void recv2Plot(const QByteArray &data, const QString &name);
    void recv2Data(const QByteArray &data, const QString &name);

public:
    explicit FormSerial(QWidget *parent = nullptr);
    ~FormSerial();

private:
    void getINI();
    void init();
    void openSerial();
    void closeSerial();
    void send(const QString &text);

private slots:
    void on_btnRecvSave_clicked();
    void on_btnRecvClear_clicked();
    void on_btnSend_clicked();
    void on_btnSerialSwitch_clicked();
    void on_cBoxPortName_activated(int index);
    void onReadyRead();
    void on_checkBoxShowSend_checkStateChanged(const Qt::CheckState &state);
    void on_cBoxSendFormat_currentTextChanged(const QString &format);
    void on_checkBoxHexDisplay_checkStateChanged(const Qt::CheckState &arg1);
    void on_checkBoxScheduledDelivery_clicked();
    void onAutoSend();
    void on_lineEditCycle_editingFinished();
    void on_tabWidget_currentChanged(int index);

private:
    Ui::FormSerial *ui;
    QMap<QString, SERIAL> m_mapSerial;
    bool m_switch;
    QSerialPort *m_serial = nullptr;
    INI_SERIAL m_ini;
    QByteArray m_buffer;
    bool m_show_send;
    SEND_FORMAT m_send_format;
    bool m_hex_display;
    QTimer *m_send_timer;
    void initMultSend();
    struct FrameType
    {
        QByteArray header;
        QString name;
    };

    const QList<FrameType> m_frameTypes = {
        {QByteArray::fromHex("DE3A096631"), "curve_24bit"},
        {QByteArray::fromHex("DE3A096633"), "curve_14bit"},
    };

    const QByteArray m_footer = QByteArray::fromHex("CEFF");
    void setINI();
};

#endif // FORMSERIAL_H
