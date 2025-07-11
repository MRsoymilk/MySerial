#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QChartView>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QtCharts>
#include <limits>

class MyChartView : public QChartView
{
    Q_OBJECT

public:
    explicit MyChartView(QChart *chart, QWidget *parent = nullptr)
        : QChartView(chart, parent)
        , m_enableBack(false)
        , m_enableCrop(false)
    {
        setMouseTracking(true);
        setRubberBand(QChartView::NoRubberBand); // 默认禁用橡皮筋

        setChart(chart);
    }

    void setChart(QChart *chart)
    {
        QChartView::setChart(chart);
        m_chart = chart;

        m_axisX = nullptr;
        m_axisY = nullptr;

        const auto axes = chart->axes();
        for (QAbstractAxis *axis : axes) {
            if (axis->orientation() == Qt::Horizontal) {
                if (auto *valueAxis = qobject_cast<QValueAxis *>(axis)) {
                    m_axisX = valueAxis;
                }
            } else if (axis->orientation() == Qt::Vertical) {
                if (auto *valueAxis = qobject_cast<QValueAxis *>(axis)) {
                    m_axisY = valueAxis;
                }
            }
        }

        if (!m_axisX) {
            m_axisX = new QValueAxis();
            chart->addAxis(m_axisX, Qt::AlignBottom);
        }
        if (!m_axisY) {
            m_axisY = new QValueAxis();
            chart->addAxis(m_axisY, Qt::AlignLeft);
        }

        clearHelpers();
        initHelpers();

        // 保存初始坐标轴范围（依据第一个序列所有点范围）
        if (!m_chart->series().isEmpty()) {
            auto *xySeries = qobject_cast<QXYSeries *>(m_chart->series().first());
            if (xySeries) {
                QRectF bounds;
                for (const QPointF &p : xySeries->points())
                    bounds |= QRectF(p, QSizeF(0, 0));
                m_initialRange = bounds;

                m_axisX->setRange(bounds.left(), bounds.right());
                m_axisY->setRange(bounds.top(), bounds.bottom());
                xySeries->attachAxis(m_axisX);
                xySeries->attachAxis(m_axisY);
            }
        }
    }

    void setInitialAxisRange(const QRectF &range)
    {
        m_initialRange = range;

        if (m_axisX && m_axisY) {
            m_axisX->setRange(range.left(), range.right());
            m_axisY->setRange(range.top(), range.bottom());
        }
    }

    void recordInitialAxisRange()
    {
        if (!m_axisX || !m_axisY)
            return;

        m_initialRange = QRectF(m_axisX->min(),
                                m_axisY->min(),
                                m_axisX->max() - m_axisX->min(),
                                m_axisY->max() - m_axisY->min());
    }

    void backInitialRange()
    {
        if (!m_initialRange.isValid() || !m_axisX || !m_axisY)
            return;

        m_axisX->setRange(m_initialRange.left(), m_initialRange.right());
        m_axisY->setRange(m_initialRange.top(), m_initialRange.bottom());
    }

    void setBackEnabled(bool enabled) { m_enableBack = enabled; }

    void setCropEnabled(bool enabled)
    {
        m_enableCrop = enabled;
        setRubberBand(enabled ? QChartView::RectangleRubberBand : QChartView::NoRubberBand);
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
            m_marker->hide();
            return;
        }

        m_lineV->show();
        m_lineH->show();
        m_coordText->show();

        qreal x = value.x();
        qreal y = value.y();

        QPointF closestPoint;
        qreal minDist = std::numeric_limits<qreal>::max();
        for (auto *series : m_chart->series()) {
            auto *xySeries = qobject_cast<QXYSeries *>(series);
            if (!xySeries)
                continue;
            for (const QPointF &p : xySeries->points()) {
                qreal dist = std::hypot(p.x() - x, p.y() - y);
                if (dist < minDist) {
                    minDist = dist;
                    closestPoint = p;
                }
            }
        }

        QPointF top(pos.x(), plotArea.top());
        QPointF bottom(pos.x(), plotArea.bottom());
        QPointF left(plotArea.left(), pos.y());
        QPointF right(plotArea.right(), pos.y());

        m_lineV->setLine(QLineF(mapToScene(top.toPoint()), mapToScene(bottom.toPoint())));
        m_lineH->setLine(QLineF(mapToScene(left.toPoint()), mapToScene(right.toPoint())));

        QPointF closestScenePos = mapToScene(m_chart->mapToPosition(closestPoint).toPoint());
        m_marker->setRect(closestScenePos.x() - 4, closestScenePos.y() - 4, 8, 8);
        m_marker->show();

        QString coordText = QString("X: %1\nY: %2")
                                .arg(closestPoint.x(), 0, 'f', 3)
                                .arg(closestPoint.y(), 0, 'f', 3);
        m_coordText->setText(coordText);
        m_coordText->setPos(closestScenePos + QPointF(10, -30));

        QChartView::mouseMoveEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        if (m_enableBack) {
            backInitialRange();
        }
        QChartView::mouseDoubleClickEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (!m_enableCrop) {
            event->ignore();
            return;
        }
        QChartView::mouseReleaseEvent(event);
    }

private:
    void initHelpers()
    {
        if (!m_chart)
            return;

        m_lineV = m_chart->scene()->addLine(QLineF(), QPen(Qt::DashLine));
        m_lineH = m_chart->scene()->addLine(QLineF(), QPen(Qt::DashLine));

        m_coordText = new QGraphicsSimpleTextItem(m_chart);
        m_coordText->setZValue(11);

        m_marker = m_chart->scene()->addEllipse(QRectF(0, 0, 8, 8), QPen(Qt::red), QBrush(Qt::red));
        m_marker->setZValue(10);
        m_marker->hide();
    }

    void clearHelpers()
    {
        if (m_lineV) {
            scene()->removeItem(m_lineV);
            delete m_lineV;
            m_lineV = nullptr;
        }
        if (m_lineH) {
            scene()->removeItem(m_lineH);
            delete m_lineH;
            m_lineH = nullptr;
        }
        if (m_coordText) {
            delete m_coordText;
            m_coordText = nullptr;
        }
        if (m_marker) {
            scene()->removeItem(m_marker);
            delete m_marker;
            m_marker = nullptr;
        }
    }

private:
    QChart *m_chart = nullptr;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QGraphicsLineItem *m_lineV = nullptr;
    QGraphicsLineItem *m_lineH = nullptr;
    QGraphicsSimpleTextItem *m_coordText = nullptr;
    QGraphicsEllipseItem *m_marker = nullptr;

    QRectF m_initialRange;
    bool m_enableBack;
    bool m_enableCrop;
};

#endif // MYCHARTVIEW_H
