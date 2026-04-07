#include "formeasy.h"

#include <QMovie>

#include "../LoadingOverLay/loadingoverlay.h"
#include "../ThreadWorker/threadworker.h"
#include "../form/FormPlotHistory/formplothistory.h"
#include "../form/FormPlotSimulate/formplotsimulate.h"
#include "../form/plot/Accumulate/accumulate.h"
#include "../form/plot/DarkSpectrum/darkspectrum.h"
#include "../form/plot/Derivation/derivation.h"
#include "../form/plot/FourierTransform/fouriertransform.h"
#include "../form/plot/PeakTrajectory/peaktrajectory.h"
#include "../form/plot/PointsTracker/pointstracker.h"
#include "../form/plot/SignalNoiseRatio/signalnoiseratio.h"
#include "../form/serial/formserial.h"
#include "DraggableLine/draggableline.h"
#include "HoverSliderButton/hoversliderbutton.h"
#include "MyChartView/mychartview.h"
#include "funcdef.h"
#include "ui_formeasy.h"
#include "findpeak.h"
#include "peakcfg.h"

FormEasy::FormEasy(QWidget *parent) : QWidget(parent), ui(new Ui::FormEasy) {
    ui->setupUi(this);
    init();
}

FormEasy::~FormEasy() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
    delete ui;
}

void FormEasy::retranslateUI() { ui->retranslateUi(this); }

void FormEasy::setAlgorithm(const QString &algorithm) {
    formSerial->onChangeFrameType(algorithm);
    m_worker->setAlgorithm(algorithm);
}

bool FormEasy::connectEasyMode() {
    m_overlay->resize(this->size());
    m_overlay->updateTry(1);
    m_overlay->show();

    QString algorithm = qApp->property("algorithm").toString();
    formSerial->updateFrameTypes(algorithm);
    m_worker->setAlgorithm(algorithm);
    m_F30_shown_mode = SETTING_CONFIG_GET(CFG_GROUP_F30_SHOWN, CFG_F30_SHOWN_MODE, CFG_F30_MODE_DOUBLE);
    emit initThreshold();
    formSerial->startEasyConnect(m_F30_shown_mode);
    return true;
}

void FormEasy::closeEasyMode() {
    m_isPlaying = false;
    ui->tBtnSwitch->setChecked(false);
    formSerial->stopFSeriesConnect();
}

void FormEasy::on_tBtnSwitch_clicked() {
    m_isPlaying = !m_isPlaying;
    if (m_isPlaying) {
        if (connectEasyMode()) {
            m_isPlaying = true;
            ui->tBtnSwitch->setChecked(true);
        } else {
            m_isPlaying = false;
            ui->tBtnSwitch->setChecked(false);
        }
    } else {
        closeEasyMode();
    }
}

void FormEasy::initAxisControl() {
    int x_start = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_START, "900").toInt();
    int x_end = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_END, "1700").toInt();
    int y_start = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_START, "0").toInt();
    int y_end = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_END, "65535").toInt();
    ui->spinBoxXStart->setValue(x_start);
    ui->spinBoxXEnd->setValue(x_end);
    ui->spinBoxYStart->setValue(y_start);
    ui->spinBoxYEnd->setValue(y_end);

    const QString val_x_enable = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_ENABLE, VAL_DISABLE);
    if (val_x_enable == VAL_ENABLE) {
        m_enableAxisX = true;
        ui->tBtnAxisX->setChecked(true);
    }
    const QString val_y_enable = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_ENABLE, VAL_DISABLE);
    if (val_y_enable == VAL_ENABLE) {
        m_enableAxisY = true;
        ui->tBtnAxisY->setChecked(true);
        m_autoZoom = false;
        ui->tBtnZoom->setChecked(false);
    }
}

void FormEasy::initChart() {
    m_line = new QLineSeries();
    m_line->setName(tr("curve"));

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
    m_chart->setTitle(tr("Spectral"));
    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(m_chartView);
}

void FormEasy::initConnectInfo() {
    ui->comboBoxTimeUnit->blockSignals(true);
    ui->comboBoxTimeUnit->addItems({"ms", "s"});
    ui->comboBoxTimeUnit->setCurrentIndex(0);
    ui->comboBoxTimeUnit->blockSignals(false);
    ui->spinBoxIntegrationTime->setValue(5);
}

