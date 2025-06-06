#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Server : public QObject
{
    Q_OBJECT
public:
    Server(const int &port, QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_clientSocket;
    bool m_bLog;
};

#endif // SERVER_H
