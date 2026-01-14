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
        QString file;
    };

public:
    explicit FormPlotSimulate(QWidget *parent = nullptr);
    ~FormPlotSimulate();
    void retranslateUI();

signals:
    void windowClose();
    void simulateDataReady(const QByteArray &data);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_btnLoadFile_clicked();
    void on_toolButtonRe_clicked();

private:
    void init();
    void getINI();
    void setINI();
    void simulate4k();

private:
    Ui::FormPlotSimulate *ui;
    INI_SIMULATE m_ini;
};

#endif // FORMPLOTSIMULATE_H
