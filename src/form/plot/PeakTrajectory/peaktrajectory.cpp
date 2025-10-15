#include "peaktrajectory.h"

#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMenu>
#include "ui_peaktrajectory.h"

PeakTrajectory::PeakTrajectory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PeakTrajectory)
{
    ui->setupUi(this);
    init();
}

PeakTrajectory::~PeakTrajectory()
{
    delete ui;
}

void PeakTrajectory::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);

    QAction *setRangeAction = menu.addAction(tr("Set Range"));
    QAction *clearChartAction = menu.addAction(tr("Clear Chart"));

    QAction *selectedAction = menu.exec(event->globalPos());
    if (!selectedAction)
        return;

    if (selectedAction == setRangeAction) {
        bool ok = false;
        int newRange
            = QInputDialog::getInt(this, tr("Set Range"), tr("Range:"), m_range, 1, 10000, 1, &ok);
        if (ok) {
            m_range = newRange;

            int total = m_data.size();
            if (total <= m_range) {
                m_axisX->setRange(0, m_range);
            } else {
                m_axisX->setRange(total - m_range, total);
            }
        }
    } else if (selectedAction == clearChartAction) {
        m_data.clear();
        m_line->clear();

        m_axisX->setRange(0, m_range);
        m_axisY->setRange(0, 1);
    }
}

void PeakTrajectory::appendPeak(const int &value)
{
    m_data.append(value);

    int index = m_data.size() - 1;
    m_line->append(index, value);

    int rangeStart = std::max(0, static_cast<int>(m_data.length() - m_range));
    auto [minIt, maxIt] = std::minmax_element(m_data.begin() + rangeStart, m_data.end());
    int minY = *minIt;
    int maxY = *maxIt;

    if (m_data.size() == 1) {
        m_history_min = minY;
        m_history_max = maxY;
    } else {
        m_history_min = std::min(m_history_min, minY);
        m_history_max = std::max(m_history_max, maxY);
    }

    if (minY == maxY) {
        minY -= 1;
        maxY += 1;
    }
    m_axisY->setRange(minY, maxY);

    m_axisX->setTickInterval(1);
    m_axisX->setMinorTickCount(0);
    m_axisX->setLabelFormat("%d");

    int total = m_data.size();
    if (total <= m_range) {
        m_axisX->setRange(0, m_range);
    } else {
        m_axisX->setRange(total - m_range, total);
    }

    double avg = std::accumulate(m_data.begin(), m_data.end(), 0.0) / m_data.size();

    ui->labelInfo->setText(QString("history Min: %1, Max: %2\n"
                                   "min: %3, max: %4\n"
                                   "avg: %5")
                               .arg(m_history_min)
                               .arg(m_history_max)
                               .arg(minY)
                               .arg(maxY)
                               .arg(avg, 0, 'f', 2));
}

void PeakTrajectory::init()
{
    m_line = new QLineSeries();

    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    m_chart = new QChart();
    m_chart->addSeries(m_line);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_line->attachAxis(m_axisX);
    m_line->attachAxis(m_axisY);
    m_axisX->setTitleText(tr("index"));
    m_axisY->setTitleText(tr("intensity"));
    m_chart->setTitle(tr("peak trajectory"));
    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gridLayout->addWidget(m_chartView);
    m_history_min = INT_MAX;
    m_history_max = INT_MIN;
}
