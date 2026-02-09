#ifndef FORMPLOTSIMULATE_H
#define FORMPLOTSIMULATE_H

#include <QTimer>
#include <QWidget>

namespace Ui {
class FormPlotSimulate;
}

class FormPlotSimulate : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlotSimulate(QWidget *parent = nullptr);
    ~FormPlotSimulate();

    void retranslateUI();

signals:
    void simulateDataReady(const QByteArray &data);
    void simulateReset();
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_btnLoadFile_clicked();
    void on_toolButtonRe_clicked();

    void sendChunk();

private:
    void init();
    void getINI();
    void setINI();

    void simulate4k();

private:
    Ui::FormPlotSimulate *ui;

    struct
    {
        QString file;
    } m_ini;

    QTimer *m_timer = nullptr;
    QByteArray m_sendData;
    qint64 m_offset = 0;
    QList<QByteArray> m_frames; // 每一帧
    int m_frameIndex = 0;       // 当前发送索引
};

#endif // FORMPLOTSIMULATE_H
