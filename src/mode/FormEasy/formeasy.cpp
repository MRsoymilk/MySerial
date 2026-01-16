#include "formeasy.h"
#include <QMovie>
#include "../ThreadWorker/threadworker.h"
#include "../form/FormPlotHistory/formplothistory.h"
#include "../form/FormPlotSimulate/formplotsimulate.h"
#include "../form/plot/Accumulate/accumulate.h"
#include "../form/plot/Derivation/derivation.h"
#include "../form/plot/FourierTransform/fouriertransform.h"
#include "../form/plot/SignalNoiseRatio/signalnoiseratio.h"
#include "../form/serial/formserial.h"
#include "LoadingOverLay/loadingoverlay.h"
#include "MyChartView/mychartview.h"
#include "funcdef.h"
#include "ui_formeasy.h"

FormEasy::FormEasy(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormEasy)
{
    ui->setupUi(this);
    init();
}

FormEasy::~FormEasy()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
    delete ui;
}

void FormEasy::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormEasy::setAlgorithm(const QString &algorithm)
{
    formSerial->onChangeFrameType(algorithm);
    m_worker->setAlgorithm(algorithm);
}

bool FormEasy::connectEasyMode()
{
    LoadingOverLay *overlay = new LoadingOverLay(this);
    connect(formSerial, &FormSerial::statusReport, overlay, &LoadingOverLay::updateInfo);
    overlay->resize(this->size());
    overlay->show();
    QString algorithm = qApp->property("algorithm").toString();
    formSerial->updateFrameTypes(algorithm);
    m_worker->setAlgorithm(algorithm);
    bool isConnect = false;

    int *connect_count = new int(0);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() mutable {
        overlay->updateTry(++(*connect_count));

        if (formSerial->startEasyConnect()) {
            LOG_INFO("connect [{}] success.", *connect_count);
            timer->stop();
            overlay->deleteLater();
            timer->deleteLater();
            delete connect_count;
        }
    });
    timer->start(500);
    return true;
}

void FormEasy::closeEasyMode()
{
    formSerial->stopEasyConnect();
}

void FormEasy::on_tBtnSwitch_clicked()
{
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
        m_isPlaying = false;
        ui->tBtnSwitch->setChecked(false);
        closeEasyMode();
    }
}

void FormEasy::init()
{
    int x_start = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_START, "0").toInt();
    int x_end = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_END, "1").toInt();
    int y_start = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_START, "0").toInt();
    int y_end = SETTING_CONFIG_GET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_END, "1").toInt();
    ui->spinBoxXStart->setValue(x_start);
    ui->spinBoxXEnd->setValue(x_end);
    ui->spinBoxYStart->setValue(y_start);
    ui->spinBoxYEnd->setValue(y_end);

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

    ui->comboBoxTimeUnit->addItems({"ms", "s"});
    ui->comboBoxTimeUnit->setCurrentIndex(0);

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

    ui->spinBoxIntegrationTime->setValue(5);

    ui->tBtnSwitch->setToolTip(tr("switch"));
    ui->tBtnPause->setToolTip(tr("pause"));
    ui->tBtnZoom->setToolTip(tr("zoom"));
    ui->tBtnCrop->setToolTip(tr("crop"));
    ui->tBtnPeak->setToolTip(tr("find peak"));
    ui->tBtnFWHM->setToolTip(tr("FWHM"));
    ui->tBtnImg->setToolTip(tr("save image"));
    ui->tBtnSimulate->setToolTip(tr("simulate"));
    ui->tBtnHistory->setToolTip(tr("history"));
    ui->tBtnFourier->setToolTip(tr("fourier"));
    ui->tBtnAccumulate->setToolTip(tr("accumulate"));
    ui->tBtnSNR->setToolTip(tr("signal noise ratio"));

    ui->tBtnZoom->setChecked(true);

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

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();

    connect(m_plotSimulate,
            &FormPlotSimulate::simulateDataReady,
            formSerial,
            &FormSerial::onSimulateRecv,
            Qt::QueuedConnection);

    connect(m_worker,
            &ThreadWorker::plotReady4k,
            this,
            &FormEasy::updatePlot4k,
            Qt::QueuedConnection);
    connect(m_worker, &ThreadWorker::dataReady4k, this, &FormEasy::updateTable, Qt::QueuedConnection);

    connect(formSerial, &FormSerial::recv2PlotLLC, m_worker, &ThreadWorker::processDataLLC);
    connect(formSerial, &FormSerial::recv2PlotF30, m_worker, &ThreadWorker::processDataF30);
    connect(formSerial, &FormSerial::recv2PlotF15, m_worker, &ThreadWorker::processDataF15);

    connect(this, &FormEasy::toHistory, m_plotHistory, &FormPlotHistory::onHistoryRecv);

    connect(m_chartView, &MyChartView::toSelect, this, [&](const QPointF &point) {
        ui->lineEditCurrentX->setText(QString::number(point.x()));
        ui->lineEditCurrentY->setText(QString::number(point.y()));
        highlightRowByX(point.x());
    });

    connect(m_plotSimulate, &FormPlotSimulate::windowClose, this, [this]() {
        ui->tBtnSimulate->setChecked(false);
    });
    connect(m_plotHistory, &FormPlotHistory::windowClose, this, [this]() {
        ui->tBtnHistory->setChecked(false);
    });
    connect(m_accumulate, &Accumulate::windowClose, this, [this]() {
        m_enableAccumulate = false;
        ui->tBtnAccumulate->setChecked(false);
    });
    connect(m_snr, &SignalNoiseRatio::windowClose, this, [this]() {
        m_enableSNR = false;
        ui->tBtnSNR->setChecked(false);
    });
    connect(m_fourierTransform, &FourierTransform::windowClose, this, [this]() {
        m_enableFourier = false;
        ui->tBtnFourier->setChecked(false);
    });
}

