#ifndef HANDLEMODEEASY_H
#define HANDLEMODEEASY_H

#include <QPointer>
#include <QSerialPort>
#include <QTimer>
#include "keydef.h"
#include "global.h"

class HandleModeEasy  : public QObject{
    Q_OBJECT

public:
    enum STEP_EASY_CONNECT {
        EASY_NONE,
        EASY_CONNECT_PORT = 15,
        EASY_HANDSHAKE = 30,
        EASY_MODE_DOUBLE_DO_THRESHOLD = 45,
        EASY_MODE_DOUBLE_DO_BASELINE = 60,
        EASY_SET_INTEGRATION_TIME = 75,
        EASY_DATA_REQUEST = 90,
        EASY_FINISH = 100
    };

signals:
    void connectEstablished();
    void dataReady(const QByteArray &data);
    void redoConnect();
    void sendThreshold(bool isUse, const QList<double> &values);
    void sendOption(const QJsonObject &option);
    void statusReport(int progress, const QString &msg);

public slots:
    void stopConnect();

public:
    HandleModeEasy(QObject *parent = nullptr);
    void setFrameType(QList<FrameType> type);
    void doConnect(const QStringList &ports, const QString& mode);

private:
    void init();
    void processEasyConnect(const QByteArray &data);
    bool doEasyFrameExtra();
    void onEasyModeTimeout();
    void onEasyModeReadyRead();
    void doEasyConnect();
    void processEasyRetry();
    QTimer *m_timer_easy = nullptr;
    STEP_EASY_CONNECT m_step = EASY_CONNECT_PORT;
    QString m_easy_mode = CFG_F30_MODE_DOUBLE;
    QByteArray m_easy_buffer;

    QList<FrameType> m_frameTypes = {};
    bool m_establish = false;
    QStringList m_ports;
    int m_port_index = 0;
    void tryNextPort();
    QPointer<QSerialPort> m_serial;
    QElapsedTimer m_timer_elapsed;
    bool m_wait_next_cmd = false;
    QString m_mode;
    bool m_wait_next_port = false;
    void sendCMD(const QString &text);
    bool doThresholdExtra(const QByteArray &data);
    bool doBaselineExtra(const QByteArray &data);
};

#endif  // HANDLEMODEEASY_H
