#ifndef FORMSETTING_H
#define FORMSETTING_H

#include <QWidget>

class FormTip;
class Server;

namespace Ui {
class FormSetting;
}

class FormSetting : public QWidget
{
    Q_OBJECT
public:
    struct INI_UPDATE
    {
        QString url;
        QString check;
        QString tip;
    };

    struct INI_SERVER
    {
        QString port;
        QString enable;
        QString log;
    };

public:
    explicit FormSetting(QWidget *parent = nullptr);
    ~FormSetting();
signals:
    void showUpdates(const QString &url);
private slots:
    void on_btnCheck_clicked();
    void on_checkBoxCheckUpdates_checkStateChanged(const Qt::CheckState &state);
    void on_lineEditURL_editingFinished();
    void on_checkBoxEnable_clicked();

    void on_checkBoxLog_clicked();

public slots:
    void onAutoUpdate();

private:
    void getINI();
    void init();
    bool checkAndDownload(const QString &filename);
    void scriptAndUpdate(QStringList items);

private:
    Ui::FormSetting *ui;
    INI_UPDATE m_iniUpdate;
    INI_SERVER m_iniServer;
    FormTip *m_tip;
    Server *m_server;
};

#endif // FORMSETTING_H
