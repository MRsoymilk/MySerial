#ifndef FORMSERIAL_H
#define FORMSERIAL_H

#include <QMap>
#include <QPointer>
#include <QSerialPort>
#include <QWidget>

#include "global.h"
#include "keydef.h"

class LineSend;
class ThreadParser;

namespace Ui {
class FormSerial;
}

class FormSerial : public QWidget {
    Q_OBJECT

public:
    enum class SEND_FORMAT { NORMAL = 0, HEX, HEX_TRANSLATE };
    enum STEP_PRODUCE_CONNECT { PRODUCE_CONNECT_PORT = 1, PRODUCE_HANDSHAKE, PRODUCE_DATA_REQUEST, PRODUCE_FINISH };
    enum STEP_EASY_CONNECT {
        EASY_CONNECT_PORT = 1,
        EASY_HANDSHAKE,
        EASY_MODE_DOUBLE_DO_THRESHOLD,
        EASY_MODE_DOUBLE_DO_BASELINE,
        EASY_SET_INTEGRATION_TIME,
        EASY_DATA_REQUEST,
        EASY_FINISH
    };

    struct SERIAL {
        QString Description;
        QString Manufacturer;
        QString SerialNumber;
        QString SystemLocation;
        QList<qint32> StandardBaudRates;
    };

    struct INI_SERIAL {
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
    bool startEasyConnect(const QString &F30_shown_mode);
    bool startProduceConnect();
    void stopFSeriesConnect();
    void sendEasyData(const QString &text, int timeout = 1000);
    void sendExpertData(const QString &text);
    void updateFrameTypes(const QString &idx);
    void sendProduceData(const QString &text, std::function<bool(const QByteArray &)> func = nullptr);

signals:
    void recv2PlotLLC(const FRAME &frame, const double &temperature = 0.0);
    void recv2PlotF30(const FRAME &frame, const double &temperature = 0.0);
    void recv2PlotF15(const FRAME &frame, const double &temperature = 0.0);
    void recv2MPU(const QByteArray &data);
    void statusReport(int progress, const QString &msg);
    void connectProduceModeEstablished();
    void connectEasyModeEstablished();
    void pushParserData(QByteArray data);
    void sendThreshold(bool isUse, const QList<double> &values);
    void sendOption(const QJsonObject &option);

public slots:
    void sendRaw(const QByteArray &bytes);
    void onChangeFrameType(const QString &algorithm);
    void onSimulateRecv(const QByteArray &bytes);
    void clearData();
    void handleFrame(const QString &frameName, const QByteArray &frame_candidate, const QByteArray &temp = "");

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void getINI();
    void init();
    void openSerial();
    void closeSerial();
    void initMultSend();
    void setINI();
    void loadPage(int page);
    void flushUiBuffer();
    bool doThresholdExtra(const QByteArray &data);
    bool doBaselineExtra(const QByteArray &data);

private slots:
    void on_btnRecvSave_clicked();
    void on_btnRecvClear_clicked();
    void on_btnSend_clicked();
    void on_btnSerialSwitch_clicked();
    void on_cBoxPortName_activated(int index);
    void onProduceModeReadyRead();
    void on_checkBoxShowSend_checkStateChanged(const Qt::CheckState &state);
    void on_cBoxSendFormat_currentTextChanged(const QString &format);
    void on_checkBoxHexDisplay_checkStateChanged(const Qt::CheckState &state);
    void on_checkBoxScheduledDelivery_clicked();
    void onAutoSend();
    void on_lineEditCycle_editingFinished();
    void on_tabWidget_currentChanged(int index);
    void on_tBtnRefresh_clicked();
    void on_tBtnNext_clicked();
    void on_tBtnPrev_clicked();
    void on_lineEditPageName_editingFinished();
    void on_checkBoxAcceptTemperature_clicked();
    void onExpertModeReadyRead();

private:
    Ui::FormSerial *ui;
    QMap<QString, SERIAL> m_mapSerial;
    bool m_switch;
    QPointer<QSerialPort> m_serial;
    INI_SERIAL m_ini;
    bool m_show_send;
    SEND_FORMAT m_send_format;
    bool m_hex_display;
    QTimer *m_send_timer;

    QList<FrameType> m_frameTypes = {};
    QString m_algorithm;
    FRAME m_frame;
    long long m_recv_count;

    QVector<LineSend *> m_lineSends;
    int m_pageSize = 5;
    int m_currentPage = 0;
    bool m_acceptTemperature = false;
    QThread *m_workerThread = nullptr;
    ThreadParser *m_parser = nullptr;

private:
    void doPortsScan();
    int m_port_index = 0;
    QStringList m_ports;
    bool m_establish = false;

private:
    void doProduceConnect();
    void onProduceModeTimeout();
    void processProduceConnect(const QByteArray &frame);
    std::function<bool(const QByteArray &)> m_call_produce_func = nullptr;
    QTimer *m_timer_produce = nullptr;
    STEP_PRODUCE_CONNECT m_step_produce = PRODUCE_CONNECT_PORT;

private:
    bool doFrameExtra(const QByteArray &data);
    void onEasyModeTimeout();
    void onEasyModeReadyRead();
    void doEasyConnect();
    void processEasyConnect();
    void processEasyRetry();
    QTimer *m_timer_easy = nullptr;
    bool m_easy_wait = false;
    STEP_EASY_CONNECT m_step_easy = EASY_CONNECT_PORT;
    QString m_easy_mode = CFG_F30_MODE_DOUBLE;
    QByteArray m_easy_buffer;
};

#endif  // FORMSERIAL_H
