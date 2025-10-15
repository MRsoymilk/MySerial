#include <QCursor>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QtCharts/QChart>

class DraggableLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    DraggableLine(QChart *chart, qreal xPos, const QColor &color = Qt::red)
        : m_chart(chart)
    {
        setPen(QPen(color, 2));
        setZValue(999);
        setCursor(Qt::SizeHorCursor);
        setFlag(ItemIsSelectable);
        setAcceptedMouseButtons(Qt::LeftButton);

        // 初始位置（scene坐标）
        QRectF plotRect = m_chart->plotArea();
        QPointF top = m_chart->mapToScene(QPointF(xPos, plotRect.top()));
        QPointF bottom = m_chart->mapToScene(QPointF(xPos, plotRect.bottom()));
        setLine(QLineF(top, bottom));
    }

signals:
    void xValueChanged(qreal x);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        m_lastScenePos = event->scenePos();
        m_lastX = line().x1();
        event->accept();
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        QPointF delta = event->scenePos() - m_lastScenePos;
        qreal newX = m_lastX + delta.x();

        // 限制范围（基于scene坐标）
        QRectF plotRect = m_chart->plotArea();
        QPointF leftScene = m_chart->mapToScene(plotRect.topLeft());
        QPointF rightScene = m_chart->mapToScene(plotRect.topRight());

        if (newX < leftScene.x())
            newX = leftScene.x();
        if (newX > rightScene.x())
            newX = rightScene.x();

        // 更新竖线位置
        QLineF l = line();
        qreal dx = newX - l.x1();
        l.translate(dx, 0);
        setLine(l);

        event->accept();
        QPointF scenePos = event->scenePos();
        QPointF value = m_chart->mapToValue(m_chart->mapFromScene(scenePos));
        emit xValueChanged(value.x());
    }

private:
    QChart *m_chart = nullptr;
    QPointF m_lastScenePos;
    qreal m_lastX = 0;
};
