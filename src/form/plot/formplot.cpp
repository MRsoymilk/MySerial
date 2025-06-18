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

void FormPlot::retranslateUI()
{
    ui->retranslateUi(this);
    m_plotData->retranslateUI();
    m_plotHistory->retranslateUI();
    m_plotData->retranslateUI();
}

void FormPlot::getINI()
{
    int offset14 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET14, "0").toInt();
    int offset24 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET24, "0").toInt();
    if (offset14 != 0) {
        ui->spinBox14Offset->setValue(offset14);
    }
    if (offset24 != 0) {
        ui->spinBox24Offset->setValue(offset24);
    }

    ui->comboBoxAlgorithm->blockSignals(true);
    ui->comboBoxAlgorithm->addItems({"normal", "max_neg_95"});
    ui->comboBoxAlgorithm->blockSignals(false);

    int algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
    ui->comboBoxAlgorithm->setCurrentIndex(algorithm);
}

void FormPlot::setINI()
{
    int offset14 = ui->spinBox14Offset->value();
    int offset24 = ui->spinBox24Offset->value();
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET14, QString::number(offset14));
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET24, QString::number(offset24));
}

void FormPlot::init2d()
{
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
}

void FormPlot::init3d()
{
    m_glWidget = new MyGLCurveWidget();
    ui->stackedWidget->addWidget(m_glWidget);
    ui->stackedWidget->setCurrentWidget(m_chartView);
}

void FormPlot::initToolButtons()
{
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
}

void FormPlot::init()
{
    init2d();
    init3d();
    initToolButtons();

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
            &FormPlotSimulate::simulateDataReady4k,
            m_worker,
            &PlotWorker::processData4k);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &FormPlot::newDataReceived4k, m_worker, &PlotWorker::processData4k);
    connect(m_worker, &PlotWorker::dataReady4k, this, &FormPlot::updatePlot4k, Qt::QueuedConnection);
    connect(m_worker,
            &PlotWorker::pointsReady4k,
            m_plotData,
            &FormPlotData::updateTable4k,
            Qt::QueuedConnection);

    m_workerThread->start();
    getINI();
}

void FormPlot::onDataReceived4k(const QByteArray &data14, const QByteArray &data24)
{
    emit newDataReceived4k(data14, data24);
}

void FormPlot::updatePlot2d(const QList<QPointF> &data14,
                            const QList<QPointF> &data24,
                            const double &xMin,
                            const double &xMax,
                            const double &yMin,
                            const double &yMax)
{
    m_series14->replace(data14);
    m_series14->setName("curve14_bit");
    m_points14.push_back(data14);
    m_series24->replace(data24);
    m_series24->setName("curve24_bit");
    m_points24.push_back(data24);

    m_axisX->setRange(xMin, xMax);
    if (m_autoZoom) {
        double padding = (yMax - yMin) * 0.1;
        if (padding == 0) {
            padding = 0.1;
        }
        m_axisY->setRange(yMin - padding, yMax + padding);
    } else {
        m_axisY->setRange(m_fixedYMin, m_fixedYMax);
    }
}

void FormPlot::updatePlot3d(const QList<QPointF> &data14,
                            const QList<QPointF> &data24,
                            const double &xMin,
                            const double &xMax,
                            const double &yMin,
                            const double &yMax)
{
    QVector<CurveData> curves;
    CurveData curve14;
    CurveData curve24;
    curve14.color = Qt::magenta;
    curve24.color = Qt::blue;
    auto to3dCurve = [&](const QLineSeries *series, CurveData &curve) {
        for (int i = 0; i < series->count(); ++i) {
            curve.points.append(QVector2D(series->at(i).x(), series->at(i).y()));
        }
    };
    to3dCurve(m_series14, curve14);
    to3dCurve(m_series24, curve24);
    curves.append(curve14);
    curves.append(curve24);
    m_glWidget->setCurves(curves);
}

void FormPlot::updatePlot4k(const QList<QPointF> &data14,
                            const QList<QPointF> &data24,
                            const double &xMin,
                            const double &xMax,
                            const double &yMin,
                            const double &yMax)
{
    updatePlot2d(data14, data24, xMin, xMax, yMin, yMax);
    updatePlot3d(data14, data24, xMin, xMax, yMin, yMax);
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
        ui->stackedWidget->setCurrentWidget(m_glWidget);
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
    m_worker->setAlgorithm(index);
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, QString::number(index));
}