void FormEasy::initTable() {
    m_modelValue = new QStandardItemModel(this);
    m_modelValue->setColumnCount(3);
    m_modelValue->setHeaderData(0, Qt::Horizontal, tr("index"));
    m_modelValue->setHeaderData(1, Qt::Horizontal, tr("intensity"));
    m_modelValue->setHeaderData(2, Qt::Horizontal, tr("raw"));

    ui->tableViewValue->setModel(m_modelValue);
    ui->tableViewValue->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewValue->verticalHeader()->setVisible(false);
    ui->tableViewValue->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewValue->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void FormEasy::initToolButton() {
    ui->tBtnSwitch->setToolTip(tr("switch"));
    connect(ui->tBtnSwitch, &HoverTbtnButton::buttonClicked, this, [this](int id, bool status) {
        switch (id) {
            case HoverTbtnButton::BTN_HANDSHAKE:
                formSerial->doEasyOpt(EASY_HANDSHAKE);
                break;
            case HoverTbtnButton::BTN_SET_INTEGRATION_TIME:
                formSerial->doEasyOpt(EASY_SET_INTEGRATION_TIME);
                break;
            case HoverTbtnButton::BTN_DO_THRESHOLD:
                formSerial->doEasyOpt(EASY_DO_THRESHOLD);
                break;
            case HoverTbtnButton::BTN_DO_BASELINE:
                formSerial->doEasyOpt(EASY_DO_BASELINE);
                break;
            case HoverTbtnButton::BTN_DATA_REQUEST:
                formSerial->doEasyOpt(EASY_DATA_REQUEST, m_F30_shown_mode);
                break;
            case HoverTbtnButton::BTN_STOP:
                formSerial->doEasyOpt(EASY_DISCONNECT);
                break;
            case HoverTbtnButton::BTN_CONNECT:
                if(!status) {
                    formSerial->doEasyOpt(EASY_CONNECT, ui->tBtnSwitch->getCurrentPort());
                }
                else {
                    formSerial->doEasyOpt(EASY_DISCONNECT, ui->tBtnSwitch->getCurrentPort());
                }
                break;
            default:
                break;
        }
    });
    connect(ui->tBtnSwitch, &HoverTbtnButton::buttonHover, this,
            [this]() { ui->tBtnSwitch->updatePorts(formSerial->getPorts()); });
    ui->tBtnPause->setToolTip(tr("pause"));
    ui->tBtnZoom->setToolTip(tr("zoom"));
    ui->tBtnCrop->setToolTip(tr("crop"));
    ui->tBtnPeak->setToolTip(tr("find peak"));
    ui->tBtnFWHM->setToolTip(tr("FWHM"));
    ui->tBtnImg->setToolTip(tr("save image"));
    ui->tBtnFourier->setToolTip(tr("fourier"));
    connect(ui->tBtnFourier, &HoverSliderButton::valueChanged, this, [this](int v) {
        m_fourierTransform->setPercent(v);
        if (v != 0) {
            ui->tBtnFourier->setChecked(true);
            m_enableFourierPercent = true;
        } else {
            ui->tBtnFourier->setChecked(false);
            m_enableFourierPercent = false;
        }
    });
    ui->tBtnAccumulate->setToolTip(tr("accumulate"));
    ui->tBtnToVoltage->setToolTip(tr("to voltage"));

    ui->tBtnZoom->setChecked(true);

    m_infoPopup = new QFrame(nullptr, Qt::Popup);
    m_infoPopup->setFrameShape(QFrame::Box);

    QVBoxLayout *layout = new QVBoxLayout(m_infoPopup);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(2);

    m_tBtnSimulate = new QToolButton;
    m_tBtnSimulate->setObjectName("tBtnSimulate");
    m_tBtnSimulate->setToolTip(tr("simulate"));
    m_tBtnSimulate->setCheckable(true);
    connect(m_tBtnSimulate, &QToolButton::clicked, this, &FormEasy::doSimulateClicked);

    m_tBtnHistory = new QToolButton;
    m_tBtnHistory->setObjectName("tBtnHistory");
    m_tBtnHistory->setToolTip(tr("history"));
    m_tBtnHistory->setCheckable(true);
    connect(m_tBtnHistory, &QToolButton::clicked, this, &FormEasy::doHistoryClicked);

    m_tBtnSNR = new QToolButton;
    m_tBtnSNR->setObjectName("tBtnSNR");
    m_tBtnSNR->setToolTip(tr("signal noise ratio"));
    m_tBtnSNR->setCheckable(true);
    connect(m_tBtnSNR, &QToolButton::clicked, this, &FormEasy::doSNRClicked);

    m_tBtnPointsTracker = new QToolButton;
    m_tBtnPointsTracker->setObjectName("tBtnPointsTracker");
    m_tBtnPointsTracker->setToolTip(tr("Points Tracker"));
    m_tBtnPointsTracker->setCheckable(true);
    connect(m_tBtnPointsTracker, &QToolButton::clicked, this, &FormEasy::doPointsTracker);

    m_tBtnDarkSpectrum = new QToolButton;
    m_tBtnDarkSpectrum->setObjectName("tBtnDarkSpectrum");
    m_tBtnDarkSpectrum->setToolTip(tr("Dark Spectrum"));
    m_tBtnDarkSpectrum->setCheckable(true);
    connect(m_tBtnDarkSpectrum, &QToolButton::clicked, this, &FormEasy::doDarkSpectrum);

    connect(m_plotSimulate, &FormPlotSimulate::windowClose, this, [=]() {
        m_enableSimulate = false;
        m_tBtnSimulate->setChecked(false);
    });
    connect(m_plotHistory, &FormPlotHistory::windowClose, this, [=]() {
        m_enableHistory = false;
        m_tBtnHistory->setChecked(false);
    });
    connect(m_snr, &SignalNoiseRatio::windowClose, this, [=]() {
        m_enableSNR = false;
        m_tBtnSNR->setChecked(false);
    });
    connect(m_pointsTracker, &PointsTracker::windowClose, this, [=]() {
        m_enablePointsTracker = false;
        m_tBtnPointsTracker->setChecked(false);
    });
    connect(m_darkSpectrum, &DarkSpectrum::windowClose, this, [=]() {
        m_enableDarkSpectrum = false;
        m_tBtnDarkSpectrum->setChecked(false);
    });
    connect(m_darkSpectrum, &DarkSpectrum::doCalculate, this, [=](bool status) { m_doDarkSpectrumCalc = status; });
    layout->addWidget(m_tBtnSimulate);
    layout->addWidget(m_tBtnHistory);
    layout->addWidget(m_tBtnSNR);
    layout->addWidget(m_tBtnDarkSpectrum);
    layout->addWidget(m_tBtnPointsTracker);
}

void FormEasy::init() {
    m_trajectory = new PeakTrajectory;

    formSerial = new FormSerial;
    m_workerThread = new QThread(this);
    m_worker = new ThreadWorker();
    m_worker->moveToThread(m_workerThread);

    m_plotSimulate = new FormPlotSimulate;
    m_plotSimulate->hide();

    m_plotHistory = new FormPlotHistory;
    m_plotHistory->hide();

    m_fourierTransform = new FourierTransform;
    m_fourierTransform->hide();

    m_derivation = new Derivation;
    m_derivation->hide();

    m_snr = new SignalNoiseRatio;
    m_snr->hide();

    m_accumulate = new Accumulate;
    m_accumulate->hide();

    m_pointsTracker = new PointsTracker;
    m_pointsTracker->hide();

    m_darkSpectrum = new DarkSpectrum;
    m_darkSpectrum->hide();

    initAxisControl();
    initChart();
    initConnectInfo();
    initTable();
    initToolButton();

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

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();

    connect(m_plotSimulate, &FormPlotSimulate::simulateDataReady, formSerial, &FormSerial::onSimulateRecv,
            Qt::QueuedConnection);

    connect(m_worker, &ThreadWorker::plotReady4k, this, &FormEasy::updatePlot4k, Qt::QueuedConnection);
    connect(this, &FormEasy::recvThreshold, m_worker, &ThreadWorker::onUseLoadedThreshold);
    connect(this, &FormEasy::recvThresholdOption, m_worker, &ThreadWorker::onUseLoadedThreadsholdOption);
    connect(this, &FormEasy::sendOption, m_worker, &ThreadWorker::onUseLoadedThreadsholdOption);

    connect(formSerial, &FormSerial::sendThreshold, m_worker, &ThreadWorker::onUseLoadedThreshold,
            Qt::QueuedConnection);
    connect(formSerial, &FormSerial::sendOption, m_worker, &ThreadWorker::onUseLoadedThreadsholdOption,
            Qt::QueuedConnection);
    connect(formSerial, &FormSerial::recv2PlotLLC, m_worker, &ThreadWorker::processDataLLC, Qt::QueuedConnection);
    connect(formSerial, &FormSerial::recv2PlotF30, m_worker, &ThreadWorker::processDataF30, Qt::QueuedConnection);
    connect(formSerial, &FormSerial::recv2PlotF15, m_worker, &ThreadWorker::processDataF15, Qt::QueuedConnection);

    connect(this, &FormEasy::toHistory, m_plotHistory, &FormPlotHistory::onHistoryRecv, Qt::QueuedConnection);
    connect(m_plotHistory, &FormPlotHistory::sendToPlot, this, &FormEasy::updatePlot4k, Qt::QueuedConnection);

    connect(m_chartView, &MyChartView::toSelect, this, [&](const QPointF &point) {
        ui->lineEditCurrentX->setText(QString::number(point.x()));
        ui->lineEditCurrentY->setText(QString::number(point.y()));
        highlightRowByX(point.x());
    });
    connect(m_trajectory, &PeakTrajectory::windowClose, this, [this]() {
        ui->checkBoxPeakTrack->setChecked(false);
        on_checkBoxPeakTrack_checkStateChanged(Qt::Unchecked);
    });
    connect(m_accumulate, &Accumulate::windowClose, this, [this]() {
        m_enableAccumulate = false;
        ui->tBtnAccumulate->setChecked(false);
    });
    connect(m_fourierTransform, &FourierTransform::windowClose, this, [this]() {
        m_enableFourier = false;
        ui->tBtnFourier->setChecked(false);
    });

    m_overlay = new LoadingOverLay(this);
    m_overlay->hide();
    connect(m_overlay, &LoadingOverLay::stopConnect, this, [this]() {
        LOG_INFO("connect stopped by user");
        m_isPlaying = false;
        ui->tBtnSwitch->setChecked(false);
        closeEasyMode();
        m_overlay->hide();
    });
    connect(formSerial, &FormSerial::connectEasyModeEstablished, this, [this]() {
        m_isPlaying = true;
        ui->tBtnSwitch->setChecked(true);
        m_overlay->hide();
    });
    connect(formSerial, &FormSerial::redoConnect, this, [this]() {
        formSerial->startEasyConnect(m_F30_shown_mode);
        m_overlay->reTry();
    });
    connect(formSerial, &FormSerial::statusReport, m_overlay, &LoadingOverLay::updateInfo, Qt::QueuedConnection);
    connect(formSerial, &FormSerial::optReturn, this, [this](int id, const QString& msg) {
        if(id == EASY_CONNECT_SUCCESS) {
            ui->tBtnSwitch->isConnect(true);
        }
        if(id == EASY_CONNECT_STOP) {
            ui->tBtnSwitch->isConnect(false);
        }
        ui->labelMsg->setText(msg);
    });
    formSerial->initEasyConnect();
}

void FormEasy::highlightRowByX(double x) {
    if (!m_modelValue) return;

    int targetRow = -1;
    for (int row = 0; row < m_modelValue->rowCount(); ++row) {
        QVariant data = m_modelValue->data(m_modelValue->index(row, 0));
        if (qFuzzyCompare(data.toDouble(), x)) {
            targetRow = row;
            break;
        }
    }

    if (targetRow < 0) return;

    QTableView *table = ui->tableViewValue;
    table->setCurrentIndex(m_modelValue->index(targetRow, 0));

    QItemSelectionModel *sel = table->selectionModel();
    sel->clearSelection();
    QItemSelection selection(m_modelValue->index(targetRow, 0),
                             m_modelValue->index(targetRow, m_modelValue->columnCount() - 1));
    sel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    table->scrollTo(m_modelValue->index(targetRow, 0), QAbstractItemView::PositionAtCenter);
}

static int findClosestIndex(const QVector<QPointF> &data, double pos) {
    if (data.isEmpty()) return -1;

    int left = 0;
    int right = data.size() - 1;

    while (left <= right) {
        int mid = (left + right) / 2;

        if (data[mid].x() < pos)
            left = mid + 1;
        else
            right = mid - 1;
    }

    if (left >= data.size()) return data.size() - 1;
    if (left == 0) return 0;

    double d1 = std::abs(data[left].x() - pos);
    double d2 = std::abs(data[left - 1].x() - pos);

    return (d1 < d2) ? left : (left - 1);
}

void FormEasy::updatePlot4k(const MY_DATA &my_data, bool record) {
    if (m_pause) {
        return;
    }
    m_data = my_data;

    if (m_enableFourier || m_enableFourierPercent) {
        if (m_toVoltage) {
            auto data = m_fourierTransform->transform(m_data.curve31.data);
            if (!data.isEmpty()) {
                m_data.curve31.data = data;
            }
        } else {
            auto data = m_fourierTransform->transform(m_data.curve31.raw.data);
            if (!data.isEmpty()) {
                m_data.curve31.raw.data = data;
            }
        }
    }

    if(m_enableAccumulate) {
        if (m_toVoltage) {
            auto data = m_accumulate->accumulate(m_data.curve31.data);
            if (!data.isEmpty()) {
                m_data.curve31.data = data;
            }
        } else {
            auto data = m_accumulate->accumulate(m_data.curve31.raw.data);
            if (!data.isEmpty()) {
                m_data.curve31.raw.data = data;
            }
        }
    }

    if (m_enablePointsTracker) {
        QMap<QString, double> values;
        for (double pos : m_vPointsTracker) {
            int idx = findClosestIndex(m_toVoltage ? m_data.curve31.data : m_data.curve31.raw.data, pos);

            if (idx >= 0) {
                double value = m_toVoltage ? m_data.curve31.data.at(idx).y() : m_data.curve31.raw.data.at(idx).y();
                QString name = QString::number(pos, 'f', 2);
                values[name] = value;
            }
        }

        m_pointsTracker->addPoints(values);
    }

    if (m_enableSNR) {
        if (m_toVoltage) {
            m_snr->calculate(m_data.curve31.data);
        } else {
            m_snr->calculate(m_data.curve31.raw.data);
        }
    }

    if (record) {
        emit toHistory(m_data);
    }

    updatePlot(m_data.curve31, {}, m_data.temperature, record);
    QVector<double> v31, r31;
    for (int i = 0; i < m_data.curve31.data.size(); ++i) {
        v31.push_back(m_data.curve31.data.at(i).y());
    }
    for (int i = 0; i < m_data.curve31.raw.data.size(); ++i) {
        r31.push_back(m_data.curve31.raw.data.at(i).y());
    }
    updateTable(m_data.curve31.data, m_data.curve31.raw.data);

    if (m_enableDarkSpectrum) {
        if (m_doDarkSpectrumCalc) {
            if (m_toVoltage) {
                m_darkSpectrum->calculate(v31);
            } else {
                m_darkSpectrum->calculate(r31);
            }
        }
    }
}

void FormEasy::on_tBtnZoom_clicked() {
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);
    if (m_autoZoom) {
        m_enableAxisY = false;
        ui->tBtnAxisY->setChecked(false);
    } else {
        m_enableAxisY = true;
        ui->tBtnAxisY->setChecked(true);
    }
}

