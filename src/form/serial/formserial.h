#ifndef FORMSERIAL_H
#define FORMSERIAL_H

#include <QMap>
#include <QSerialPort>
#include <QWidget>

class LineSend;

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
        QByteArray bit31;
        QByteArray bit33;
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
    bool startEasyConnect();
    void stopEasyConnect();
    void writeEasyData(const QString &value);
    void setEasyFrame();

signals:
    void recv2PlotLLC(const QByteArray &data31,
                      const QByteArray &data33,
                      const double &temperature = 0.0);
    void recv2PlotF30(const QByteArray &data31,
                      const QByteArray &data33,
                      const double &temperature = 0.0);
    void recv2DataF30(const QByteArray &data31,
                      const QByteArray &data33,
                      const QByteArray &temperature = "");
    void recv2PlotF15(const QByteArray &data31,
                      const QByteArray &data33,
                      const double &temperature = 0.0);
    void recv2DataF15(const QByteArray &data31,
                      const QByteArray &data33,
                      const QByteArray &temperature = "");
    void recv2MPU(const QByteArray &data);
    void recvTemperature(double temperature);

public slots:
    void sendRaw(const QByteArray &bytes);
    void onChangeFrameType(int index);
    void onSimulateRecv(const QByteArray &bytes);

protected:
    void closeEvent(QCloseEvent *event) override;

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
    void loadPage(int page);
    void updateFrameTypes(int idx);
    void doFrameExtra(const QByteArray &data);

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
    void on_tBtnNext_clicked();
    void on_tBtnPrev_clicked();
    void on_lineEditPageName_editingFinished();
    void on_checkBoxAcceptTemperature_clicked();

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

    // bool m_toPeek = false;
    // bool m_waitting_byte = false;
    // bool m_need_after = false;

    QVector<LineSend *> m_lineSends;
    int m_pageSize = 5;
    int m_currentPage = 0;
    bool m_acceptTemperature = false;
};

#endif // FORMSERIAL_H
