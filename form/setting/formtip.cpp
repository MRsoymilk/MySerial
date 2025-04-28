#include "formtip.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
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
        qDebug() << "Response:" << data;
        ui->textBrowser->setText(data);
    } else {
        qDebug() << "Error:" << reply->errorString();
    }

    reply->deleteLater();
}
