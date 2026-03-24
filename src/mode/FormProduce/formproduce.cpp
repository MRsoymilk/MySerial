#include "formproduce.h"

#include <QThread>

#include "../LoadingOverLay/loadingoverlay.h"
#include "../ThreadWorker/threadworker.h"
#include "../form/FormPlotCorrection/fitting/formfittingpoints.h"
#include "../form/serial/formserial.h"
#include "MyChartView/mychartview.h"
#include "funcdef.h"
#include "ui_formproduce.h"

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
    if(m_formFittingPoints) {
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
    if(m_pause) {
        return;
    }

    CURVE plot31 = my_data.curve31;
    CURVE plot33 = my_data.curve33;
    if (m_enableVoltage) {
        updatePlot2d(plot31.data, plot33.data);
    } else {
        updatePlot2d(plot31.raw.data, plot33.raw.data);
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

    m_formFittingPoints = new FormFittingPoints;
    connect(m_formFittingPoints, &FormFittingPoints::windowClose, this, [&]() {
        m_enableFitting = false;
        m_formFittingPoints->setVisible(false);
    });
    formSerial = new FormSerial;
    m_workerThread = new QThread(this);
    m_worker = new ThreadWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();

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

    QObject::connect(formSerial, &FormSerial::recv2PlotF30, m_worker, &ThreadWorker::processDataF30,
                     Qt::QueuedConnection);
    connect(m_worker, &ThreadWorker::plotReady4k, this, &FormProduce::updatePlot4k, Qt::QueuedConnection);
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
    LoadingOverLay *overlay = new LoadingOverLay(this);
    overlay->resize(this->size());
    overlay->show();
    QString algorithm = qApp->property("algorithm").toString();
    formSerial->updateFrameTypes(algorithm);
    m_worker->setAlgorithm(algorithm);
    connect(overlay, &LoadingOverLay::stopConnect, this, [=]() {
        LOG_INFO("connect stopped by user");
        formSerial->stopFSeriesConnect();
        m_isPlaying = false;
        ui->tBtnSwitch->setChecked(false);
        overlay->deleteLater();
    });
    connect(formSerial, &FormSerial::connectProduceModeEstablished, this, [=]() {
        m_isPlaying = true;
        ui->tBtnSwitch->setChecked(true);
        overlay->close();
    });
    int count = 1;
    overlay->updateTry(count);
    connect(formSerial, &FormSerial::redoConnect, this, [=]() {
        formSerial->stopFSeriesConnect();
        formSerial->startProduceConnect();
        overlay->reTry();
    });
    connect(formSerial, &FormSerial::statusReport, overlay, &LoadingOverLay::updateInfo, Qt::QueuedConnection);
    formSerial->startProduceConnect();
    return true;
}

void FormProduce::closeProduceMode() {
    m_isPlaying = false;
    ui->tBtnSwitch->setChecked(false);
    formSerial->stopFSeriesConnect();
}

void FormProduce::on_btnWriteDeviceSerial_clicked() {
    QString data = ui->lineEditDeviceSerial->text();
    QString msg = QString("%1%2%3").arg("DD3C000920").arg(data).arg("CDFF");
    formSerial->sendProduceData(msg);
}

void FormProduce::on_btnQueryDeviceSerial_clicked() {
    const QByteArray header = QByteArray::fromHex("DE3A000913");
    const QByteArray tail = QByteArray::fromHex("CEFF");
    formSerial->sendProduceData("DD3C000312CDFF", [this, header, tail](const QByteArray &buf) -> bool {
        QByteArray data = buf;
        int headPos = data.indexOf(header);
        if (headPos < 0) {
            LOG_WARN("DeviceSerial: not found header: DE3A000913");
            return false;
        }
        data.remove(0, headPos);
        int tailPos = data.indexOf(tail);
        if (tailPos < 0) {
            LOG_WARN("DeviceSerial: not found tail: CEFF");
            return false;
        }

        int start = header.size();
        int len = tailPos - start;
        if (len <= 0) {
            LOG_WARN("DeviceSerial: invalid length");
            return false;
        }
        data = data.mid(start, len);
        ui->labelValDeviceSerial->setText(data.toHex(' ').toUpper());
        return false;
    });
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
    formSerial->sendProduceData(msg);
}

void FormProduce::on_btnQueryBaseline_clicked() {
    const QByteArray header = QByteArray::fromHex("DE3A000671");
    const QByteArray tail = QByteArray::fromHex("CEFF");
    formSerial->sendProduceData("DD3C000370CDFF", [this, header, tail](const QByteArray &buf) -> bool {
        QByteArray data = buf;
        int headPos = data.indexOf(header);
        if (headPos < 0) {
            LOG_WARN("Baseline: not found header: DE3A000671");
            return false;
        }
        data.remove(0, headPos);
        int tailPos = data.indexOf(tail);
        if (tailPos < 0) {
            LOG_WARN("Baseline: not found tail: CEFF");
            return false;
        }

        int start = header.size();
        int len = tailPos - start;

        if (len <= 0) {
            LOG_WARN("Baseline: invalid length");
            return false;
        }

        data = data.mid(start, len);
        int val =
            (static_cast<quint8>(data[0]) << 16) | (static_cast<quint8>(data[1]) << 8) | static_cast<quint8>(data[2]);
        ui->labelValBaseline->setText(QString::number(val));
        return false;
    });
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
    formSerial->sendProduceData("DD3C000350CDFF", [this](const QByteArray &buf) -> bool {
        if (buf.contains(QByteArray::fromHex("DE3A000351CEFF"))) {
            ui->textBrowserSelfCheckInfo->append(tr("start self check"));
            return true;
        }
        const QByteArray header = QByteArray::fromHex("DE3A000453");
        const QByteArray tail = QByteArray::fromHex("CEFF");
        QByteArray data = buf;
        int headPos = data.indexOf(header);
        if (headPos < 0) {
            LOG_WARN("SelfCheck: not found header: DE3A000453");
            return false;
        }
        data.remove(0, headPos);
        int tailPos = data.indexOf(tail);
        if (tailPos < 0) {
            LOG_WARN("SelfCheck: not found tail: CEFF");
            return false;
        }

        int start = header.size();
        int len = tailPos - start;

        if (len <= 0) {
            LOG_WARN("SelfCheck: invalid length");
            return false;
        }

        data = data.mid(start, len);
        const qint8 code = static_cast<quint8>(data[0]);
        if (code == 0) {
            ui->textBrowserSelfCheckInfo->append(tr("No fault"));
        } else if (code == 1) {
            ui->textBrowserSelfCheckInfo->append(tr("Module not working"));
        } else if (code == 2) {
            ui->textBrowserSelfCheckInfo->append(tr("Temperature too high (above 50℃)"));
        } else if (code == 3) {
            ui->textBrowserSelfCheckInfo->append(tr("Temperature too low (below -20℃)"));
        } else if (code == 4) {
            ui->textBrowserSelfCheckInfo->append(tr("TEC not working"));
        } else if (code == 5) {
            ui->textBrowserSelfCheckInfo->append(tr("TEC unable to power on"));
        } else if (code == 6) {
            ui->textBrowserSelfCheckInfo->append(tr("Fan not working"));
        } else if (code == 7) {
            ui->textBrowserSelfCheckInfo->append(tr("DAC no output"));
        } else if (code == 8) {
            ui->textBrowserSelfCheckInfo->append(tr("Module unstable"));
        }
        return false;
    });
}

void FormProduce::on_tBtnToVoltage_clicked() {
    m_enableVoltage = !m_enableVoltage;
    ui->tBtnToVoltage->setChecked(m_enableVoltage);
}

void FormProduce::on_tBtnPause_clicked()
{
    m_pause = !m_pause;
    ui->tBtnPause->setChecked(m_pause);
}

