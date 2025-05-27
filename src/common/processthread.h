#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include <QProcess>
#include <QThread>

class ProcessThread : public QThread
{
    Q_OBJECT

   public:
    explicit ProcessThread(const QString &program, const QStringList &arguments, QObject *parent = nullptr);

   signals:
    void outputReceived(const QString &output);
    void errorReceived(const QString &error);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

   protected:
    void run() override;

   private:
    QString m_program;
    QStringList m_arguments;
};

#endif  // PROCESSTHREAD_H
