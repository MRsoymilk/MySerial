#ifndef FORMSETTING_H
#define FORMSETTING_H

#include <QWidget>

class Server;
class AutoUpdate;
class Calculate;
class FrameSetting;

namespace Ui {
class FormSetting;
}

class FormSetting : public QWidget {
    Q_OBJECT
public:
    explicit FormSetting(QWidget *parent = nullptr);
    ~FormSetting();
    void retranslateUI();

public slots:
    void initThreshold();

signals:
    void windowClose();
    void sendThreshold(bool isUse, const QList<double> &values);
    void sendThresholdOption(const QJsonObject &option);
    void fullyControl(bool isUse);
    void sendDouble(bool isDouble);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_radioButtonUseSingle_clicked(bool checked);
    void on_radioButtonUseDouble_clicked(bool checked);
    void on_tBtnLoadThreshold_clicked();
    void on_checkBoxEnableInterpolation_clicked();
    void on_checkBoxEnableDebug_checkStateChanged(const Qt::CheckState &state);
    void on_checkBoxEnableLocalThreshold_clicked();
    void on_checkBoxFullyMode_clicked();

private:
    void init();
    void readThresholdCSV(const QString &path);

private:
    Ui::FormSetting *ui;
};

#endif  // FORMSETTING_H
