#include "formplot.h"
#include "ui_formplot.h"

#include <QThread>
#include <QTimer>
#include <QWheelEvent>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataArray>
#include <QtDataVisualization/QSurfaceDataProxy>

FormPlot::FormPlot(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlot)
{
    ui->setupUi(this);
    init();
}

FormPlot::~FormPlot()
{
    if (m_plotdata) {
        m_plotdata->close();
        delete m_plotdata;
    }
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

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->stackedWidget->addWidget(m_chartView);

    m_surface = new Q3DSurface();
    m_surfaceAxisX = new QValue3DAxis();
    m_surfaceAxisY = new QValue3DAxis();
    m_surfaceAxisZ = new QValue3DAxis();
    m_surfaceAxisX->setTitle("Time (s)");
    m_surfaceAxisY->setTitle("Voltage (V)");
    m_surfaceAxisZ->setTitle("line");
    m_surfaceAxisX->setTitleVisible(true);
    m_surfaceAxisY->setTitleVisible(true);
    m_surfaceAxisZ->setTitleVisible(true);
    m_surface->setAxisX(m_surfaceAxisX);
    m_surface->setAxisY(m_surfaceAxisY);
    m_surface->setAxisZ(m_surfaceAxisZ);
    m_surface->setTitle("Live ADC Waveform");
    m_surfaceProxy = new QSurfaceDataProxy();
    m_surfaceSeries = new QSurface3DSeries(m_surfaceProxy);

    m_surfaceSeries->setMesh(QAbstract3DSeries::MeshCylinder);
    m_surfaceSeries->setBaseColor(Qt::cyan);
    m_surfaceSeries->setMeshSmooth(true);
    m_surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurface);

    m_surface->activeTheme()->setGridEnabled(true);
    m_surface->activeTheme()->setGridLineColor(Qt::gray);

    m_surface->setShadowQuality(QAbstract3DGraph::ShadowQualityMedium);

    m_surface->addSeries(m_surfaceSeries);
    m_surface->setVisible(true);

    m_surfaceWidget = QWidget::createWindowContainer(m_surface);
    ui->stackedWidget->addWidget(m_surfaceWidget);

    ui->stackedWidget->setCurrentWidget(m_chartView);

    ui->tBtnZoom->setIcon(QIcon(":/res/icons/zoom.png"));
    ui->tBtnZoom->setIconSize(QSize(16, 16));
    ui->tBtnZoom->setCheckable(true);
    ui->tBtnZoom->setChecked(m_autoZoom);
    ui->tBtnZoom->setToolTip("Auto Zoom");

    ui->tBtnData->setIcon(QIcon(":/res/icons/data.png"));
    ui->tBtnData->setIconSize(QSize(16, 16));
    ui->tBtnData->setCheckable(true);
    ui->tBtnData->setChecked(m_showData);
    ui->tBtnData->setToolTip("Data");

    ui->tBtn3D->setIcon(QIcon(":/res/icons/3d.png"));
    ui->tBtn3D->setIconSize(QSize(16, 16));
    ui->tBtn3D->setCheckable(true);
    ui->tBtn3D->setChecked(m_show3D);
    ui->tBtn3D->setToolTip("3D");

    m_workerThread = new QThread(this);
    m_worker = new PlotWorker();
    m_worker->moveToThread(m_workerThread);

    m_plotdata = new FormPlotData;
    m_plotdata->hide();
    connect(m_plotdata, &FormPlotData::windowClose, this, &FormPlot::plotDataClose);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &FormPlot::newDataReceived, m_worker, &PlotWorker::processData);
    connect(m_worker, &PlotWorker::dataReady, this, &FormPlot::updatePlot, Qt::QueuedConnection);
    connect(m_worker,
            &PlotWorker::pointsReady,
            m_plotdata,
            &FormPlotData::updateTable,
            Qt::QueuedConnection);
    m_workerThread->start();
}

void FormPlot::wheelEvent(QWheelEvent *event)
{
    if (!m_autoZoom && (event->modifiers() & Qt::ControlModifier)) {
        int delta = event->angleDelta().y();
        double factor = (delta > 0) ? 0.9 : 1.1;

        double center = (m_fixedYMin + m_fixedYMax) / 2.0;
        double range = (m_fixedYMax - m_fixedYMin) * factor / 2.0;

        m_fixedYMin = center - range;
        m_fixedYMax = center + range;

        m_axisY->setRange(m_fixedYMin, m_fixedYMax);
    } else {
        QWidget::wheelEvent(event);
    }
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

    QSurfaceDataArray *data = new QSurfaceDataArray;
    QSurfaceDataRow *surface_line = new QSurfaceDataRow;
    QSurfaceDataRow *surface_line_ = new QSurfaceDataRow;

    for (int i = 0; i < m_series->count(); ++i) {
        (*surface_line) << QVector3D(m_series->at(i).x(), m_series->at(i).y(), 0);
        (*surface_line_) << QVector3D(m_series->at(i).x(), m_series->at(i).y(), 0.1);
    }
    *data << surface_line << surface_line_;

    m_surfaceProxy->resetArray(data);
}

void FormPlot::on_tBtnZoom_clicked()
{
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);
}

void FormPlot::on_tBtnData_clicked()
{
    m_showData = !m_showData;
    m_plotdata->setVisible(m_showData);
}

void FormPlot::plotDataClose()
{
    m_showData = false;
    ui->tBtnData->setChecked(m_showData);
}

void FormPlot::on_tBtn3D_clicked()
{
    m_show3D = !m_show3D;
    if (m_show3D) {
        ui->stackedWidget->setCurrentWidget(m_surfaceWidget);
    } else {
        ui->stackedWidget->setCurrentWidget(m_chartView);
    }
}
