#include "processthread.h"

ProcessThread::ProcessThread(const QString &program, const QStringList &arguments, QObject *parent)
    : QThread(parent), m_program(program), m_arguments(arguments)
{
}

void ProcessThread::run()
{
    QProcess process;
    connect(&process, &QProcess::readyReadStandardOutput,
            [&]() { emit outputReceived(process.readAllStandardOutput()); });
    connect(&process, &QProcess::readyReadStandardError, [&]() { emit errorReceived(process.readAllStandardError()); });
    process.start(m_program, m_arguments);
    if (!process.waitForFinished(-1))
    {
        emit errorReceived(QString("Process failed: %1").arg(process.errorString()));
    }
    else
    {
        emit processFinished(process.exitCode(), process.exitStatus());
    }
}
