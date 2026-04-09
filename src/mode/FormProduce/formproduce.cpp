#include "formproduce.h"

#include <QThread>

#include "../LoadingOverLay/loadingoverlay.h"
#include "../ThreadWorker/threadworker.h"
#include "../form/FormPlotCorrection/fitting/formfittingpoints.h"
#include "../form/serial/formserial.h"
#include "../form/plot/PeakTrajectory/peaktrajectory.h"
#include "../form/FormPlotSimulate/formplotsimulate.h"
#include "DraggableLine/draggableline.h"
#include "MyChartView/mychartview.h"
#include "funcdef.h"
#include "ui_formproduce.h"
#include "findpeak.h"
#include "peakcfg.h"

FormProduce::FormProduce(QWidget *parent) : QWidget(parent), ui(new Ui::FormProduce) {
    ui->setupUi(this);
    init();
}

FormProduce::~FormProduce() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
    delete ui;
}

void FormProduce::retranslateUI() {
    ui->retranslateUi(this);
    if (m_formFittingPoints) {
        m_formFittingPoints->retranslateUI();
    }
    for (int i = 0; i < m_tabLabels.size(); ++i) {
        m_tabLabels[i]->setText(tr("is done"));
    }
}

void FormProduce::setAlgorithm(const QString &algorithm) {
    formSerial->onChangeFrameType(algorithm);
    m_worker->setAlgorithm(algorithm);
}

void FormProduce::updatePlot4k(const MY_DATA &my_data, bool record) {
    if (m_pause) {
        return;
    }

    CURVE plot31 = my_data.curve31;
    CURVE plot33 = my_data.curve33;
    if (m_enableVoltage) {
        updatePlot2d(plot31.data, plot33.data);
    } else {
        updatePlot2d(plot31.raw.data, plot33.raw.data);
    }

    if(m_enablePeak) {
        callFindPeak();
    }
}

void FormProduce::callFindPeak() {
    if (m_enablePeak) {
        if (!m_series31 || m_series31->count() < 5) {
            return;
        }

        auto cfg = m_peakCfg->getCfg();
        auto peaks31 = FindPeak::find(m_series31, cfg[0], cfg[1], cfg[2]);
        peakTrajectory(peaks31);
        m_peaks->clear();
        for (const auto &pt : peaks31) {
            m_peaks->append(pt);
        }
        m_chart->update();
    }
}

void FormProduce::peakTrajectory(const QVector<QPointF> &peaks) {
    if (peaks.isEmpty() || !m_series31) return;

    // 找到 peaks 中 Y 最大的点
    QPointF maxPeak = QPointF(0, std::numeric_limits<float>::lowest());
    for (const QPointF &p : peaks) {
        if (p.x() < m_trajectory_start || p.x() > m_trajectory_end) {
            continue;
        }
        if (p.y() > maxPeak.y()) {
            maxPeak = p;
        }
    }

    int xPeak = maxPeak.x();
    double y = 0;
    if (m_series33->count() > xPeak) {
        y = m_series33->at(xPeak).y();
    }
    if (m_enableVoltage) {
        y = y * 0x8000 / 3.3;
    }
    if (m_trajectory) {
        m_trajectory->appendPeak(y);
    }
}

void FormProduce::updatePlot2d(const QList<QPointF> &data31, const QList<QPointF> &data33) {
    static bool flip = false;
    flip = !flip;
    if (flip) {
        ui->labelUpdateSign->setStyleSheet("background-color: green; color: white;");
    } else {
        ui->labelUpdateSign->setStyleSheet("background-color: blue; color: white;");
    }
    m_series31->replace(data31);
    m_series33->replace(data33);
    updateAxis();
}

void FormProduce::updateAxis() {
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::min();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::min();

    for (int i = 0; i < m_series31->count(); ++i) {
        xMin = std::min(xMin, m_series31->at(i).x());
        xMax = std::max(xMax, m_series31->at(i).x());
        yMin = std::min(yMin, m_series31->at(i).y());
        yMax = std::max(yMax, m_series31->at(i).y());
    }
    for (int i = 0; i < m_series33->count(); ++i) {
        xMin = std::min(xMin, m_series31->at(i).x());
        xMax = std::max(xMax, m_series31->at(i).x());
        yMin = std::min(yMin, m_series33->at(i).y());
        yMax = std::max(yMax, m_series33->at(i).y());
    }
    double padding = (yMax - yMin) * 0.1;
    if (padding == 0) {
        padding = 0.1;
    }
    m_axisX->setRange(xMin, xMax);
    m_axisY->setRange(yMin - padding, yMax + padding);
}

