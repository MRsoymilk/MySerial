#include "formplot.h"
#include "ui_formplot.h"

#include <QThread>
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
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    delete ui;
}

void FormPlot::init()
{
    m_series = new QLineSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

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

    m_workerThread = new QThread(this);
    m_worker = new PlotWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &FormPlot::newDataReceived, m_worker, &PlotWorker::processData);
    connect(m_worker, &PlotWorker::dataReady, this, &FormPlot::updatePlot, Qt::QueuedConnection);

    m_workerThread->start();
}

void FormPlot::onDataReceived(const QByteArray &data)
{
    emit newDataReceived(data);
}

void FormPlot::updatePlot(QLineSeries *line,
                          const int &points,
                          const double &min_y,
                          const double &max_y,
                          const double &min_x,
                          const double &max_x)
{
    m_series = line;
    m_chart->removeAllSeries();
    m_chart->addSeries(m_series);

    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    m_axisX->setRange(min_x, max_x);

    if (m_autoZoom) {
        double padding = (max_y - min_y) * 0.1;
        if (padding == 0)
            padding = 0.1;
        m_axisY->setRange(min_y - padding, max_y + padding);
    } else {
        m_axisY->setRange(m_fixedYMin, m_fixedYMax);
    }

    m_chart->update();
}

void FormPlot::on_tBtnZoom_clicked()
{
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);
}
