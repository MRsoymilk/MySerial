#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H
#include <QGraphicsSimpleTextItem>
#include <QtCharts>

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
    }

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
            return;
        }

        m_lineV->show();
        m_lineH->show();
        m_coordText->show();

        qreal x = value.x();
        qreal y = value.y();

        // Update crosshair
        QPointF top = QPointF(pos.x(), plotArea.top());
        QPointF bottom = QPointF(pos.x(), plotArea.bottom());
        QPointF left = QPointF(plotArea.left(), pos.y());
        QPointF right = QPointF(plotArea.right(), pos.y());

        m_lineV->setLine(QLineF(mapToScene(top.toPoint()), mapToScene(bottom.toPoint())));
        m_lineH->setLine(QLineF(mapToScene(left.toPoint()), mapToScene(right.toPoint())));

        // Update text
        QString coordText = QString("X: %1\nY: %2").arg(x, 0, 'f', 3).arg(y, 0, 'f', 3);
        m_coordText->setText(coordText);
        m_coordText->setPos(mapToScene(pos.toPoint()) + QPointF(10, -30));

        QChartView::mouseMoveEvent(event);
    }

private:
    QChart *m_chart;
    QGraphicsLineItem *m_lineV;
    QGraphicsLineItem *m_lineH;
    QGraphicsSimpleTextItem *m_coordText;
};
#endif