void FormEasy::highlightRowByX(double x)
{
    if (!m_modelValue)
        return;

    int row = qRound(x) - 900;

    if (row < 0 || row >= m_modelValue->rowCount())
        return;

    QTableView *table = ui->tableViewValue;

    table->setCurrentIndex(m_modelValue->index(row, 0));

    QItemSelectionModel *sel = table->selectionModel();
    sel->clearSelection();

    QItemSelection selection(m_modelValue->index(row, 0),
                             m_modelValue->index(row, m_modelValue->columnCount() - 1));

    sel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    table->scrollTo(m_modelValue->index(row, 0), QAbstractItemView::PositionAtCenter);
}

void FormEasy::updatePlot4k(const CURVE &curve31,
                            const CURVE &curve33,
                            const double &temperature,
                            bool record)
{
    if (m_pause) {
        return;
    }
    CURVE plot31 = curve31;

    if (m_enableFourier) {
        if (m_toVoltage) {
            auto data = m_fourierTransform->transform(plot31.data);
            if (!data.isEmpty()) {
                plot31.data = data;
            }
        } else {
            auto data = m_fourierTransform->transform(plot31.raw.data);
            if (!data.isEmpty()) {
                plot31.raw.data = data;
            }
        }
    }

    if (m_enableAccumulate) {
        if (m_toVoltage) {
            auto data = m_accumulate->accumulate(plot31.data);
            if (!data.isEmpty()) {
                plot31.data = data;
            } else {
                return;
            }
        } else {
            auto data = m_accumulate->accumulate(plot31.raw.data);
            if (!data.isEmpty()) {
                plot31.raw.data = data;
            } else {
                return;
            }
        }
    }

    if (m_enableSNR) {
        if (m_toVoltage) {
            m_snr->calculate(plot31.data);
        } else {
            m_snr->calculate(plot31.raw.data);
        }
    }

    if (record) {
        emit toHistory(plot31, {}, temperature);
    }

    updatePlot(plot31, {}, temperature, record);
}

void FormEasy::on_tBtnZoom_clicked()
{
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

void FormEasy::on_tBtnCrop_clicked()
{
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

void FormEasy::on_tBtnPeak_clicked()
{
    m_findPeak = !m_findPeak;
    ui->tBtnPeak->setChecked(m_findPeak);
    m_peaks->setVisible(m_findPeak);
    callFindPeak();
}

void FormEasy::on_tBtnFWHM_clicked()
{
    m_calcFWHM = !m_calcFWHM;
    ui->tBtnFWHM->setChecked(m_calcFWHM);
    callCalcFWHM();
}

void FormEasy::on_tBtnImg_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save Chart"),
                                                    QString("%1.png").arg(TIMESTAMP_0()),
                                                    "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (!filePath.isEmpty()) {
        saveChartAsImage(filePath);
    }
}

void FormEasy::on_spinBoxIntegrationTime_valueChanged(int val)
{
    if (m_isPlaying) {
        QString unit = ui->comboBoxTimeUnit->currentText();
        if (unit == "ms") {
            val = val;
        } else if (unit == "s") {
            val *= 1000;
        }
        formSerial->writeEasyData(calcIntegrationTime(val));
    }
}

