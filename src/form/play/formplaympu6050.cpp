#include "formplaympu6050.h"
#include "ui_formplaympu6050.h"

#include <QFile>

FormPlayMPU6050::FormPlayMPU6050(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlayMPU6050)
    , m_model(nullptr)
{
    ui->setupUi(this);

    m_model = new QStandardItemModel(4, 3, this);
    m_model->setHorizontalHeaderLabels({"X", "Y", "Z"});
    m_model->setVerticalHeaderLabels({"Accel", "Gyro", "Euler", "Temp"});
    ui->tableView->setModel(m_model);
}

FormPlayMPU6050::~FormPlayMPU6050()
{
    delete ui;
}

void FormPlayMPU6050::onRecvMPU(const QByteArray &data)
{
    int start = data.indexOf("<MPU>");
    int end = data.indexOf("<END>", start);

    if (start != -1 && end != -1 && end > start) {
        QByteArray packet = data.mid(start + 5, end - (start + 5));

        QList<QByteArray> parts = packet.split(',');
        if (parts.size() == 10) {
            float ax = parts[0].toFloat();
            float ay = parts[1].toFloat();
            float az = parts[2].toFloat();
            float gx = parts[3].toFloat();
            float gy = parts[4].toFloat();
            float gz = parts[5].toFloat();
            float roll = parts[6].toFloat();
            float pitch = parts[7].toFloat();
            float yaw = parts[8].toFloat();
            float temp = parts[9].toFloat();

            m_model->setItem(0, 0, new QStandardItem(QString::number(ax, 'f', 2)));
            m_model->setItem(0, 1, new QStandardItem(QString::number(ay, 'f', 2)));
            m_model->setItem(0, 2, new QStandardItem(QString::number(az, 'f', 2)));

            m_model->setItem(1, 0, new QStandardItem(QString::number(gx, 'f', 2)));
            m_model->setItem(1, 1, new QStandardItem(QString::number(gy, 'f', 2)));
            m_model->setItem(1, 2, new QStandardItem(QString::number(gz, 'f', 2)));

            m_model->setItem(2, 0, new QStandardItem(QString::number(roll, 'f', 2)));
            m_model->setItem(2, 1, new QStandardItem(QString::number(pitch, 'f', 2)));
            m_model->setItem(2, 2, new QStandardItem(QString::number(yaw, 'f', 2)));

            m_model->setItem(3, 0, new QStandardItem(QString::number(temp, 'f', 2)));
            m_model->setItem(3, 1, new QStandardItem(""));
            m_model->setItem(3, 2, new QStandardItem(""));
        }
    }
}
