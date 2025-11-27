#include "formsetting.h"
#include "funcdef.h"
#include "ui_formsetting.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOperatingSystemVersion>
#include <QProcess>
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

    if (m_externalControl) {
        m_externalControl->retranslateUI();
    }
}

void FormSetting::init()
{
    m_externalControl = new ExternalControl;
    ui->vLay->addWidget(m_externalControl);
}
