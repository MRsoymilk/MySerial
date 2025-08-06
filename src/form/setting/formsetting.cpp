#include "formsetting.h"
#include "funcdef.h"
#include "ui_formsetting.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOperatingSystemVersion>
#include <QProcess>
#include "AutoUpdate/autoupdate.h"
#include "Calculate/calculate.h"
#include "ExternalControl/externalcontrol.h"

FormSetting::FormSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormSetting)
{
    ui->setupUi(this);
    init();
}

FormSetting::~FormSetting()
{
    SETTING_CONFIG_SYNC();
    delete ui;
}

void FormSetting::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormSetting::init()
{
    m_autoUpdate = new AutoUpdate;
    m_calculate = new Calculate;
    m_externalControl = new ExternalControl;
    ui->vLay->addWidget(m_autoUpdate);
    ui->vLay->addWidget(m_calculate);
    ui->vLay->addWidget(m_externalControl);
}
