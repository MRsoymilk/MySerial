#ifndef FORMPLAYMPU6050_H
#define FORMPLAYMPU6050_H

#include <QStandardItemModel>
#include <QWidget>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QSceneLoader>

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

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::FormPlayMPU6050 *ui;

    Qt3DExtras::Qt3DWindow *m_view;
    QWidget *m_container;
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *m_airplaneEntity;
    Qt3DCore::QTransform *m_airplaneTransform;

    QStandardItemModel *m_model;
};

#endif // FORMPLAYMPU6050_H