void FormProduce::closeEvent(QCloseEvent *event) {
    m_peakCfg->close();
    m_trajectory->close();
    m_formFittingPoints->close();
    formSerial->stopFSeriesConnect();
}

void FormProduce::initTabUI() {
    auto tabBar = ui->tabWidget->tabBar();
    int count = tabBar->count();

    m_tabWidgets.resize(count);
    m_tabLabels.resize(count);

    for (int i = 0; i < count; ++i) {
        QWidget *w = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(w);
        layout->setContentsMargins(6, 2, 6, 2);

        QLabel *label = new QLabel(tr("is done"));
        label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        label->setMinimumWidth(60);
        layout->addWidget(label);
        w->setLayout(layout);

        tabBar->setTabButton(i, QTabBar::LeftSide, w);

        m_tabWidgets[i] = w;
        m_tabLabels[i] = label;
    }
}

void FormProduce::init() {
    initTabUI();
    makeTabTodo();

    m_series31 = new QLineSeries();
    m_series31->setName(tr("curve31"));

    m_series33 = new QLineSeries();
    m_series33->setName(tr("curve33"));

    m_series31->setColor(Qt::blue);
    m_series33->setColor(Qt::magenta);

    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_chart = new QChart();
    m_chart->addSeries(m_series31);
    m_chart->addSeries(m_series33);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series31->attachAxis(m_axisX);
    m_series31->attachAxis(m_axisY);
    m_series33->attachAxis(m_axisX);
    m_series33->attachAxis(m_axisY);
    m_axisX->setTitleText(tr("index"));
    m_axisY->setTitleText(tr("intensity"));
    m_chart->setTitle(tr("Spectral"));
    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayChart->addWidget(m_chartView);

    m_peakCfg = new PeakCfg;
    m_peaks = new QScatterSeries();
    m_chart->addSeries(m_peaks);
    m_peaks->attachAxis(m_axisX);
    m_peaks->attachAxis(m_axisY);
    m_peaks->setColor(Qt::red);
    m_peaks->setName(tr("Peaks"));
    m_peaks->setMarkerSize(5.0);
    m_peaks->setPointLabelsVisible(true);
    m_peaks->setPointLabelsClipping(false);
    m_peaks->setPointLabelsColor(Qt::red);
    m_peaks->setPointLabelsFont(QFont("Arial", 10, QFont::Bold));
    m_peaks->setPointLabelsFormat("(@xPoint, @yPoint)");
    m_peaks->setVisible(false);
    m_trajectory = new PeakTrajectory;
    m_plotSimulate = new FormPlotSimulate;
    m_plotSimulate->hide();

    m_formFittingPoints = new FormFittingPoints;
    connect(m_formFittingPoints, &FormFittingPoints::windowClose, this, [&]() {
        m_enableFitting = false;
        m_formFittingPoints->setVisible(false);
    });
    connect(m_formFittingPoints, &FormFittingPoints::doFile, m_plotSimulate, &FormPlotSimulate::onDoFile);
    connect(m_trajectory, &PeakTrajectory::broadcast, m_formFittingPoints, [&](const double &avg) {
        m_formFittingPoints->setTargetIntensity(avg);
    });
    formSerial = new FormSerial;
    m_workerThread = new QThread(this);
    m_worker = new ThreadWorker();
    m_worker->moveToThread(m_workerThread);
    connect(m_formFittingPoints, &FormFittingPoints::toCollectionFittingPoints, m_worker,
            &ThreadWorker::onCollectionFittingPoints, Qt::QueuedConnection);
    connect(m_plotSimulate, &FormPlotSimulate::simulateDataReady, formSerial, &FormSerial::onSimulateRecv,
            Qt::QueuedConnection);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();

    connect(m_worker, &ThreadWorker::collectionFitingPointsFinish, this,
            [this](){
                m_formFittingPoints->updateCollectionStatus(true);
    }, Qt::QueuedConnection);
    QObject::connect(formSerial, &FormSerial::recv2PlotF30, m_worker, &ThreadWorker::processDataF30,
                     Qt::QueuedConnection);
    connect(m_worker, &ThreadWorker::plotReady4k, this, &FormProduce::updatePlot4k, Qt::QueuedConnection);

    m_overlay = new LoadingOverLay(this);
    m_overlay->hide();
    connect(m_overlay, &LoadingOverLay::stopConnect, this, [=]() {
        LOG_INFO("connect stopped by user");
        formSerial->stopFSeriesConnect();
        m_isPlaying = false;
        ui->tBtnSwitch->setChecked(false);
        m_overlay->hide();
    });
    connect(formSerial, &FormSerial::connectProduceModeEstablished, this, [=]() {
        m_isPlaying = true;
        ui->tBtnSwitch->setChecked(true);
        m_overlay->hide();
    });

    connect(formSerial, &FormSerial::redoConnect, this, [=]() {
        formSerial->startProduceConnect();
        m_overlay->reTry();
    });
    connect(formSerial, &FormSerial::statusReport, m_overlay, &LoadingOverLay::updateInfo, Qt::QueuedConnection);
    connect(formSerial, &FormSerial::optReturn, this, &FormProduce::onOptReturn);
    formSerial->initProduceConnect();
}

