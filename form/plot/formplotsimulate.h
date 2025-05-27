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
    };

public:
    explicit FormPlotSimulate(QWidget *parent = nullptr);
    ~FormPlotSimulate();
signals:
    void windowClose();
    void simulateDataReady(const QByteArray &data, const QString &name);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_btnLoadFile_clicked();

    void on_toolButtonRe_clicked();

private:
    void init();
    void getINI();
    void setINI();

private:
    Ui::FormPlotSimulate *ui;
    INI_SIMULATE m_ini;
    void simulate();
};

#endif // FORMPLOTSIMULATE_H
