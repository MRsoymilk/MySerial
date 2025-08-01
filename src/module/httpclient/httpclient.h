#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPixmap>

class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient();

    void post(const QUrl &url, const QJsonObject &data);
    void getImage(const QUrl &imageUrl);

signals:
    void success(const QJsonDocument &response);
    void failure(const QString &errorString);

    void imageLoaded(const QPixmap &pixmap);
    void imageFailed(const QString &error);

private slots:
    void onFinished();

private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_currentReply = nullptr;
};

#endif // HTTPCLIENT_H