void FormProduce::on_tBtnSwitch_clicked() {
    m_isPlaying = !m_isPlaying;
    if (m_isPlaying) {
        if (connectProduceMode()) {
            m_isPlaying = true;
            ui->tBtnSwitch->setChecked(true);
            makeTabTodo();
        } else {
            m_isPlaying = false;
            ui->tBtnSwitch->setChecked(false);
        }
    } else {
        closeProduceMode();
    }
}

bool FormProduce::connectProduceMode() {
    m_overlay->resize(this->size());
    m_overlay->show();
    m_overlay->updateTry(1);

    QString algorithm = qApp->property("algorithm").toString();
    formSerial->updateFrameTypes(algorithm);
    m_worker->setAlgorithm(algorithm);

    formSerial->startProduceConnect();
    return true;
}

void FormProduce::closeProduceMode() {
    m_isPlaying = false;
    ui->tBtnSwitch->setChecked(false);
    formSerial->stopFSeriesConnect();
}

void FormProduce::onOptReturn(int id, const QString& msg) {
    if(id == PRODUCE_QUERY_DEVICE_SERIAL) {
        ui->labelValDeviceSerial->setText(msg);
    }
    else if(id == PRODUCE_QUERY_BASEINE) {
        ui->labelValBaseline->setText(msg);
    }
    else if(id == PRODUCE_SELF_CHECK) {
        ui->textBrowserSelfCheckInfo->append(msg);
    }
}

void FormProduce::on_btnWriteDeviceSerial_clicked() {
    QString data = ui->lineEditDeviceSerial->text();
    QString msg = QString("%1%2%3").arg("DD3C000920").arg(data).arg("CDFF");
    formSerial->doProduceOpt(PRODUCE_WRITE_DEVICE_SERIAL, msg);
}

void FormProduce::on_btnQueryDeviceSerial_clicked() {
    formSerial->doProduceOpt(PRODUCE_QUERY_DEVICE_SERIAL);
}

void FormProduce::on_btnWriteBaseline_clicked() {
    QString baseline = ui->lineEditBaseline->text();
    int val = baseline.toInt();
    QString byte_val = QString("%1")
                           .arg(static_cast<quint64>(val),
                                6,   // 6 bytes = 12 hex chars
                                16,  // base 16
                                QChar('0'))
                           .toUpper();
    QString msg = QString("%1%2%3").arg("DD3C000644").arg(byte_val).arg("CDFF");
    formSerial->doProduceOpt(PRODUCE_WRITE_BASEINE, msg);
}

void FormProduce::on_btnQueryBaseline_clicked() {
    formSerial->doProduceOpt(PRODUCE_QUERY_BASEINE);
}

