#ifndef EXTERNALCONTROL_H
#define EXTERNALCONTROL_H

#include <QWidget>
#include "server.h"

namespace Ui {
class ExternalControl;
}

class ExternalControl : public QWidget
{
    Q_OBJECT
public:
    struct INI_SERVER
    {
        QString port;
        QString enable;
        QString log;
    };

public:
    explicit ExternalControl(QWidget *parent = nullptr);
    ~ExternalControl();
    void retranslateUI();

private slots:
    void on_checkBoxEnable_clicked();
    void on_checkBoxLog_clicked();

private:
    void init();

private:
    Ui::ExternalControl *ui;
    INI_SERVER m_iniServer;
    Server *m_server = nullptr;
};

#endif // EXTERNALCONTROL_H
