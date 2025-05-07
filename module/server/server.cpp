#include "server.h"
#include <QProcess>
#include <QRegularExpression>
#include "funcdef.h"
Server::Server(const int &port, QObject *parent)
    : QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);
    m_tcpServer->listen(QHostAddress::Any, port);
    LOG_INFO("Server start: {}", port);
    m_bLog = SETTING_GET(CFG_GROUP_SERVER, CFG_SERVER_LOG, VAL_DISABLE) == VAL_ENABLE ? true
                                                                                      : false;
}

void Server::onNewConnection()
{
    m_clientSocket = m_tcpServer->nextPendingConnection();
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
}

QStringList parseCommand(const QString &commandLine)
{
    QRegularExpression regex(R"((?:"([^"]+)\"|'([^']+)'|(\S+)))");
    QRegularExpressionMatchIterator i = regex.globalMatch(commandLine);

    QStringList args;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.captured(1).length())
            args << match.captured(1); // 双引号
        else if (match.captured(2).length())
            args << match.captured(2); // 单引号
        else
            args << match.captured(3); // 普通参数
    }
    return args;
}

void Server::onReadyRead()
{
    QByteArray data = m_clientSocket->readAll();
    QString commandLine = QString::fromUtf8(data).trimmed();

    QStringList args = parseCommand(commandLine);
    if (args.isEmpty()) {
        QString err = "Invalid command\n";
        m_clientSocket->write(err.toUtf8());
        m_clientSocket->flush();
        return;
    }

    QString program = args.takeFirst();

    QProcess process;
    process.start(program, args);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    QString result = output + error;

    if (m_bLog) {
        LOG_INFO("command: {}", commandLine);
        LOG_INFO("result: {}", result);
    }

    m_clientSocket->write(result.toUtf8());
    m_clientSocket->flush();
}