void FormEasy::on_tBtnCrop_clicked() {
    m_enableCrop = !m_enableCrop;
    ui->tBtnCrop->setChecked(m_enableCrop);
    if (m_enableCrop) {
        m_chartView->setCropEnabled(true);
        m_chartView->recordInitialAxisRange();
    } else {
        m_chartView->setCropEnabled(false);
        m_chartView->backInitialRange();
    }
}

void FormEasy::on_tBtnPeak_clicked() {
    m_findPeak = !m_findPeak;
    ui->tBtnPeak->setChecked(m_findPeak);
    m_peaks->setVisible(m_findPeak);
    m_peakCfg->setVisible(m_findPeak || m_calcFWHM);
    callFindPeak();
}

void FormEasy::on_tBtnFWHM_clicked() {
    m_calcFWHM = !m_calcFWHM;
    ui->tBtnFWHM->setChecked(m_calcFWHM);
    m_peakCfg->setVisible(m_calcFWHM || m_findPeak);
    callCalcFWHM();
}

void FormEasy::on_tBtnImg_clicked() {
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Chart"), QString("%1.png").arg(TIMESTAMP_1("yyyyMMdd_hhmmss")),
                                                    "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (!filePath.isEmpty()) {
        saveChartAsImage(filePath);
    }
}

