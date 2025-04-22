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
        QString baud_rate;
        QString test_port_name;
        QString test_baud_rate;
    };
signals:
    void dataReceived(const QByteArray &data);

public:
    explicit FormSerial(QWidget *parent = nullptr);
    ~FormSerial();

private:
    void setINI();
    void getINI();
private slots:
    void on_btnRecvSave_clicked();
    void on_btnRecvClear_clicked();
    void on_btnSend_clicked();
    void on_btnSerialSwitch_clicked();
    void on_cBoxPortName_activated(int index);
    void onReadyRead();

private:
    Ui::FormSerial *ui;
    void init();
    QMap<QString, SERIAL> m_mapSerial;
    bool m_switch;
    QSerialPort *m_serialPort = nullptr;
    INI_SERIAL m_ini;
    QByteArray m_buffer;
    void openSerial();
    void closeSerial();
};

#endif // FORMSERIAL_H
