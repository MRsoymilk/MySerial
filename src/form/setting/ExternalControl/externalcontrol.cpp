#include "externalcontrol.h"
#include "funcdef.h"
#include "keydef.h"
#include "ui_externalcontrol.h"

ExternalControl::ExternalControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExternalControl)
{
    ui->setupUi(this);
    init();
}

ExternalControl::~ExternalControl()
{
    delete ui;
}

void ExternalControl::init()
{
    m_iniServer.enable = SETTING_CONFIG_GET(CFG_GROUP_SERVER, CFG_SERVER_ENABLE, VAL_DISABLE);
    m_iniServer.log = SETTING_CONFIG_GET(CFG_GROUP_SERVER, CFG_SERVER_LOG, VAL_DISABLE);
    m_iniServer.port = SETTING_CONFIG_GET(CFG_GROUP_SERVER, CFG_SERVER_PORT, "12345");
    ui->checkBoxEnable->setChecked(m_iniServer.enable == VAL_ENABLE ? true : false);
    ui->checkBoxLog->setChecked(m_iniServer.log == VAL_ENABLE ? true : false);
    ui->lineEditPort->setText(m_iniServer.port);
    if (m_iniServer.enable == VAL_ENABLE) {
        m_server = new Server(m_iniServer.port.toInt());
    }
}

void ExternalControl::on_checkBoxEnable_clicked()
{
    m_iniServer.enable = ui->checkBoxEnable->isChecked() ? VAL_ENABLE : VAL_DISABLE;
    SETTING_CONFIG_SET(CFG_GROUP_SERVER, CFG_SERVER_ENABLE, m_iniServer.enable);
}

void ExternalControl::on_checkBoxLog_clicked()
{
    m_iniServer.log = ui->checkBoxLog->isChecked() ? VAL_ENABLE : VAL_DISABLE;
    SETTING_CONFIG_SET(CFG_GROUP_SERVER, CFG_SERVER_LOG, m_iniServer.log);
}