void FormEasy::sendIntegrationTime() {
    int val = ui->spinBoxIntegrationTime->value();
    QString unit = ui->comboBoxTimeUnit->currentText();
    if (unit == "ms") {
        val = val;
    } else if (unit == "s") {
        val *= 1000;
    }
    int count = (val + 1) / 5;
    emit sendOption({{"integration", count}});
    formSerial->doEasyOpt(EASY_SET_INTEGRATION_TIME, calcIntegrationTime(val));
}

void FormEasy::on_spinBoxIntegrationTime_valueChanged(int val) {
    if (m_isPlaying) {
        sendIntegrationTime();
    }
}

void FormEasy::on_tBtnPause_clicked() {
    m_pause = !m_pause;
    ui->tBtnPause->setChecked(m_pause);
}

void FormEasy::saveChartAsImage(const QString &filePath) {
    if (!m_chartView) return;

    QSize size = m_chartView->size();

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    m_chartView->render(&painter);
    painter.end();

    image.save(filePath);
}

void FormEasy::closeEvent(QCloseEvent *event) {
    closeEasyMode();
    m_peakCfg->close();
    m_trajectory->close();
    m_plotSimulate->close();
    m_plotHistory->close();
    m_fourierTransform->close();
    m_derivation->close();
    m_snr->close();
    m_accumulate->close();
    m_pointsTracker->close();
    m_darkSpectrum->close();
}

