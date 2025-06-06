#ifndef MYPROCESS_H
#define MYPROCESS_H

#include <QProcess>

class MyProcess : public QObject
{
    Q_OBJECT

   public:
    ~MyProcess() = default;
    static MyProcess &getInstance();
    void process(const QString &program, const QStringList &arguments);
    void startAttach(const QString &program, const QStringList &arguments);
    bool startDetach(const QString &program, const QStringList &arguments);
    void stopScript(const int &wait_time = 3000);
   private slots:
    void handleStandardOutput();
    void handleStandardError();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

   private:
    MyProcess(QObject *parent = nullptr);
    QProcess *m_process;
};

#define MY_PROCESS MyProcess::getInstance()

#endif  // MYPROCESS_H
