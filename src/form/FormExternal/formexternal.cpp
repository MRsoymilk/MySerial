#include "formexternal.h"
#include "funcdef.h"
#include "keydef.h"
#include "ui_formexternal.h"

#include <QJsonDocument>

FormExternal::FormExternal(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExternalControl)
{
    ui->setupUi(this);
    init();
}

FormExternal::~FormExternal()
{
    stopServer();
    delete ui;
}

void FormExternal::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormExternal::init()
{
    m_iniServer.enable = SETTING_CONFIG_GET(CFG_GROUP_SERVER, CFG_SERVER_ENABLE, VAL_DISABLE);
    m_iniServer.log = SETTING_CONFIG_GET(CFG_GROUP_SERVER, CFG_SERVER_LOG, VAL_DISABLE);
    m_iniServer.port = SETTING_CONFIG_GET(CFG_GROUP_SERVER, CFG_SERVER_PORT, "12345");

    ui->checkBoxEnable->setChecked(m_iniServer.enable == VAL_ENABLE);
    ui->checkBoxLog->setChecked(m_iniServer.log == VAL_ENABLE);
    ui->lineEditPort->setText(m_iniServer.port);

    if (m_iniServer.enable == VAL_ENABLE) {
        startServer();
    }
}

void FormExternal::onExternalSpectral(const QJsonObject &spectral)
{
    QMutexLocker locker(&m_mutex);
    m_objSpectrum = spectral;
}

void FormExternal::startServer()
{
    if (!m_server) {
        m_server = new httplib::Server();
        m_server->new_task_queue = [] { return new httplib::ThreadPool(4); };
        addRouter();
    }

    if (m_running.load())
        return;

    m_running.store(true);

    int port = m_iniServer.port.toInt();
    LOG_INFO("start http server on port {}", port);

    m_listenThread = QThread::create([this, port]() {
        m_server->listen("0.0.0.0", port);
        LOG_INFO("http listen thread exited");
    });

    connect(m_listenThread,
            &QThread::finished,
            m_listenThread,
            &QObject::deleteLater,
            Qt::QueuedConnection);

    m_listenThread->start();
}

void FormExternal::stopServer()
{
    if (!m_server || !m_running.load())
        return;

    LOG_INFO("stop http server");

    m_running.store(false);

    m_server->stop();

    if (m_listenThread) {
        m_listenThread->wait();
        m_listenThread = nullptr;
    }
}

void FormExternal::addRouter()
{
    // ===== /info =====
    m_server->Get("/info", [this](const httplib::Request &, httplib::Response &res) {
        if (!m_running.load()) {
            res.status = 503;
            return;
        }
        QJsonObject objInfo;
        objInfo["status"] = "ok";
        objInfo["interface_name"] = "F30";
        objInfo["timestamp"] = TIMESTAMP();
        res.set_content(TO_STR(objInfo).toStdString(), "application/json");
    });

    // ===== /spectrum =====
    m_server->Get("/spectrum", [this](const httplib::Request &, httplib::Response &res) {
        if (!m_running.load()) {
            res.status = 503;
            return;
        }

        QMutexLocker locker(&m_mutex);
        res.set_content(QJsonDocument(m_objSpectrum).toJson(QJsonDocument::Compact).toStdString(),
                        "application/json");
        m_objSpectrum = {};
    });
}

void FormExternal::on_checkBoxEnable_clicked()
{
    bool enable = ui->checkBoxEnable->isChecked();
    m_iniServer.enable = enable ? VAL_ENABLE : VAL_DISABLE;
    SETTING_CONFIG_SET(CFG_GROUP_SERVER, CFG_SERVER_ENABLE, m_iniServer.enable);

    if (enable) {
        startServer();
    } else {
        stopServer();
    }

}

void FormExternal::on_checkBoxLog_clicked()
{
    m_iniServer.log = ui->checkBoxLog->isChecked() ? VAL_ENABLE : VAL_DISABLE;
    SETTING_CONFIG_SET(CFG_GROUP_SERVER, CFG_SERVER_LOG, m_iniServer.log);
}
