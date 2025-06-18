#include "myglcurvewidget.h"
#include <QPainter>
#include <algorithm>

MyGLCurveWidget::MyGLCurveWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{}

void MyGLCurveWidget::setCurves(const QVector<CurveData> &curves)
{
    curveDataList = curves;
    updateScaling();
    update();
}

void MyGLCurveWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    shader.addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
        uniform mat4 mvp;
        attribute vec3 position;
        void main() {
            gl_Position = mvp * vec4(position, 1.0);
        }
    )");

    shader.addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
        uniform vec3 curveColor;
        void main() {
            gl_FragColor = vec4(curveColor, 1.0);
        }
    )");

    shader.link();
}

void MyGLCurveWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    projection.setToIdentity();
    projection.perspective(45.0f, float(w) / float(h), 0.1f, 100.0f);
}

void MyGLCurveWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view.setToIdentity();
    view.translate(0, 0, -zoom);
    view.rotate(pitch, 1, 0, 0);
    view.rotate(yaw, 0, 1, 0);

    shader.bind();
    shader.setUniformValue("mvp", projection * view * model);

    drawAxis();
    drawCurves();

    shader.release();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::gray);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    auto mapToScreen = [&](const QVector3D &world) -> QPoint {
        QVector4D clip = projection * view * model * QVector4D(world, 1.0);
        if (clip.w() == 0.0f)
            return QPoint();
        QVector3D ndc = clip.toVector3DAffine();
        return QPoint(width() * (ndc.x() * 0.5f + 0.5f), height() * (-ndc.y() * 0.5f + 0.5f));
    };

    painter.drawText(mapToScreen(QVector3D(3, 0, 0)), "X");
    painter.drawText(mapToScreen(QVector3D(0, 3, 0)), "Y");
    painter.drawText(mapToScreen(QVector3D(0, 0, 3)), "Z");
    painter.end();
}

void MyGLCurveWidget::mousePressEvent(QMouseEvent *event)
{
    lastMousePos = event->pos();
}

void MyGLCurveWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - lastMousePos;
    yaw += delta.x() * 0.4f;
    pitch += delta.y() * 0.4f;
    lastMousePos = event->pos();
    update();
}

void MyGLCurveWidget::wheelEvent(QWheelEvent *event)
{
    zoom -= event->angleDelta().y() / 120.0f;
    zoom = std::clamp(zoom, 5.0f, 200.0f);
    update();
}

void MyGLCurveWidget::updateScaling()
{
    float xMax = 1.0f, yMax = 1.0f;

    for (const auto &curve : curveDataList) {
        for (const QVector2D &pt : curve.points) {
            xMax = std::max(xMax, std::abs(pt.x()));
            yMax = std::max(yMax, std::abs(pt.y()));
        }
    }

    xScale = 10.0f / xMax;
    yScale = 10.0f / yMax;
}

void MyGLCurveWidget::drawAxis()
{
    QVector<QVector3D> axis = {
        {0, 0, 0},
        {1000, 0, 0}, // X
        {0, 0, 0},
        {0, 10, 0}, // Y
        {0, 0, 0},
        {0, 0, 3} // Z
    };

    shader.setUniformValue("curveColor", QVector3D(0.4f, 0.4f, 0.4f));
    int loc = shader.attributeLocation("position");
    shader.enableAttributeArray(loc);
    shader.setAttributeArray(loc, GL_FLOAT, axis.constData(), 3);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, axis.size());
    shader.disableAttributeArray(loc);
}

void MyGLCurveWidget::drawCurves()
{
    int loc = shader.attributeLocation("position");
    int layer = 0;

    for (const CurveData &curve : curveDataList) {
        QVector<QVector3D> curve3D;
        for (const QVector2D &pt : curve.points)
            curve3D.append(QVector3D(pt.x() * xScale, pt.y() * yScale, layer));

        shader.setUniformValue("curveColor",
                               QVector3D(curve.color.redF(),
                                         curve.color.greenF(),
                                         curve.color.blueF()));

        shader.enableAttributeArray(loc);
        shader.setAttributeArray(loc, GL_FLOAT, curve3D.constData(), 3);

        glLineWidth(2.5f);
        glDrawArrays(GL_LINE_STRIP, 0, curve3D.size());

        shader.disableAttributeArray(loc);
        ++layer;
    }
}
