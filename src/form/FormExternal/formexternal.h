#pragma once

#include <QJsonObject>
#include <QMutex>
#include <QThread>
#include <QWidget>
#include <atomic>

#include "httplib.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ExternalControl;
}
QT_END_NAMESPACE

class FormExternal : public QWidget
{
    Q_OBJECT
public:
    explicit FormExternal(QWidget *parent = nullptr);
    ~FormExternal();
    void retranslateUI();

signals:
    void toExternalControl(bool enable);

public slots:
    void onExternalSpectral(const QJsonObject &spectral);

private slots:
    void on_checkBoxEnable_clicked();
    void on_checkBoxLog_clicked();

private:
    void init();
    void startServer();
    void stopServer();
    void addRouter();

private:
    Ui::ExternalControl *ui;

    httplib::Server *m_server = nullptr;
    QThread *m_listenThread = nullptr;

    // ===== 数据 =====
    QJsonObject m_objSpectrum;
    QMutex m_mutex;

    // ===== 运行状态 =====
    std::atomic_bool m_running{false};

    struct
    {
        QString enable;
        QString log;
        QString port;
    } m_iniServer;
};
