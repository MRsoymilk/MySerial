#ifndef LOADINGOVERLAY_H
#define LOADINGOVERLAY_H

#include <QWidget>

namespace Ui {
class LoadingOverLay;
}

class LoadingOverLay : public QWidget {
    Q_OBJECT

public:
    explicit LoadingOverLay(QWidget *parent = nullptr);
    ~LoadingOverLay();

    void updateTry(int count);
    void reTry();

signals:
    void stopConnect();

public slots:
    void updateInfo(int progress, const QString &msg);

private slots:
    void on_btnStop_clicked();

private:
    void init();

private:
    Ui::LoadingOverLay *ui;
    int m_count = 0;
};

#endif  // LOADINGOVERLAY_H
