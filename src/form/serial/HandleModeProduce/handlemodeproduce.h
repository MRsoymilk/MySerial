#ifndef HANDLEMODEPRODUCE_H
#define HANDLEMODEPRODUCE_H

#include <QByteArray>
#include <QTimer>
#include <QSerialPort>
#include <QPointer>
#include "global.h"

class HandleModeProduce : public QObject{
    Q_OBJECT

public:
    enum STEP_PRODUCE_CONNECT {
        PRODUCE_NONE,
        PRODUCE_HANDSHAKE,
        PRODUCE_DATA_REQUEST,
        PRODUCE_FINISH
    };
signals:
    void connectEstablished();
    void dataReady(const QByteArray &data);
    void redoConnect();

public slots:
    void stopConnect();

public:
    explicit HandleModeProduce(QObject *parent = nullptr);
    void doConnect(const QStringList& ports);
    void setFrameType(QList<FrameType> type);

private slots:
    void onProduceModeReadyRead();

private:
    void tryResetHardware();
    bool doProduceFrameExtra();
    void onProduceModeTimeout();
    void processProduceConnect(const QByteArray &data);
    void processProduceRetry();
private:
    std::function<bool(const QByteArray &)> m_call_produce_func = nullptr;
    QTimer *m_timer_produce = nullptr;
    bool m_wait_next_port = false;
    bool m_wait_next_cmd = false;
    STEP_PRODUCE_CONNECT m_step = PRODUCE_NONE;
    QByteArray m_produce_buffer;
    QPointer<QSerialPort> m_serial;

    QElapsedTimer m_timer_elapsed;
    void sendCMD(const QString &text);
    QList<FrameType> m_frameTypes = {};
    bool m_establish = false;
    void init();
    int m_port_index = 0;
    QStringList m_ports;
    void tryNextPort();
};

#endif  // HANDLEMODEPRODUCE_H
