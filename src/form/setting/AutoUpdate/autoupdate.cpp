#include "autoupdate.h"
#include "datadef.h"
#include "funcdef.h"
#include "httpclient.h"
#include "ui_autoupdate.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include "version.h"

AutoUpdate::AutoUpdate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AutoUpdate)
{
    ui->setupUi(this);
    init();
}

AutoUpdate::~AutoUpdate()
{
    delete ui;
}

void AutoUpdate::showEvent(QShowEvent *event)
{
    QJsonObject res = checkUpdate();

    ui->progressBar->setVisible(false);

    ui->lineEditURL->setText(QString("%1/%2").arg(res["url"].toString(), res["file"].toString()));
    ui->textBrowser->setText(res["description"].toString());
    ui->lineEditTargetVersion->setText(res["version"].toString());
    ui->lineEditCurrentVersion->setText(APP_VERSION);
}

QJsonObject AutoUpdate::checkUpdate()
{
    m_url = SETTING_CONFIG_GET(CFG_GROUP_AUTOUPDATE, CFG_AUTOUPDATE_URL, SERVER_UPDATE);
    ui->labelUpdateHistory->setText(
        QString("update history: <a href='%1/update.txt'>%1/update.txt</a>").arg(m_url));
    QString url_json = QString("%1/%2").arg(m_url).arg("update.json");
    m_objUpdate = m_http->get_sync(url_json);
    return m_objUpdate;
}

void AutoUpdate::init()
{
    m_iniUpdate.url = SETTING_CONFIG_GET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_URL, SERVER_UPDATE);
    m_iniUpdate.check = SETTING_CONFIG_GET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_CHECK, VAL_DISABLE);
    ui->lineEditURL->setText(m_iniUpdate.url);
    ui->labelUpdateHistory->setTextFormat(Qt::RichText);
    ui->labelUpdateHistory->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->labelUpdateHistory->setOpenExternalLinks(true);
    ui->checkBoxUpdateCheck->setChecked(m_iniUpdate.check == VAL_ENABLE ? true : false);

    m_iniUpdate.tip = SETTING_CONFIG_GET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_TIP, VAL_DISABLE);

    m_http = new HttpClient;
}

void AutoUpdate::on_btnCheck_clicked()
{
    ui->progressBar->setVisible(true);
    QString to_downloaded = m_objUpdate["file"].toString();
    m_http->downloadBinary(
        ui->lineEditURL->text(),
        [=](QByteArray data) {
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString fullPath = QDir(tempDir).filePath(to_downloaded);

            QFile file(fullPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
                ui->textBrowser->append("✅ 下载完成并保存成功：" + fullPath);

                QProcess::startDetached(fullPath);

            } else {
                ui->textBrowser->append("❌ 无法保存文件：" + fullPath);
            }
        },
        [=](QString err) { ui->textBrowser->append("❌ 下载失败: " + err); },
        [=](qint64 received, qint64 total) {
            if (total > 0) {
                int percent = static_cast<int>((double(received) / total) * 100);
                ui->progressBar->setValue(percent);
            }
        });
}

void AutoUpdate::on_checkBoxUpdateCheck_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::CheckState::Checked) {
        m_iniUpdate.check = VAL_ENABLE;
    } else {
        m_iniUpdate.check = VAL_DISABLE;
    }
    SETTING_CONFIG_SET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_CHECK, m_iniUpdate.check);
}

void AutoUpdate::on_lineEditURL_editingFinished()
{
    m_iniUpdate.url = ui->lineEditURL->text();
    SETTING_CONFIG_SET(CFG_GROUP_SETTING, CFG_SETTING_UPDATE_URL, m_iniUpdate.url);
}
