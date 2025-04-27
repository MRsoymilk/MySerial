#include "formplot.h"
#include "ui_formplot.h"

#include <QTimer>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>

FormPlot::FormPlot(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlot)
{
    ui->setupUi(this);
    init();
}

FormPlot::~FormPlot()
{
    delete ui;
}

void FormPlot::init()
{
    m_series = new QLineSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_time = 0;

    m_chart->addSeries(m_series);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    m_axisX->setTitleText("Time (s)");
    m_axisX->setRange(0, 0.2);

    m_axisY->setTitleText("Voltage (V)");
    m_axisY->setRange(m_fixedYMin, m_fixedYMax);

    m_chart->legend()->hide();
    m_chart->setTitle("Live ADC Waveform");

    QChartView *view = new QChartView(m_chart);
    view->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(view);

    ui->tBtnZoom->setIcon(QIcon(":/res/icons/zoom.png"));
    ui->tBtnZoom->setIconSize(QSize(16, 16));
    ui->tBtnZoom->setCheckable(true);
    ui->tBtnZoom->setChecked(m_autoZoom);
    ui->tBtnZoom->setToolTip("Auto Zoom");
}

void FormPlot::updateData(const QByteArray &data)
{
    QByteArray payload = data.mid(5, data.size() - 7);
    qDebug() << "payload: " << payload.size();
    if (payload.size() % 3 != 0) {
        qWarning() << "Invalid data length";
        return;
    }

    int numPoints = payload.size() / 3;
    m_series->clear();

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (int i = 0; i < numPoints; ++i) {
        int idx = i * 3;
        quint32 raw = (quint8) payload[idx] << 16 | (quint8) payload[idx + 1] << 8
                      | (quint8) payload[idx + 2];

        qint32 signedRaw = static_cast<qint32>(raw);
        if (signedRaw & 0x800000)
            signedRaw |= 0xFF000000;

        double voltage = (signedRaw / double(1 << 23)) * 2.5;

        // 更新最小最大值
        if (voltage < minY)
            minY = voltage;
        if (voltage > maxY)
            maxY = voltage;

        m_series->append(m_time + m_T * i, voltage);
    }

    m_axisX->setRange(m_time, m_time + numPoints * m_T);
    m_time += numPoints * m_T;

    // ==== 设置 Y 轴 ====
    if (m_autoZoom) {
        double padding = (maxY - minY) * 0.1;
        if (padding == 0)
            padding = 0.1;
        m_axisY->setRange(minY - padding, maxY + padding);
    } else {
        m_axisY->setRange(m_fixedYMin, m_fixedYMax);
    }
}

void FormPlot::onDataReceived(const QByteArray &data)
{
    updateData(data);
}

QRectF FormPlot::calculateSeriesBounds(QLineSeries *series)
{
    if (!series || series->points().isEmpty())
        return QRectF();

    qreal minX = series->points().first().x();
    qreal maxX = series->points().first().x();
    qreal minY = series->points().first().y();
    qreal maxY = series->points().first().y();

    for (const QPointF &point : series->points()) {
        if (point.x() < minX)
            minX = point.x();
        if (point.x() > maxX)
            maxX = point.x();
        if (point.y() < minY)
            minY = point.y();
        if (point.y() > maxY)
            maxY = point.y();
    }

    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

void FormPlot::on_tBtnZoom_clicked()
{
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);

    if (m_series && m_chart) {
        if (m_autoZoom) {
            QRectF bounds = calculateSeriesBounds(m_series);
            if (!bounds.isNull()) {
                qreal marginX = (bounds.width()) * 0.05;
                qreal marginY = (bounds.height()) * 0.05;

                m_chart->axes(Qt::Vertical)
                    .first()
                    ->setRange(bounds.top() - marginY, bounds.bottom() + marginY);
            }
        } else {
            m_chart->axes(Qt::Vertical).first()->setRange(m_fixedYMin, m_fixedYMax);
        }
    }
}
