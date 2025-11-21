#ifndef DRAGGABLELINE_H
#define DRAGGABLELINE_H

#include <QCursor>
#include <QDebug>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QObject>
#include <QPen>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

class DraggableLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    // xPos: 传入场景坐标 (Scene Coordinate)
    DraggableLine(QChart *chart, qreal xPos, const QColor &color = Qt::red)
        : QObject(nullptr)
        , QGraphicsLineItem(nullptr)
        , m_chart(chart)
    {
        // 1. 设置主线外观 (细线)
        setPen(QPen(color, 1));
        setZValue(10); // 确保在波形之上

        // 2. 初始化位置
        QRectF plotRect = m_chart->plotArea();

        // 将 Chart 的局部坐标区域 映射到 Scene 坐标
        QPointF topScene = m_chart->mapToScene(QPointF(plotRect.left(), plotRect.top()));
        QPointF bottomScene = m_chart->mapToScene(QPointF(plotRect.left(), plotRect.bottom()));

        // 设置线条几何 (xPos 已经是 Scene 坐标)
        setLine(xPos, topScene.y(), xPos, bottomScene.y());

        // 3. 创建粗拖动区域 (视觉手柄)
        // 计算中间高度
        qreal lineLen = bottomScene.y() - topScene.y();
        qreal midTopY = topScene.y() + 0.4 * lineLen;
        qreal midBottomY = topScene.y() + 0.6 * lineLen;

        m_hitArea = new QGraphicsLineItem(this); // 设置 this 为父对象

        m_hitArea->setLine(xPos, midTopY, xPos, midBottomY);

        QPen p(QColor(200, 200, 200, 150), 15); // 半透明宽线
        p.setCapStyle(Qt::FlatCap);
        m_hitArea->setPen(p);
        m_hitArea->setZValue(11); // 比红线更高，优先捕获鼠标

        // 4. 交互设置
        setCursor(Qt::SizeHorCursor);
        // 让父子都接受鼠标，或者只让 m_hitArea 接受也可以
        // 这里让整个组合都能被拖动
        setAcceptedMouseButtons(Qt::LeftButton);
        m_hitArea->setCursor(Qt::SizeHorCursor);
    }

signals:
    void xValueChanged(qreal value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        // 记录按下的初始位置
        m_lastScenePos = event->scenePos();
        m_isDragging = true;
        event->accept(); // 必须接受，否则收不到 Move 事件
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (!m_isDragging)
            return;

        // 1. 计算增量 (Delta)
        QPointF currentPos = event->scenePos();
        qreal dx = currentPos.x() - m_lastScenePos.x();

        // 2. 获取当前线的位置
        QLineF currentLine = line();
        qreal currentX = currentLine.x1();
        qreal targetX = currentX + dx;

        // 3. 边界限制 check
        QRectF plotRect = m_chart->plotArea();
        QPointF leftBound = m_chart->mapToScene(plotRect.topLeft());
        QPointF rightBound = m_chart->mapToScene(plotRect.topRight());

        if (targetX < leftBound.x())
            targetX = leftBound.x();
        if (targetX > rightBound.x())
            targetX = rightBound.x();

        // 4. 重新计算实际可移动的 dx (防止超界)
        qreal finalDx = targetX - currentX;

        // 5. 移动图元
        // 方式 A: 使用 translate 移动几何点
        if (std::abs(finalDx) > 0.0001) {
            // 移动主线
            QLineF newLine = line();
            newLine.translate(finalDx, 0);
            setLine(newLine);

            QLineF hitLine = m_hitArea->line();
            hitLine.translate(finalDx, 0);
            m_hitArea->setLine(hitLine);

            // 6. 更新上一帧位置
            m_lastScenePos = currentPos;

            // 7. 发射信号 (坐标映射)
            // 先转回 Chart 局部坐标
            QPointF chartPos = m_chart->mapFromScene(QPointF(targetX, 0));
            // 再转为数值 (使用默认 X 轴)
            QPointF val = m_chart->mapToValue(chartPos);

            emit xValueChanged(val.x());
        }

        event->accept();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
    {
        m_isDragging = false;
        QGraphicsLineItem::mouseReleaseEvent(event);
    }

    QPainterPath shape() const override
    {
        QPainterPath path;
        QLineF l = line();
        // 创建一个宽 20px 的隐形矩形
        path.addRect(l.x1() - 10, l.y1(), 20, l.y2() - l.y1());
        return path;
    }

private:
    QChart *m_chart = nullptr;
    QGraphicsLineItem *m_hitArea = nullptr;
    QPointF m_lastScenePos;
    bool m_isDragging = false;
};

#endif // DRAGGABLELINE_H