void FormEasy::updatePlot(const CURVE &curve31, const CURVE &curve33, const double &temperature, bool record) {
    double val_min = std::numeric_limits<double>::max();
    double val_max = std::numeric_limits<double>::min();
    QList<QPointF> v;

    if (m_toVoltage) {
        for (int i = 0; i < curve31.data.size(); ++i) {
            v.push_back(QPointF(curve31.data[i].x(), curve31.data[i].y()));
            val_min = std::min(val_min, curve31.data[i].y());
            val_max = std::max(val_max, curve31.data[i].y());
        }
    } else {
        for (int i = 0; i < curve31.raw.data.size(); ++i) {
            v.push_back(QPointF(curve31.raw.data[i].x(), curve31.raw.data[i].y()));
            val_min = std::min(val_min, curve31.raw.data[i].y());
            val_max = std::max(val_max, curve31.raw.data[i].y());
        }
    }

    ui->labelValMinIntensity->setText(QString::number(val_min));
    ui->labelValMaxIntensity->setText(QString::number(val_max));
    m_line->replace(v);

    if (m_autoZoom) {
        m_axisY->setRange(val_min, val_max);
    } else {
        m_axisY->setRange(m_y_start, m_y_end);
    }

    if (m_enableAxisX) {
        if (m_toVoltage) {
            m_axisX->setRange(m_x_start, m_x_end);
        } else {
            m_axisX->setRange(m_x_start, m_x_end);
        }
    } else {
        if (m_toVoltage) {
            m_axisX->setRange(curve31.x_min, curve31.x_max);
        } else {
            m_axisX->setRange(curve31.raw.x_min, curve31.raw.x_max);
        }
    }

    callFindPeak();
    if (m_enablePeakTrack) {
        QPointF maxPoint;
        qreal maxY = -std::numeric_limits<qreal>::max();

        for (const QPointF &p : m_peaks->points()) {
            if (p.x() < m_trajectory_start || p.x() > m_trajectory_end) {
                continue;
            }
            if (p.y() > maxY) {
                maxY = p.y();
                maxPoint = p;
            }
        }
        m_trajectory->appendPeak(maxPoint.rx());
    }
    callCalcFWHM();
}