void FormProduce::updateTabStyle() {
    for (int i = 0; i < m_tabWidgets.size(); ++i) {
        if (m_tabDone[i]) {
            m_tabWidgets[i]->setStyleSheet("background: green;");
        } else {
            m_tabWidgets[i]->setStyleSheet("background: red;");
        }

        m_tabLabels[i]->setStyleSheet("color: white;");
    }
}

void FormProduce::makeTabTodo() {
    int count = ui->tabWidget->count();
    m_tabDone = QVector<bool>(count, false);

    updateTabStyle();
}

void FormProduce::on_tBtnDoneDeviceSerial_clicked() {
    int idx = ui->tabWidget->indexOf(ui->tabDevicenfo);
    m_tabDone[idx] = true;

    updateTabStyle();
    ui->tabWidget->setCurrentWidget(ui->tabBaseline);
}

void FormProduce::on_tBtnDoneBaseline_clicked() {
    int idx = ui->tabWidget->indexOf(ui->tabBaseline);
    m_tabDone[idx] = true;

    updateTabStyle();
    ui->tabWidget->setCurrentWidget(ui->tabCorrection);
}

void FormProduce::on_tBtnDoneCorrection_clicked() {
    int idx = ui->tabWidget->indexOf(ui->tabCorrection);
    m_tabDone[idx] = true;

    updateTabStyle();
    ui->tabWidget->setCurrentWidget(ui->tabSelfCheck);
}

void FormProduce::on_tBtnDoneSelfCheck_clicked() {
    int idx = ui->tabWidget->indexOf(ui->tabSelfCheck);
    m_tabDone[idx] = true;

    updateTabStyle();

    QMessageBox::information(this, TITLE_INFO, tr("Job Done!"));
}

void FormProduce::on_btnStartCorrection_clicked() {
    m_enableFitting = !m_enableFitting;
    m_formFittingPoints->setVisible(m_enableFitting);
}

void FormProduce::on_btnStartSelfCheck_clicked() {
    formSerial->doProduceOpt(PRODUCE_SELF_CHECK);
}

void FormProduce::on_tBtnToVoltage_clicked() {
    m_enableVoltage = !m_enableVoltage;
    ui->tBtnToVoltage->setChecked(m_enableVoltage);
}

void FormProduce::on_tBtnPause_clicked() {
    m_pause = !m_pause;
    ui->tBtnPause->setChecked(m_pause);
}

void FormProduce::on_checkBoxTrackPeak_checkStateChanged(const Qt::CheckState &state)
{
    if(state == Qt::Checked) {
        m_enablePeak = true;
        m_trajectory_start = m_axisX->min();
        m_trajectory_end = m_axisX->max();
        QChart *chart = m_chartView->chart();
        QRectF plot = chart->plotArea();

        qreal leftX = plot.left();
        qreal rightX = plot.right();
        m_lineLeft = new DraggableLine(chart, leftX, Qt::green);
        m_lineRight = new DraggableLine(chart, rightX, Qt::darkGreen);
        connect(m_lineLeft, &DraggableLine::xValueChanged, this, [this](qreal x) { m_trajectory_start = x; });
        connect(m_lineRight, &DraggableLine::xValueChanged, this, [this](qreal x) { m_trajectory_end = x; });

        chart->scene()->addItem(m_lineLeft);
        chart->scene()->addItem(m_lineRight);

        callFindPeak();
    }
    else {
        m_enablePeak = false;
        m_peaks->setVisible(false);
        m_trajectory->hide();
        if (m_lineLeft) {
            m_chartView->chart()->scene()->removeItem(m_lineLeft);
            delete m_lineLeft;
            m_lineLeft = nullptr;
        }
        if (m_lineRight) {
            m_chartView->chart()->scene()->removeItem(m_lineRight);
            delete m_lineRight;
            m_lineRight = nullptr;
        }
    }
    m_peaks->setVisible(m_enablePeak);
    m_peakCfg->setVisible(m_enablePeak);
    m_trajectory->setVisible(m_enablePeak);
}

void FormProduce::on_btnWriteThreshold_clicked()
{

}