void FormEasy::on_tBtnPause_clicked()
{
    m_pause = !m_pause;
    ui->tBtnPause->setChecked(m_pause);
}

void FormEasy::saveChartAsImage(const QString &filePath)
{
    if (!m_chartView)
        return;

    QSize size = m_chartView->size();

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    m_chartView->render(&painter);
    painter.end();

    image.save(filePath);
}

void FormEasy::closeEvent(QCloseEvent *event)
{
    closeEasyMode();

    m_plotSimulate->close();
    m_plotHistory->close();
    m_fourierTransform->close();
    m_derivation->close();
    m_snr->close();
    m_accumulate->close();
}

void FormEasy::updatePlot(const CURVE &curve31,
                          const CURVE &curve33,
                          const double &temperature,
                          bool record)
{
    double val_min = std::numeric_limits<double>::max();
    double val_max = std::numeric_limits<double>::min();
    QList<QPointF> v;

    if (m_toVoltage) {
        for (int i = 0; i < curve31.data.size(); ++i) {
            v.push_back(QPointF(curve31.data[i].x() + 900, curve31.data[i].y()));
            val_min = std::min(val_min, curve31.data[i].y());
            val_max = std::max(val_max, curve31.data[i].y());
        }
    } else {
        for (int i = 0; i < curve31.raw.data.size(); ++i) {
            v.push_back(QPointF(curve31.raw.data[i].x() + 900, curve31.raw.data[i].y()));
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
            m_axisX->setRange(curve31.x_min + 900, curve31.x_max + 900);
        } else {
            m_axisX->setRange(curve31.raw.x_min + 900, curve31.raw.x_max + 900);
        }
    }

    callFindPeak();
    callCalcFWHM();
}

void FormEasy::updateTable(const QVector<double> &v14,
                           const QVector<double> &v24,
                           const QVector<double> &raw14,
                           const QVector<double> &raw24)
{
    if (m_pause) {
        return;
    }
    if (m_modelValue->rowCount() > 0) {
        m_modelValue->removeRows(0, m_modelValue->rowCount());
    }
    int count = qMax(qMax(v14.size(), v24.size()), qMax(raw14.size(), raw24.size()));
    for (int i = 0; i < count; ++i) {
        QString index = QString::number(i + 900);
        QString yV14 = (i < v14.size()) ? QString::number(v14[i]) : "";
        QString yR14 = (i < raw14.size()) ? QString::number(raw14[i]) : "";

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(index);
        rowItems << new QStandardItem(yV14);
        rowItems << new QStandardItem(yR14);
        m_modelValue->appendRow(rowItems);
    }
}

QVector<QPointF> FormEasy::findPeak(int window, double thresholdFactor, double minDist)
{
    QVector<QPointF> peaks;
    int n = m_line->count();

    QVector<double> values;
    values.reserve(n);
    for (int i = 0; i < n; i++)
        values.append(m_line->at(i).y());

    double mean = std::accumulate(values.begin(), values.end(), 0.0) / n;
    double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / n - mean * mean);
    double threshold = mean + thresholdFactor * stdev;

    double lastPeakX = -1e9;
    for (int i = window; i < n - window; i++) {
        double yCurr = m_line->at(i).y();
        if (yCurr < threshold)
            continue;

        bool isPeak = true;
        for (int j = i - window; j <= i + window; j++) {
            if (m_line->at(j).y() > yCurr) {
                isPeak = false;
                break;
            }
        }

        if (isPeak) {
            double xCurr = m_line->at(i).x();
            if (xCurr - lastPeakX >= minDist) {
                peaks.append(m_line->at(i));
                lastPeakX = xCurr;
            }
        }
    }
    return peaks;
}

void FormEasy::callFindPeak()
{
    if (m_findPeak) {
        if (!m_line || m_line->count() < 5) {
            return;
        }

        QVector<QPointF> peaks24 = findPeak(3, 1.0, 5.0);

        m_peaks->clear();
        for (const auto &pt : peaks24) {
            m_peaks->append(pt);
        }

        m_chart->update();
    } else {
        m_peaks->clear();
    }
}

