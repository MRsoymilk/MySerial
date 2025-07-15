#include "formplaympu6050.h"
#include "ui_formplaympu6050.h"

#include <QResizeEvent>

FormPlayMPU6050::FormPlayMPU6050(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlayMPU6050)
{
    ui->setupUi(this);

    m_model = new QStandardItemModel(4, 3, this);
    m_model->setHorizontalHeaderLabels({"X", "Y", "Z"});
    m_model->setVerticalHeaderLabels({"Accel", "Gyro", "Euler", "Temp"});
    ui->tableView->setModel(m_model);

    m_view = new Qt3DExtras::Qt3DWindow();
    m_container = QWidget::createWindowContainer(m_view);
    ui->gLayPlane->addWidget(m_container);

    m_rootEntity = new Qt3DCore::QEntity();

    Qt3DRender::QCamera *camera = m_view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 10.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));

    auto *camController = new Qt3DExtras::QOrbitCameraController(m_rootEntity);
    camController->setCamera(camera);

    auto *loader = new Qt3DRender::QSceneLoader(m_rootEntity);
    loader->setSource(QUrl::fromLocalFile(":/res/models/airplane.gltf"));

    m_airplaneTransform = new Qt3DCore::QTransform();
    m_airplaneEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_airplaneEntity->addComponent(loader);
    m_airplaneEntity->addComponent(m_airplaneTransform);

    auto *material = new Qt3DExtras::QPhongMaterial(m_rootEntity);
    m_airplaneEntity->addComponent(material);

    m_view->setRootEntity(m_rootEntity);
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

            QQuaternion q = QQuaternion::fromEulerAngles(pitch, yaw, roll);
            m_airplaneTransform->setRotation(q);

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

void FormPlayMPU6050::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!m_view || !m_airplaneTransform)
        return;

    QSize size = event->size();
    float aspect = float(size.width()) / size.height();
    auto *camera = m_view->camera();

    camera->lens()->setPerspectiveProjection(45.0f, aspect, 0.1f, 1000.0f);

    float baseDistance = 10.0f;
    float scaleFactor = qMin(size.width(), size.height()) / 400.0f;
    float camDistance = baseDistance / scaleFactor;
    camera->setPosition(QVector3D(0, 0, camDistance));

    float modelBaseScale = 1.0f;
    float modelScale = scaleFactor * modelBaseScale;
    m_airplaneTransform->setScale(modelScale);
}
