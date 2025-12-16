#include "httpclient.h"

#include <QEventLoop>
#include <QJsonObject>
#include <QTimer>

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

void HttpClient::downloadBinary(const QString &url,
                                std::function<void(QByteArray)> onSuccess,
                                std::function<void(QString)> onError,
                                std::function<void(qint64, qint64)> onProgress)
{
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    QNetworkReply *reply = m_manager->get(request);

    if (onProgress) {
        connect(reply,
                &QNetworkReply::downloadProgress,
                reply,
                [reply, onProgress](qint64 received, qint64 total) { onProgress(received, total); });
    }

    connect(reply, &QNetworkReply::finished, reply, [reply, onSuccess, onError]() {
        if (reply->error() != QNetworkReply::NoError) {
            onError(reply->errorString());
        } else {
            QByteArray data = reply->readAll();
            onSuccess(data);
        }
        reply->deleteLater();
    });

    connect(reply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            reply,
            [reply, onError](QNetworkReply::NetworkError) { onError(reply->errorString()); });
}

QJsonObject HttpClient::get_sync(const QString &url, int timeoutMs)
{
    QUrl qurl(url);
    QNetworkRequest request(qurl);

    QNetworkReply *reply = m_manager->get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    connect(&timer, &QTimer::timeout, [&]() {
        qWarning() << "[MyHttp] GET timeout:" << url;
        reply->abort();
        loop.quit();
    });

    timer.start(timeoutMs);
    loop.exec();

    QJsonObject result;

    if (timer.isActive() && reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            result = doc.object();
        } else {
            qWarning() << "[MyHttp] JSON parse error:" << err.errorString();
        }
    } else if (!timer.isActive()) {
        qWarning() << "[MyHttp] GET aborted due to timeout";
    } else {
        qWarning() << "[MyHttp] GET Error:" << reply->errorString();
    }

    reply->deleteLater();
    return result;
}
