#include "formplot.h"
#include "ui_formplot.h"

#include <QLabel>
#include <QLegendMarker>
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
#include "funcdef.h"

FormPlot::FormPlot(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlot)
{
    ui->setupUi(this);
    init();
}

FormPlot::~FormPlot()
{
    if (m_plotData) {
        m_plotData->close();
        delete m_plotData;
    }
    if (m_plotHistory) {
        m_plotHistory->close();
        delete m_plotHistory;
    }
    if (m_plotSimulate) {
        m_plotSimulate->close();
        delete m_plotSimulate;
    }
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
    delete ui;
}

void FormPlot::getINI()
{
    int offset14 = SETTING_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET14, "0").toInt();
    int offset24 = SETTING_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET24, "0").toInt();
    if (offset14 != 0) {
        ui->spinBox14Offset->setValue(offset14);
    }
    if (offset24 != 0) {
        ui->spinBox24Offset->setValue(offset24);
    }

    ui->comboBoxAlgorithm->blockSignals(true);
    ui->comboBoxAlgorithm->addItems({"normal", "max_neg_95"});
    ui->comboBoxAlgorithm->blockSignals(false);

    int algorithm = SETTING_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
    ui->comboBoxAlgorithm->setCurrentIndex(algorithm);
}

void FormPlot::setINI()
{
    int offset14 = ui->spinBox14Offset->value();
    int offset24 = ui->spinBox24Offset->value();
    SETTING_SET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET14, QString::number(offset14));
    SETTING_SET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET24, QString::number(offset24));
}

void FormPlot::init()
{
    // 2D chart
    m_series24 = new QLineSeries();
    m_series14 = new QLineSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    m_chart->addSeries(m_series24);
    m_chart->addSeries(m_series14);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series24->attachAxis(m_axisX);
    m_series24->attachAxis(m_axisY);
    m_series14->attachAxis(m_axisX);
    m_series14->attachAxis(m_axisY);
    m_series24->setColor(Qt::blue);
    m_series14->setColor(Qt::magenta);

    m_axisX->setTitleText("Time (s)");
    m_axisX->setRange(0, 0.2);
    m_axisY->setTitleText("Voltage (V)");
    m_axisY->setRange(m_fixedYMin, m_fixedYMax);

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    for (QLegendMarker *marker : m_chart->legend()->markers()) {
        QObject::connect(marker, &QLegendMarker::clicked, [=]() {
            QAbstractSeries *series = marker->series();
            bool visible = series->isVisible();
            series->setVisible(!visible);
            marker->setVisible(true);
            marker->setLabelBrush(visible ? Qt::gray : Qt::black);
        });
    }
    m_chart->setTitle("Live ADC Waveform");

    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->stackedWidget->addWidget(m_chartView);

    // 3D Surface
    m_surface = new Q3DSurface();
    m_surfaceAxisX = new QValue3DAxis();
    m_surfaceAxisY = new QValue3DAxis();
    m_surfaceAxisZ = new QValue3DAxis();

    m_surfaceAxisX->setTitle("Time (s)");
    m_surfaceAxisY->setTitle("Voltage (V)");
    m_surfaceAxisZ->setTitle("Line Index");

    m_surfaceAxisX->setTitleVisible(true);
    m_surfaceAxisY->setTitleVisible(true);
    m_surfaceAxisZ->setTitleVisible(true);

    m_surface->setAxisX(m_surfaceAxisX);
    m_surface->setAxisY(m_surfaceAxisY);
    m_surface->setAxisZ(m_surfaceAxisZ);
    m_surface->setTitle("Live ADC Waveform");
    m_surface->activeTheme()->setGridEnabled(true);
    m_surface->activeTheme()->setGridLineColor(Qt::gray);
    m_surface->setShadowQuality(QAbstract3DGraph::ShadowQualityMedium);

    m_surfaceProxy24 = new QSurfaceDataProxy();
    m_surfaceProxy14 = new QSurfaceDataProxy();

    m_surfaceSeries24 = new QSurface3DSeries(m_surfaceProxy24);
    m_surfaceSeries14 = new QSurface3DSeries(m_surfaceProxy14);

    m_surfaceSeries24->setMesh(QAbstract3DSeries::MeshCylinder);
    m_surfaceSeries24->setBaseColor(Qt::cyan);
    m_surfaceSeries24->setDrawMode(QSurface3DSeries::DrawSurface);

    m_surfaceSeries14->setMesh(QAbstract3DSeries::MeshCylinder);
    m_surfaceSeries14->setBaseColor(Qt::magenta);
    m_surfaceSeries14->setDrawMode(QSurface3DSeries::DrawSurface);

    m_surface->addSeries(m_surfaceSeries24);
    m_surface->addSeries(m_surfaceSeries14);

    QSurfaceDataArray *initData = new QSurfaceDataArray;
    initData->reserve(1);
    QSurfaceDataRow *row = new QSurfaceDataRow;
    row->append(QSurfaceDataItem(QVector3D(0, 0, 0)));
    initData->append(row);
    m_surfaceProxy24->resetArray(initData);

    QSurfaceDataArray *initData1 = new QSurfaceDataArray;
    initData1->reserve(1);
    QSurfaceDataRow *row1 = new QSurfaceDataRow;
    row1->append(QSurfaceDataItem(QVector3D(0, 0, 0)));
    initData1->append(row1);
    m_surfaceProxy14->resetArray(initData1);

    m_surfaceWidget = QWidget::createWindowContainer(m_surface);
    m_surfaceWidget->setFocusPolicy(Qt::ClickFocus);
    ui->stackedWidget->addWidget(m_surfaceWidget);
    ui->stackedWidget->setCurrentWidget(m_chartView);

    // tool button
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

    ui->tBtnHistory->setIcon(QIcon(":/res/icons/history.png"));
    ui->tBtnHistory->setIconSize(QSize(16, 16));
    ui->tBtnHistory->setCheckable(true);
    ui->tBtnHistory->setChecked(m_showHistory);
    ui->tBtnHistory->setToolTip("History");

    ui->tBtnSimulate->setIcon(QIcon(":/res/icons/simulate.png"));
    ui->tBtnSimulate->setIconSize(QSize(16, 16));
    ui->tBtnSimulate->setCheckable(true);
    ui->tBtnSimulate->setChecked(m_showSimulate);
    ui->tBtnSimulate->setToolTip("Simulate");

    // thread worker
    m_workerThread = new QThread(this);
    m_worker = new PlotWorker();
    m_worker->moveToThread(m_workerThread);

    m_plotData = new FormPlotData;
    m_plotData->hide();

    m_plotHistory = new FormPlotHistory;
    m_plotHistory->hide();

    m_plotSimulate = new FormPlotSimulate;
    m_plotSimulate->hide();

    connect(m_plotData, &FormPlotData::windowClose, this, &FormPlot::plotDataClose);
    connect(m_plotHistory, &FormPlotHistory::windowClose, this, &FormPlot::plotHistoryClose);
    connect(m_plotSimulate, &FormPlotSimulate::windowClose, this, &FormPlot::plotSimulateClose);
    connect(m_plotSimulate,
            &FormPlotSimulate::simulateDataReady,
            m_worker,
            &PlotWorker::processData);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &FormPlot::newDataReceived, m_worker, &PlotWorker::processData);
    connect(m_worker, &PlotWorker::dataReady, this, &FormPlot::updatePlot, Qt::QueuedConnection);
    connect(m_worker,
            &PlotWorker::pointsReady,
            m_plotData,
            &FormPlotData::updateTable,
            Qt::QueuedConnection);

    m_workerThread->start();
    getINI();
}

