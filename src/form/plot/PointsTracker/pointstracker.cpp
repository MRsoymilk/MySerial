#include "pointstracker.h"
#include "ui_pointstracker.h"
#include "MyChartView/mychartview.h"

PointsTracker::PointsTracker(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PointsTracker)
{
    ui->setupUi(this);
    init();
}

PointsTracker::~PointsTracker()
{
    delete ui;
}

void PointsTracker::addPoints(QMap<QString, double> values)
{
    for (auto it = values.begin(); it != values.end(); ++it) {
        const QString &name = it.key();
        double value = it.value();

        if (!m_mapLines.contains(name)) {
            QLineSeries *series = new QLineSeries;
            series->setName(name);
            static int colorIndex = 0;
            QColor color = QColor::fromHsv((colorIndex * 40) % 360, 200, 220);
            series->setColor(color);
            colorIndex++;
            for (int i = 0; i < m_idx; ++i) {
                series->append(i, std::numeric_limits<double>::quiet_NaN());
            }
            m_chart->addSeries(series);
            series->attachAxis(m_axisX);
            series->attachAxis(m_axisY);
            m_mapLines.insert(name, series);
        }

        QLineSeries *series = m_mapLines[name];
        series->append(m_idx, value);
        m_axisY->setRange(
            std::min(m_axisY->min(), value),
            std::max(m_axisY->max(), value)
            );
    }

    if (m_idx > m_axisX->max())
        m_axisX->setMax(m_idx);

    m_idx++;
}

void PointsTracker::clearPoints()
{
    m_chart->removeAllSeries();
    m_axisX->setRange(0, 10);
    m_axisY->setRange(0, 1);
    m_mapLines.clear();
    m_idx = 0;
}

void PointsTracker::init()
{
    m_chart = new QChart();
    m_chart->setTitle(tr("Points Tracker"));

    m_axisX = new QValueAxis;
    m_axisX->setTitleText(tr("Index"));

    m_axisY = new QValueAxis;
    m_axisY->setTitleText(tr("Value"));

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->gLayChart->addWidget(m_chartView);
}

void PointsTracker::closeEvent(QCloseEvent *event)
{
    emit windowClose();
}
