#include "formsetting.h"
#include "funcdef.h"
#include "ui_formsetting.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOperatingSystemVersion>
#include <QProcess>
#include "formtip.h"
#include "server.h"

FormSetting::FormSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormSetting)
{
    ui->setupUi(this);
    init();
}

FormSetting::~FormSetting()
{
    if (m_tip) {
        delete m_tip;
    }
    if (m_server) {
        delete m_server;
    }
    SETTING_SYNC();
    delete ui;
}

void FormSetting::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormSetting::init()
{
    getINI();
    if (m_iniUpdate.tip == VAL_ENABLE) {
        m_tip = new FormTip;
        connect(this, &FormSetting::showUpdates, m_tip, &FormTip::onFetchUpdates);
        emit showUpdates(m_iniUpdate.url);
        m_tip->show();
        SETTING_SET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_TIP, VAL_DISABLE);
    }
    if (m_iniServer.enable == VAL_ENABLE) {
        m_server = new Server(m_iniServer.port.toInt());
    }
}

void FormSetting::on_btnCheck_clicked()
{
    LOG_INFO("start check update");
    onAutoUpdate();
}

void FormSetting::on_checkBoxCheckUpdates_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::CheckState::Checked) {
        m_iniUpdate.check = VAL_ENABLE;
    } else {
        m_iniUpdate.check = VAL_DISABLE;
    }
    SETTING_SET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_CHECK, m_iniUpdate.check);
}

void FormSetting::on_lineEditURL_editingFinished()
{
    m_iniUpdate.url = ui->lineEditURL->text();
    SETTING_SET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_URL, m_iniUpdate.url);
}

void FormSetting::scriptAndUpdate(QStringList items)
{
    QString appPath = QCoreApplication::applicationFilePath();
    QString updaterPath;
    QString scriptContent;
    QString tempDir = QDir::tempPath();

    for (const auto &item : items) {
        QString tempFile = tempDir + "/" + item;

        if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows) {
            updaterPath = tempDir + "/updater.bat";

            scriptContent += "@echo off\n";
            scriptContent += "timeout /t 2 >nul\n";
            scriptContent += "move /Y \"" + QDir::toNativeSeparators(tempFile) + "\" \""
                             + QDir::toNativeSeparators(appPath) + "\"\n";
            scriptContent += "start \"\" \"" + QDir::toNativeSeparators(appPath) + "\"\n";
            scriptContent += "del \"%~f0\"\n";
        } else {
            updaterPath = tempDir + "/updater.sh";

            scriptContent += "#!/bin/bash\n";
            scriptContent += "sleep 2\n";
            scriptContent += "mv -f \"" + tempFile + "\" \"" + appPath + "\"\n";
            scriptContent += "chmod +x \"" + appPath + "\"\n";
            scriptContent += "\"" + appPath + "\" &\n";
            scriptContent += "rm -- \"$0\"\n";
        }
    }

    QFile scriptFile(updaterPath);
    if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        scriptFile.write(scriptContent.toUtf8());
        scriptFile.close();

        LOG_INFO("<update> Script: {}", scriptContent.toUtf8());

        QFile::setPermissions(updaterPath, QFile::ExeUser | QFile::ReadUser | QFile::WriteUser);

        LOG_INFO("<update> Updater ready: {}", updaterPath);

        if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows) {
            QProcess::startDetached("cmd.exe", QStringList() << "/c" << updaterPath);
        } else {
            QProcess::startDetached("/bin/bash", QStringList() << updaterPath);
        }

        QTimer::singleShot(500, []() { qApp->quit(); });
    }
}

void FormSetting::onAutoUpdate()
{
    QStringList items;
    if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows) {
        items = {
            "MySerial.exe",
        };
    } else {
        items = {
            "MySerial",
        };
    }

    for (const auto &item : items) {
        if (!checkAndDownload(item)) {
            return;
        }
    }

    scriptAndUpdate(items);
}

