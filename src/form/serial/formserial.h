#ifndef FORMSERIAL_H
#define FORMSERIAL_H

#include <QMap>
#include <QPointer>
#include <QSerialPort>
#include <QWidget>

#include "global.h"

class LineSend;
class ThreadParser;
class HandleModeProduce;
class HandleModeEasy;

namespace Ui {
class FormSerial;
}

class FormSerial : public QWidget {
    Q_OBJECT

public:
    enum class SEND_FORMAT { NORMAL = 0, HEX, HEX_TRANSLATE };

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
    void doProduceOpt(int id, const QString &msg = "");
    void doEasyOpt(int id, const QString& msg = "");
    void sendExpertData(const QString &text);
    void updateFrameTypes(const QString &idx);
    QStringList getPorts();

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
    void redoConnect();
    void optReturn(int id, const QString& msg);
    void doOpt(int id, const QString& msg);

public slots:
    void sendRaw(const QByteArray &bytes);
    void onChangeFrameType(const QString &algorithm);
    void onSimulateRecv(const QByteArray &bytes);
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
    QThread *m_connectThread = nullptr;
    HandleModeProduce *m_handleProduce = nullptr;
    HandleModeEasy *m_handleEasy = nullptr;

signals:
    bool stopProduceConnect();
    bool stopEasyConnect();
};

#endif  // FORMSERIAL_H
