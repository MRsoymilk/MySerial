#ifndef LOADINGOVERLAY_H
#define LOADINGOVERLAY_H

#include <QWidget>

namespace Ui {
class LoadingOverLay;
}

class LoadingOverLay : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingOverLay(QWidget *parent = nullptr);
    ~LoadingOverLay();

    void updateTry(int count);
public slots:
    void updateInfo(int progress, const QString &msg);

private:
    void init();

private:
    Ui::LoadingOverLay *ui;
};

#endif // LOADINGOVERLAY_H
