#ifndef FORMPLOTSIMULATE_H
#define FORMPLOTSIMULATE_H

#include <QWidget>

namespace Ui {
class FormPlotSimulate;
}

class FormPlotSimulate : public QWidget
{
    Q_OBJECT
public:
    struct INI_SIMULATE
    {
        QStringList head;
        QStringList tail;
        QString file;
        QString option_correction;
    };

public:
    explicit FormPlotSimulate(QWidget *parent = nullptr);
    ~FormPlotSimulate();
    void retranslateUI();

signals:
    void windowClose();
    void simulateOption(bool enable);
    void simulateDataReady4k(const QByteArray &data14, const QByteArray &data24);

public slots:
    void onChangeFrameType(int index);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_btnLoadFile_clicked();
    void on_toolButtonRe_clicked();
    void on_checkBoxCorrection_clicked();

private:
    void init();
    void getINI();
    void setINI();
    void simulate4k();

private:
    Ui::FormPlotSimulate *ui;
    INI_SIMULATE m_ini;
    int m_algorithm;
};

#endif // FORMPLOTSIMULATE_H
