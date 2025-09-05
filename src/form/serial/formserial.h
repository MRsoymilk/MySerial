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

    struct FRAME
    {
        QByteArray bit14;
        QByteArray bit24;
    };

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

public:
    explicit FormSerial(QWidget *parent = nullptr);
    ~FormSerial();
    void retranslateUI();

signals:
    void recv2Plot4k(const QByteArray &data14, const QByteArray &data24);
    void recv2Data4k(const QByteArray &data14,
                     const QByteArray &data24,
                     const QByteArray &temperature = "");
    void recv2MPU(const QByteArray &data);
    void recvTemperature(double temperature);

private:
    void getINI();
    void init();
    void openSerial();
    void closeSerial();
    void send(const QString &text);
    void initMultSend();
    void setINI();
    void handleFrame(const QString &frameName,
                     const QByteArray &frame_candidate,
                     const QByteArray &temp = "");

public slots:
    void sendRaw(const QByteArray &bytes);
    void onChangeFrameType(int index);

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
    void on_tBtnRefresh_clicked();

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
    struct FrameType
    {
        QString name;
        QByteArray header;
        QByteArray footer;
        int length;
    };

    QList<FrameType> m_frameTypes = {};
    int m_algorithm;
    FormSerial::FRAME m_frame;
    long long m_recv_count;
    QStringList m_lastPortList;

    bool m_toPeek = false;
    bool m_waitting_byte = false;
};

#endif // FORMSERIAL_H