void FormPlot::onDataReceived(const QByteArray &data, const QString &name)
{
    emit newDataReceived(data, name);
}

void FormPlot::updatePlot(const QString &name,
                          QLineSeries *line,
                          const int &points,
                          const double &min_y,
                          const double &max_y,
                          const double &min_x,
                          const double &max_x)
{
    if (name == "curve_24bit") {
        m_series24->replace(line->points());
        m_series24->setName(name);
        m_points24.push_back(line->points());
    } else if (name == "curve_14bit") {
        m_series14->replace(line->points());
        m_series14->setName(name);
        m_points14.push_back(line->points());
    }

    m_axisX->setRange(min_x, max_x);

    if (m_autoZoom) {
        double padding = (max_y - min_y) * 0.1;
        if (padding == 0) {
            padding = 0.1;
        }
        m_axisY->setRange(min_y - padding, max_y + padding);
    } else {
        m_axisY->setRange(m_fixedYMin, m_fixedYMax);
    }

    // Update 3D
    auto toSurfaceArray = [](const QLineSeries *series, float z) -> QSurfaceDataArray * {
        QSurfaceDataArray *data = new QSurfaceDataArray;

        QSurfaceDataRow *surface_line = new QSurfaceDataRow;
        QSurfaceDataRow *surface_line_ = new QSurfaceDataRow;

        for (int i = 0; i < series->count(); ++i) {
            (*surface_line) << QVector3D(series->at(i).x(), series->at(i).y(), z);
            (*surface_line_) << QVector3D(series->at(i).x(), series->at(i).y(), z + 0.1);
        }
        *data << surface_line << surface_line_;
        return data;
    };

    m_surfaceProxy24->resetArray(toSurfaceArray(m_series24, 0));
    m_surfaceProxy14->resetArray(toSurfaceArray(m_series14, 0));
}

void FormPlot::wheelEvent(QWheelEvent *event)
{
    if (m_autoZoom) {
        on_tBtnZoom_clicked();
    }
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

void FormPlot::on_tBtnZoom_clicked()
{
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);
}

void FormPlot::on_tBtnData_clicked()
{
    m_showData = !m_showData;
    m_plotData->setVisible(m_showData);
}

void FormPlot::plotDataClose()
{
    m_showData = false;
    ui->tBtnData->setChecked(false);
}

void FormPlot::plotHistoryClose()
{
    m_showHistory = false;
    ui->tBtnHistory->setChecked(false);
}

void FormPlot::plotSimulateClose()
{
    m_showSimulate = false;
    ui->tBtnSimulate->setChecked(false);
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

void FormPlot::on_tBtnHistory_clicked()
{
    m_showHistory = !m_showHistory;
    if (m_showHistory) {
        m_plotHistory->updateData(m_points14, m_points24);
        m_points14.clear();
        m_points24.clear();
    }
    m_plotHistory->setVisible(m_showHistory);
}

void FormPlot::on_tBtnSimulate_clicked()
{
    m_showSimulate = !m_showSimulate;
    m_plotSimulate->setVisible(m_showSimulate);
}

void FormPlot::on_spinBox14Offset_valueChanged(int val)
{
    m_worker->setOffset14(val);
    setINI();
}

void FormPlot::on_spinBox24Offset_valueChanged(int val)
{
    m_worker->setOffset24(val);
    setINI();
}

void FormPlot::on_comboBoxAlgorithm_currentIndexChanged(int index)
{
    if (index == static_cast<int>(PLOT_ALGORITHM::NORMAL)) {
        m_worker->setAlgorithm(index);
    } else if (index == static_cast<int>(PLOT_ALGORITHM::MAX_NEG_95)) {
        m_worker->setAlgorithm(index);
    }
    SETTING_SET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, QString::number(index));
}
