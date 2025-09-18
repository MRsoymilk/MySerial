#include "formtip.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "funcdef.h"
#include "ui_formtip.h"

FormTip::FormTip(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormTip)
{
    ui->setupUi(this);
}

FormTip::~FormTip()
{
    delete ui;
}

void FormTip::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormTip::onFetchUpdates(const QString &url)
{
    QNetworkAccessManager manager;
    QNetworkRequest request((QUrl(url + "/update.txt")));

    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        ui->textBrowser->setText(data);
    } else {
        LOG_WARN("Error: {}", reply->errorString());
    }

    reply->deleteLater();
}
