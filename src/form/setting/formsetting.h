#ifndef FORMSETTING_H
#define FORMSETTING_H

#include <QWidget>

class Server;
class AutoUpdate;
class Calculate;
class ExternalControl;

namespace Ui {
class FormSetting;
}

class FormSetting : public QWidget
{
    Q_OBJECT
public:
    explicit FormSetting(QWidget *parent = nullptr);
    ~FormSetting();
    void retranslateUI();

private:
    void init();

private:
    Ui::FormSetting *ui;

    AutoUpdate *m_autoUpdate;
    Calculate *m_calculate;
    ExternalControl *m_externalControl;
};

#endif // FORMSETTING_H
