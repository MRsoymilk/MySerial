#ifndef FORMSETTING_H
#define FORMSETTING_H

#include <QWidget>

class FormTip;

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

public:
    explicit FormSetting(QWidget *parent = nullptr);
    ~FormSetting();
signals:
    void showUpdates(const QString &url);
private slots:
    void on_btnCheck_clicked();
    void on_checkBoxCheckUpdates_checkStateChanged(const Qt::CheckState &state);
    void on_lineEditURL_editingFinished();
public slots:
    void onAutoUpdate();

private:
    void getINI();
    void init();
    bool checkAndDownload(const QString &filename);
    void scriptAndUpdate(QStringList items);

private:
    Ui::FormSetting *ui;
    INI_UPDATE m_update;
    FormTip *m_tip;
};

#endif // FORMSETTING_H