void FormEasy::updateTable(const QList<QPointF> &data, const QList<QPointF> &data_raw) {
    if (m_pause) {
        return;
    }
    if (m_modelValue->rowCount() > 0) {
        m_modelValue->removeRows(0, m_modelValue->rowCount());
    }
    int count = qMax(data.size(), data_raw.size());
    for (int i = 0; i < count; ++i) {
        QString index = QString::number(data.at(i).x());
        QString yV14 = (i < data.size()) ? QString::number(data.at(i).y()) : "";
        QString yR14 = (i < data_raw.size()) ? QString::number(data_raw.at(i).y()) : "";

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(index);
        rowItems << new QStandardItem(yV14);
        rowItems << new QStandardItem(yR14);
        m_modelValue->appendRow(rowItems);
    }
}

void FormEasy::callFindPeak() {
    if (m_findPeak) {
        if (!m_line || m_line->count() < 5) {
            return;
        }

        auto cfg = m_peakCfg->getCfg();
        QVector<QPointF> peaks24 = FindPeak::find(m_line, cfg[0], cfg[1], cfg[2]);;

        m_peaks->clear();
        for (const auto &pt : peaks24) {
            m_peaks->append(pt);
        }

        m_chart->update();
    } else {
        m_peaks->clear();
    }
}

void FormEasy::clearFWHM() {
    for (auto *line : m_fwhmLines) {
        m_chart->removeSeries(line);
        delete line;
    }
    m_fwhmLines.clear();
    for (auto *label : m_fwhmLabels) {
        delete label;
    }
    m_fwhmLabels.clear();
}

void FormEasy::drawFWHM(double xPeak, double xLeft, double xRight, double yHalf) {
    double fwhm = xRight - xLeft;

    QLineSeries *fwhmLine = new QLineSeries();
    fwhmLine->setColor(Qt::red);
    fwhmLine->setName(QString("FWHM %1").arg(xPeak, 0, 'f', 1));
    fwhmLine->append(xLeft, yHalf);
    fwhmLine->append(xRight, yHalf);
    m_chart->addSeries(fwhmLine);
    fwhmLine->attachAxis(m_axisX);
    fwhmLine->attachAxis(m_axisY);
    m_fwhmLines.append(fwhmLine);

    QPointF mid((xLeft + xRight) / 2.0, yHalf);
    QPointF scenePos = m_chart->mapToPosition(mid, fwhmLine);
    auto *label = new QGraphicsSimpleTextItem(
        QString("FWHM=%1").arg(fwhm, 0, 'f', 2), m_chart);
    label->setBrush(Qt::red);
    label->setPos(scenePos + QPointF(5, -15));
    m_fwhmLabels.append(label);
}

