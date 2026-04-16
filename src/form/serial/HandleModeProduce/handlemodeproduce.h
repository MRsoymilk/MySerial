#ifndef HANDLEMODEPRODUCE_H
#define HANDLEMODEPRODUCE_H

#include <QByteArray>
#include <QPointer>
#include <QSerialPort>
#include <QTimer>

#include "global.h"

class HandleModeProduce : public QObject {
    Q_OBJECT

public:
    enum STEP_PRODUCE_CONNECT {
        PRODUCE_NONE = 25,
        PRODUCE_HANDSHAKE = 50,
        PRODUCE_DATA_REQUEST = 75,
        PRODUCE_FINISH = 100
    };

signals:
    void connectEstablished();
    void dataReady(const QByteArray &data);
    void redoConnect();
    void statusReport(int progress, const QString &msg);
    void optReturn(int id, const QString &msg);

public slots:
    void stopConnect();
    void doOpt(int id, const QString &msg);
    void doConnect(const QStringList &ports);
    void setFrameType(QList<FrameType> type);

public:
    explicit HandleModeProduce(QObject *parent = nullptr);

private slots:
    void onProduceModeReadyRead();

private:
    bool doProduceFrameExtra();
    void onProduceModeTimeout();
    void processProduceConnect(const QByteArray &data);
    void sendCMD(const QString &text);
    void processProduceCall(const QByteArray &data);
    void init();
    void tryNextPort();

private:
    std::function<bool(const QByteArray &)> m_call_produce_func = nullptr;
    QTimer *m_timer_produce = nullptr;
    bool m_wait_next_port = false;
    bool m_wait_next_cmd = false;
    STEP_PRODUCE_CONNECT m_step = PRODUCE_NONE;
    QByteArray m_produce_buffer;
    QPointer<QSerialPort> m_serial;

    QElapsedTimer m_timer_elapsed;
    QList<FrameType> m_frameTypes = {};
    bool m_establish = false;
    int m_port_index = 0;
    QStringList m_ports;
    bool m_wait_call = false;
    int m_call_step = 0;
    QByteArray m_call_buffer;
};

#endif  // HANDLEMODEPRODUCE_H
