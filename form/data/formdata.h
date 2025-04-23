#ifndef FORMDATA_H
#define FORMDATA_H

#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class FormData;
}

class FormData : public QWidget
{
    Q_OBJECT

public:
    explicit FormData(QWidget *parent = nullptr);
    ~FormData();

public slots:
    void onDataReceived(const QByteArray &data);

private:
    void init();

private:
    Ui::FormData *ui;
    QStandardItemModel *model;
};

#endif // FORMDATA_H