bool FormSetting::checkAndDownload(const QString &filename)
{
    QString url = m_iniUpdate.url + "/latest" + "/" + filename;
    QString tempDir = QDir::tempPath();
    QString tempFilePath = tempDir + "/" + filename;
    QString localFilePath = QCoreApplication::applicationDirPath() + "/" + filename;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request((QUrl(url)));

    QNetworkReply *reply = manager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        LOG_WARN("<update> Update Failed: Can not reach: {}", url);
        QMessageBox::warning(this, "Update Failed!", QString("Can not reach %1!").arg(url));
        reply->deleteLater();
        return false;
    }

    QByteArray remoteData = reply->readAll();
    reply->deleteLater();

    QFile localFile(localFilePath);
    bool needUpdate = false;

    if (!localFile.exists()) {
        needUpdate = true;
    } else {
        QFile file(localFilePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray localData = file.readAll();
            if (localData != remoteData) {
                needUpdate = true;
            }
            file.close();
        } else {
            needUpdate = true;
        }
    }

    if (needUpdate) {
        QFile tempFile(tempFilePath);
        if (tempFile.open(QIODevice::WriteOnly)) {
            tempFile.write(remoteData);
            tempFile.close();
            LOG_INFO("<update> Download Complted, the new version has been downloaded to a "
                     "temporary directory: {}",
                     tempFilePath);
            QMessageBox::StandardButton toUpdate
                = QMessageBox::information(this,
                                           "Download Completed!",
                                           "Do you want to update?",
                                           QMessageBox::Yes | QMessageBox::No);
            if (toUpdate == QMessageBox::Yes) {
                LOG_INFO("<update> Use Choose Update");
                SETTING_SET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_TIP, VAL_ENABLE);
                return true;
            } else {
                LOG_INFO("<update> Use Choose Not Update");
                return false;
            }

        } else {
            LOG_WARN("<update> Update Failed: Unable to save to a temporary file: {}", tempFilePath);
            QMessageBox::warning(this, "Update Failed!", "Unable to save to temporary file!");
        }
    } else {
        LOG_INFO("<update> Already theLatest ! The local version is already the latest version and "
                 "no update is needed.");
        QMessageBox::information(
            this,
            "Already the Latest!",
            "The local version is already the latest version and no update is needed.");
    }
    return needUpdate;
}

void FormSetting::getINI()
{
    m_iniUpdate.url = SETTING_GET(CFG_GROUP_SETTING,
                                  CFG_SETTING_UPDATE_URL,
                                  "http://192.168.123.14:8000");
    m_iniUpdate.check = SETTING_GET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_CHECK, VAL_DISABLE);
    ui->lineEditURL->setText(m_iniUpdate.url);
    ui->checkBoxCheckUpdates->setChecked(m_iniUpdate.check == VAL_ENABLE ? true : false);

    m_iniUpdate.tip = SETTING_GET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_TIP, VAL_DISABLE);

    m_iniServer.enable = SETTING_GET(CFG_GROUP_SERVER, CFG_SERVER_ENABLE, VAL_ENABLE);
    m_iniServer.log = SETTING_GET(CFG_GROUP_SERVER, CFG_SERVER_LOG, VAL_DISABLE);
    m_iniServer.port = SETTING_GET(CFG_GROUP_SERVER, CFG_SERVER_PORT, "12345");
    ui->checkBoxEnable->setChecked(m_iniServer.enable == VAL_ENABLE ? true : false);
    ui->checkBoxLog->setChecked(m_iniServer.log == VAL_ENABLE ? true : false);
    ui->lineEditPort->setText(m_iniServer.port);
}

void FormSetting::on_checkBoxEnable_clicked()
{
    m_iniServer.enable = ui->checkBoxEnable->isChecked() ? VAL_ENABLE : VAL_DISABLE;
    SETTING_SET(CFG_GROUP_SERVER, CFG_SERVER_ENABLE, m_iniServer.enable);
}

void FormSetting::on_checkBoxLog_clicked()
{
    m_iniServer.log = ui->checkBoxLog->isChecked() ? VAL_ENABLE : VAL_DISABLE;
    SETTING_SET(CFG_GROUP_SERVER, CFG_SERVER_LOG, m_iniServer.log);
}