void FormEasy::updateFWHMLabels() {
    if (m_fwhmLines.size() != m_fwhmLabels.size()) return;
    if (m_fwhmResults.size() != m_fwhmLabels.size()) return;

    for (int i = 0; i < m_fwhmLabels.size(); ++i) {
        const auto &r = m_fwhmResults[i];
        QPointF mid((r.xLeft + r.xRight) / 2.0, r.yHalf);
        QPointF scenePos = m_chart->mapToPosition(mid, m_fwhmLines[i]);
        m_fwhmLabels[i]->setPos(scenePos + QPointF(5, -15));
    }
}

void FormEasy::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_overlay->resize(this->size());
    updateFWHMLabels();
}

void FormEasy::callCalcFWHM() {
    clearFWHM();
    if (!m_calcFWHM) return;

    auto cfg   = m_peakCfg->getCfg();
    auto peaks = FindPeak::find(m_line, cfg[0], cfg[1], cfg[2]);
    m_fwhmResults = FindFWHM::find(m_line, peaks);

    for (const auto &r : m_fwhmResults) {
        drawFWHM(r.xPeak, r.xLeft, r.xRight, r.yHalf);
    }
}

QString FormEasy::calcIntegrationTime(int value) {
    int rawValue = value;
    QString hex = QString("%1").arg(rawValue, 6, 16, QLatin1Char('0')).toUpper();
    QString prefix = "DD3C000622";
    QString suffix = "CDFF";
    QString cmd = prefix + hex + suffix;
    LOG_INFO("CalcIntegrationTime {} -> {}", value, cmd);
    return cmd;
}

void FormEasy::doSimulateClicked() {
    m_enableSimulate = !m_enableSimulate;
    m_plotSimulate->setVisible(m_enableSimulate);
    m_tBtnSimulate->setChecked(m_enableSimulate);
}

void FormEasy::doHistoryClicked() {
    m_enableHistory = !m_enableHistory;
    m_plotHistory->setVisible(m_enableHistory);
    m_tBtnHistory->setChecked(m_enableHistory);
}

void FormEasy::on_tBtnFourier_clicked() {
    m_enableFourier = !m_enableFourier;
    m_fourierTransform->setPercent(0);
    m_fourierTransform->setVisible(ui->tBtnFourier->isChecked());
}

void FormEasy::on_tBtnAccumulate_clicked() {
    m_enableAccumulate = !m_enableAccumulate;
    m_accumulate->setVisible(ui->tBtnAccumulate->isChecked());
}

void FormEasy::doSNRClicked() {
    m_enableSNR = !m_enableSNR;
    m_snr->setVisible(m_enableSNR);
    m_tBtnSNR->setChecked(m_enableSNR);
}

void FormEasy::doPointsTracker() {
    m_enablePointsTracker = !m_enablePointsTracker;
    m_pointsTracker->setVisible(m_enablePointsTracker);
    m_tBtnPointsTracker->setChecked(m_enablePointsTracker);
}

void FormEasy::doDarkSpectrum() {
    m_enableDarkSpectrum = !m_enableDarkSpectrum;
    m_darkSpectrum->setVisible(m_enableDarkSpectrum);
    m_tBtnDarkSpectrum->setChecked(m_enableDarkSpectrum);
}

void FormEasy::on_tBtnAxisY_clicked() {
    m_enableAxisY = !m_enableAxisY;
    ui->tBtnAxisY->setChecked(m_enableAxisY);
    if (m_enableAxisY) {
        m_autoZoom = false;
        ui->tBtnZoom->setChecked(false);
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_ENABLE, VAL_ENABLE);
    } else {
        m_autoZoom = true;
        ui->tBtnZoom->setChecked(true);
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_ENABLE, VAL_DISABLE);
    }
}

void FormEasy::on_tBtnToVoltage_clicked() {
    m_toVoltage = !m_toVoltage;
    ui->tBtnToVoltage->setChecked(m_toVoltage);
}

void FormEasy::on_spinBoxYStart_valueChanged(int val) {
    m_y_start = val;
    SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_START, QString::number(val));
}

void FormEasy::on_spinBoxYEnd_valueChanged(int val) {
    m_y_end = val;
    SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_END, QString::number(val));
}

void FormEasy::on_spinBoxXStart_valueChanged(int val) {
    m_x_start = val;
    if (m_enableAxisX) {
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_START, QString::number(val));
    }
}

void FormEasy::on_spinBoxXEnd_valueChanged(int val) {
    m_x_end = val;
    if (m_enableAxisX) {
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_END, QString::number(val));
    }
}

void FormEasy::on_tBtnAxisX_clicked() {
    m_enableAxisX = !m_enableAxisX;
    ui->tBtnAxisX->setChecked(m_enableAxisX);
}

