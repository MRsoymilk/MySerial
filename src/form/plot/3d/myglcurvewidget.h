#ifndef MYGLCURVEWIDGET_H
#define MYGLCURVEWIDGET_H

#include <QColor>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <QWheelEvent>

struct CurveData
{
    QVector<QVector2D> points;
    QColor color;
};

class MyGLCurveWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MyGLCurveWidget(QWidget *parent = nullptr);
    void setCurves(const QVector<CurveData> &curves);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawAxis();
    void drawCurves();
    void updateScaling();

    QVector<CurveData> curveDataList;
    QOpenGLShaderProgram shader;

    QMatrix4x4 projection, view, model;

    float yaw = 0;
    float pitch = 0;
    float zoom = 30.0f;
    QPoint lastMousePos;

    float xScale = 1.0f;
    float yScale = 1.0f;
};

#endif // MYGLCURVEWIDGET_H