void FormEasy::callCalcFWHM()
{
    if (m_calcFWHM) {
        for (auto *line : m_fwhmLines) {
            m_chart->removeSeries(line);
            delete line;
        }
        m_fwhmLines.clear();
        for (auto *label : m_fwhmLabels) {
            delete label;
        }
        m_fwhmLabels.clear();

        auto peaks = findPeak(3, 1.0, 5.0);
        if (peaks.isEmpty())
            return;

        for (const auto &peak : peaks) {
            double yPeak = peak.y();
            double xPeak = peak.x();
            double yHalf = yPeak / 2.0;

            double xLeft = xPeak, xRight = xPeak;
            for (int i = m_line->count() - 1; i >= 1; --i) {
                if (m_line->at(i).x() >= xPeak)
                    continue;
                double y1 = m_line->at(i).y();
                double y2 = m_line->at(i - 1).y();
                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_line->at(i).x();
                    double x2 = m_line->at(i - 1).x();
                    // 线性插值
                    xLeft = x1 + (yHalf - y1) * (x2 - x1) / (y2 - y1);
                    break;
                }
            }
            for (int i = 0; i < m_line->count() - 1; ++i) {
                if (m_line->at(i).x() <= xPeak)
                    continue;
                double y1 = m_line->at(i).y();
                double y2 = m_line->at(i + 1).y();
                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_line->at(i).x();
                    double x2 = m_line->at(i + 1).x();
                    // 线性插值
                    xRight = x1 + (yHalf - y1) * (x2 - x1) / (y2 - y1);
                    break;
                }
            }
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

            auto *label = new QGraphicsSimpleTextItem(QString("FWHM=%1").arg(fwhm, 0, 'f', 2),
                                                      m_chart);
            label->setBrush(Qt::red);
            label->setPos(scenePos + QPointF(5, -15));
            m_fwhmLabels.append(label);
        }
    } else {
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
}

QString FormEasy::calcIntegrationTime(int value)
{
    int rawValue = value;
    QString hex = QString("%1").arg(rawValue, 6, 16, QLatin1Char('0')).toUpper();
    QString prefix = "DD3C000622";
    QString suffix = "CDFF";
    QString cmd = prefix + hex + suffix;
    LOG_INFO("CalcIntegrationTime {} -> {}", value, cmd);
    return cmd;
}

void FormEasy::on_tBtnSimulate_clicked()
{
    m_plotSimulate->setVisible(ui->tBtnSimulate->isChecked());
}

void FormEasy::on_tBtnHistory_clicked()
{
    m_plotHistory->setVisible(ui->tBtnHistory->isChecked());
}

void FormEasy::on_tBtnFourier_clicked()
{
    m_enableFourier = !m_enableFourier;
    m_fourierTransform->setVisible(ui->tBtnFourier->isChecked());
}

void FormEasy::on_tBtnAccumulate_clicked()
{
    m_enableAccumulate = !m_enableAccumulate;
    m_accumulate->setVisible(ui->tBtnAccumulate->isChecked());
}

void FormEasy::on_tBtnSNR_clicked()
{
    m_enableSNR = !m_enableSNR;
    m_snr->setVisible(ui->tBtnSNR->isChecked());
}

void FormEasy::on_tBtnSetting_clicked()
{
}

void FormEasy::on_tBtnAxisY_clicked()
{
    m_enableAxisY = !m_enableAxisY;
    ui->tBtnAxisY->setChecked(m_enableAxisY);
    if (m_enableAxisY) {
        m_autoZoom = false;
        ui->tBtnZoom->setChecked(false);
    } else {
        m_autoZoom = true;
        ui->tBtnZoom->setChecked(true);
    }
}

void FormEasy::on_tBtnToVoltage_clicked()
{
    m_toVoltage = !m_toVoltage;
    ui->tBtnToVoltage->setChecked(m_toVoltage);
}

void FormEasy::on_spinBoxYStart_valueChanged(int val)
{
    m_y_start = val;
    if (m_enableAxisY) {
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_START, QString::number(val));
    }
}

void FormEasy::on_spinBoxYEnd_valueChanged(int val)
{
    m_y_end = val;
    if (m_enableAxisY) {
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_Y_END, QString::number(val));
    }
}

void FormEasy::on_spinBoxXStart_valueChanged(int val)
{
    m_x_start = val;
    if (m_enableAxisX) {
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_START, QString::number(val));
    }
}

void FormEasy::on_spinBoxXEnd_valueChanged(int val)
{
    m_x_end = val;
    if (m_enableAxisX) {
        SETTING_CONFIG_SET(CFG_GROUP_MODE_EASY, CFG_MODE_EASY_X_END, QString::number(val));
    }
}

void FormEasy::on_tBtnAxisX_clicked()
{
    m_enableAxisX = !m_enableAxisX;
    ui->tBtnAxisX->setChecked(m_enableAxisX);
}
