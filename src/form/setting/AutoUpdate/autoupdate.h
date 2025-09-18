#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QJsonObject>
#include <QWidget>

class HttpClient;

namespace Ui {
class AutoUpdate;
}

class AutoUpdate : public QWidget
{
    Q_OBJECT
public:
    struct INI_UPDATE
    {
        QString url;
        QString check;
        QString tip;
    };

public:
    explicit AutoUpdate(QWidget *parent = nullptr);
    ~AutoUpdate();
    void init();
    void retranslateUI();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_btnCheck_clicked();
    void on_checkBoxUpdateCheck_checkStateChanged(const Qt::CheckState &state);
    void on_lineEditURL_editingFinished();

private:
    Ui::AutoUpdate *ui;
    INI_UPDATE m_iniUpdate;
    QJsonObject checkUpdate();
    HttpClient *m_http;
    QString m_url;
    QJsonObject m_objUpdate;
};

#endif // AUTOUPDATE_H
