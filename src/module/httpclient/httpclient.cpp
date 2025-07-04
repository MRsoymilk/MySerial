#include "httpclient.h"

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

HttpClient::~HttpClient()
{
    delete m_manager;
}

void HttpClient::post(const QUrl &url, const QJsonObject &data)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonDocument doc(data);
    m_currentReply = m_manager->post(request, doc.toJson());
    connect(m_currentReply, &QNetworkReply::finished, this, &HttpClient::onFinished);
}

void HttpClient::onFinished()
{
    m_currentReply->deleteLater();

    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        emit success(doc);
    } else {
        emit failure(m_currentReply->errorString());
    }

    m_currentReply = nullptr;
}

void HttpClient::getImage(const QUrl &imageUrl)
{
    QNetworkRequest request(imageUrl);
    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pixmap;
            if (pixmap.loadFromData(reply->readAll())) {
                emit imageLoaded(pixmap);
            } else {
                emit imageFailed("Failed to decode image");
            }
        } else {
            emit imageFailed(reply->errorString());
        }
    });
}
