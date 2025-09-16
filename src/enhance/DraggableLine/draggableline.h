#ifndef DRAGGABLELINE_H
#define DRAGGABLELINE_H

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QObject>
#include <QPen>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

class DraggableLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    enum Orientation { Vertical, Horizontal };
    DraggableLine(Orientation ori, QChart *chart, QGraphicsItem *parent = nullptr);

    void setAxis(QValueAxis *axisX, QValueAxis *axisY);
    double value() const;
    void setHover(bool hover); // 新增：设置悬停状态

signals:
    void moved();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void updateLabel();
    void updateInteractionRect();

    Orientation orientation;
    QGraphicsSimpleTextItem *label;
    QGraphicsRectItem *interactionRect; // 交互区域
    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;
    QChart *chart = nullptr;
    bool isHovered = false;
};

#endif // DRAGGABLELINE_H
