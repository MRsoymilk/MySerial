#ifndef FORMPLAYMPU6050_H
#define FORMPLAYMPU6050_H

#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class FormPlayMPU6050;
}

class FormPlayMPU6050 : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlayMPU6050(QWidget *parent = nullptr);
    ~FormPlayMPU6050();
public slots:
    void onRecvMPU(const QByteArray &data);

private:
    Ui::FormPlayMPU6050 *ui;
    QStandardItemModel *m_model;
};

#endif // FORMPLAYMPU6050_H
