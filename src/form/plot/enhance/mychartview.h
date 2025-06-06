#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QGraphicsSimpleTextItem>
#include <QtCharts>
#include <limits>

class MyChartView : public QChartView
{
    Q_OBJECT

public:
    MyChartView(QChart *chart, QWidget *parent = nullptr)
        : QChartView(chart, parent)
        , m_chart(chart)
    {
        setMouseTracking(true);

        m_lineV = m_chart->scene()->addLine(QLineF(), QPen(Qt::DashLine));
        m_lineH = m_chart->scene()->addLine(QLineF(), QPen(Qt::DashLine));

        m_coordText = new QGraphicsSimpleTextItem(m_chart);
        m_coordText->setZValue(11);

        m_marker = m_chart->scene()->addEllipse(QRectF(0, 0, 8, 8), QPen(Qt::red), QBrush(Qt::red));
        m_marker->setZValue(10);
        m_marker->hide();
    }

    void setChart(QChart *chart) {}

protected:
    void mouseMoveEvent(QMouseEvent *event) override
    {
        QPointF pos = event->pos();
        QPointF value = m_chart->mapToValue(pos);

        QRectF plotArea = m_chart->plotArea();
        if (!plotArea.contains(pos)) {
            m_lineV->hide();
            m_lineH->hide();
            m_coordText->hide();
            m_marker->hide();
            return;
        }

        m_lineV->show();
        m_lineH->show();
        m_coordText->show();

        qreal x = value.x();
        qreal y = value.y();

        // 吸附：查找所有 QXYSeries 中最近的点
        QPointF closestPoint;
        qreal minDist = std::numeric_limits<qreal>::max();
        for (auto *series : m_chart->series()) {
            auto *xySeries = qobject_cast<QXYSeries *>(series);
            if (!xySeries)
                continue;

            const auto &points = xySeries->points();
            for (const QPointF &p : points) {
                qreal dist = std::hypot(p.x() - x, p.y() - y);
                if (dist < minDist) {
                    minDist = dist;
                    closestPoint = p;
                }
            }
        }

        // 更新十字线
        QPointF sceneMouse = mapToScene(pos.toPoint());
        QPointF top = QPointF(pos.x(), plotArea.top());
        QPointF bottom = QPointF(pos.x(), plotArea.bottom());
        QPointF left = QPointF(plotArea.left(), pos.y());
        QPointF right = QPointF(plotArea.right(), pos.y());

        m_lineV->setLine(QLineF(mapToScene(top.toPoint()), mapToScene(bottom.toPoint())));
        m_lineH->setLine(QLineF(mapToScene(left.toPoint()), mapToScene(right.toPoint())));

        // 更新吸附点圆圈
        QPointF closestScenePos = mapToScene(m_chart->mapToPosition(closestPoint).toPoint());
        m_marker->setRect(closestScenePos.x() - 4, closestScenePos.y() - 4, 8, 8);
        m_marker->show();

        // 更新坐标文本
        QString coordText = QString("X: %1\nY: %2")
                                .arg(closestPoint.x(), 0, 'f', 3)
                                .arg(closestPoint.y(), 0, 'f', 3);
        m_coordText->setText(coordText);
        m_coordText->setPos(closestScenePos + QPointF(10, -30));

        QChartView::mouseMoveEvent(event);
    }

private:
    QChart *m_chart;
    QGraphicsLineItem *m_lineV;
    QGraphicsLineItem *m_lineH;
    QGraphicsSimpleTextItem *m_coordText;
    QGraphicsEllipseItem *m_marker; // 用于标记最近点
};

#endif // MYCHARTVIEW_H
