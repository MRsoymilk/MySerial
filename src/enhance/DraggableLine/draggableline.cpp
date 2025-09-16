#include "draggableline.h"
#include <QGraphicsScene>
#include <QtCharts/QValueAxis>

DraggableLine::DraggableLine(Orientation ori, QChart *chart, QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
    , orientation(ori)
    , chart(chart)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true); // 启用悬停事件

    setZValue(100);

    if (orientation == Vertical) {
        setLine(0, -1000, 0, 1000); // 垂直线
    } else {
        setLine(-1000, 1000, 0, 0); // 水平线
    }

    label = new QGraphicsSimpleTextItem("0", this);
    label->setBrush(Qt::darkGray);
    label->setFont(QFont("Arial", 10, QFont::Bold));

    // 创建交互区域
    interactionRect = new QGraphicsRectItem(this);
    interactionRect->setPen(QPen(Qt::transparent));     // 透明边框
    interactionRect->setBrush(QBrush(Qt::transparent)); // 透明填充
    interactionRect->setFlags(QGraphicsItem::ItemIsMovable
                              | QGraphicsItem::ItemSendsGeometryChanges);
    interactionRect->setAcceptHoverEvents(true);
    updateInteractionRect();

    updateLabel();
}

void DraggableLine::setAxis(QValueAxis *axisX, QValueAxis *axisY)
{
    this->axisX = axisX;
    this->axisY = axisY;
}

double DraggableLine::value() const
{
    if (!scene() || !axisX || !axisY || !chart)
        return 0;

    QRectF plotArea = chart->plotArea();
    if (plotArea.isEmpty() || plotArea.width() <= 0 || plotArea.height() <= 0)
        return 0;

    if (orientation == Vertical) {
        qreal x = pos().x();
        qreal plotWidth = plotArea.width();
        qreal plotLeft = plotArea.left();
        return axisX->min() + (axisX->max() - axisX->min()) * ((x - plotLeft) / plotWidth);
    } else {
        qreal y = pos().y();
        qreal plotHeight = plotArea.height();
        qreal plotTop = plotArea.top();
        return axisY->max() - (axisY->max() - axisY->min()) * ((y - plotTop) / plotHeight);
    }
}

void DraggableLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setSelected(true);
        QGraphicsLineItem::mousePressEvent(event);
    }
}

void DraggableLine::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (orientation == Horizontal) {
        setPos(pos().x(), event->scenePos().y());
        interactionRect->setPos(pos().x(), event->scenePos().y());
    } else {
        setPos(event->scenePos().x(), pos().y());
        interactionRect->setPos(event->scenePos().x(), pos().y());
    }
    updateLabel();
    emit moved();
}

void DraggableLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setHover(true);
    QGraphicsLineItem::hoverEnterEvent(event);
}

void DraggableLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setHover(false);
    QGraphicsLineItem::hoverLeaveEvent(event);
}

void DraggableLine::setHover(bool hover)
{
    isHovered = hover;
    if (hover) {
        setPen(QPen(Qt::cyan, 10, Qt::DashLine));
    } else {
        setPen(QPen(Qt::darkGray, 1));
    }
}

void DraggableLine::updateLabel()
{
    if (label) {
        label->setText(QString("%1").arg(value(), 0, 'f', 2));
        if (orientation == Vertical) {
            label->setPos(5, -20);
        } else {
            label->setPos(5, -10);
        }
    }
}

void DraggableLine::updateInteractionRect()
{
    if (interactionRect) {
        if (orientation == Vertical) {
            interactionRect->setRect(-10, -1000, 20, 2000); // 垂直线：宽 20 像素
        } else {
            interactionRect->setRect(-1000, -10, 2000, 20); // 水平线：高 20 像素
        }
    }
}