void FormEasy::on_checkBoxPeakTrack_checkStateChanged(const Qt::CheckState &state) {
    if (state == Qt::Checked) {
        m_enablePeakTrack = true;
        m_findPeak = true;
        ui->tBtnPeak->setChecked(true);
        m_peaks->setVisible(true);
        m_trajectory->show();

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
    } else {
        m_enablePeakTrack = false;
        m_findPeak = false;
        ui->tBtnPeak->setChecked(false);
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
}

void FormEasy::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    QAction *loadChartAction = menu.addAction(tr("Load Chart"));
    QAction *exportChartAction = menu.addAction(tr("Export Chart"));
    menu.addSeparator();
    QAction *pointsTrackerAction = nullptr;
    QAction *pointsClearAction = nullptr;
    if (m_enablePointsTracker) {
        pointsTrackerAction = menu.addAction(tr("Add Points Tracker"));
        pointsClearAction = menu.addAction(tr("Clear Tracker"));
    }

    QAction *selectedAction = menu.exec(event->globalPos());
    if (!selectedAction) return;

    if (selectedAction == loadChartAction) {
        loadChart();
    } else if (selectedAction == exportChartAction) {
        exportChart();
    } else if (selectedAction == pointsTrackerAction) {
        addToPointsTracker();
    } else if (selectedAction == pointsClearAction) {
        clearPointsTracker();
    }
}

void FormEasy::clearPointsTracker() {
    m_pointsTracker->clearPoints();
    m_vPointsTracker.clear();
}

void FormEasy::addToPointsTracker() {
    bool ok = false;

    double pos = QInputDialog::getDouble(this, tr("Add Point"), tr("Input position:"), 0.0, 900, 1700, 4, &ok);

    if (!ok) return;

    m_vPointsTracker.append(pos);
}

void FormEasy::loadChart() {
    QString path = QFileDialog::getOpenFileName(this, tr("choose file"), "", "CSV Files (*.csv)");

    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, TITLE_ERROR, tr("Cannot open file."));
        return;
    }

    QTextStream in(&file);

    m_line->clear();

    CURVE curve;
    bool firstLine = true;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        if (firstLine) {
            firstLine = false;
            continue;
        }

        QStringList parts = line.split(",");
        if (parts.size() < 2) continue;

        double x = parts[0].toDouble();
        double y_raw = parts[1].toDouble();
        double y_voltage = parts[2].toDouble();
        curve.data.push_back({x, y_voltage});
        curve.x_min = std::min(curve.x_min, x);
        curve.x_max = std::max(curve.x_max, x);
        curve.y_min = std::min(curve.y_min, y_voltage);
        curve.y_max = std::max(curve.y_max, y_voltage);
        curve.raw.data.push_back({x, y_raw});
        curve.raw.x_min = std::min(curve.raw.x_min, x);
        curve.raw.x_max = std::max(curve.raw.x_max, x);
        curve.raw.y_min = std::min(curve.raw.y_min, y_raw);
        curve.raw.y_max = std::max(curve.raw.y_max, y_raw);
    }

    file.close();
    MY_DATA data;
    data.curve31 = curve;
    updatePlot4k(data, false);

    QMessageBox::information(this, TITLE_INFO, tr("CSV loaded successfully."));
}

void FormEasy::exportChart() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV"), "spectral.csv", "CSV Files (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, TITLE_ERROR, tr("Cannot open file."));
        return;
    }

    QTextStream out(&file);
    out << "Index,Intensity_Raw,Intensity_Voltage\n";
    for (int i = 0; i < m_line->count(); ++i) {
        out << QString::number(m_line->at(i).x()) << "," << QString::number(m_data.curve31.raw.data.at(i).y()) << ","
            << QString::number(m_data.curve31.data.at(i).y()) << "\n";
    }
    file.close();

    QMessageBox::information(this, TITLE_INFO, tr("CSV exported successfully."));
}

void FormEasy::on_tBtnInfo_clicked() {
    QPoint pos = ui->tBtnInfo->mapToGlobal(QPoint(0, ui->tBtnInfo->height()));

    m_infoPopup->move(pos);
    m_infoPopup->show();
}

void FormEasy::on_comboBoxTimeUnit_currentIndexChanged(int index) {
    if (index == 0) {
        ui->spinBoxIntegrationTime->setSingleStep(5);
        ui->spinBoxIntegrationTime->setMinimum(5);
    } else if (index == 1) {
        ui->spinBoxIntegrationTime->setSingleStep(1);
        ui->spinBoxIntegrationTime->setMinimum(1);
    }
    sendIntegrationTime();
}
