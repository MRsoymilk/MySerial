#include "formplot.h"
#include "ui_formplot.h"

#include <QKeySequence>
#include <QLabel>
#include <QLegendMarker>
#include <QPainter>
#include <QShortcut>
#include <QTimer>
#include <QWheelEvent>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "Accumulate/accumulate.h"
#include "Derivation/derivation.h"
#include "DraggableLine/draggableline.h"
#include "FourierTransform/fouriertransform.h"
#include "PeakTrajectory/peaktrajectory.h"
#include "SignalNoiseRatio/signalnoiseratio.h"
#include "TemperatureView/temperatureview.h"
#include "funcdef.h"
#include "plot_algorithm.h"

void FormPlot::saveChartAsImage(const QString &filePath)
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

FormPlot::FormPlot(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlot)
{
    ui->setupUi(this);
    init();
}

FormPlot::~FormPlot()
{
    delete ui;
}

void FormPlot::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormPlot::onDataReceivedLLC(const QByteArray &data31,
                                 const QByteArray &data33,
                                 const double temperature)
{
    if (!m_pause) {
        emit newDataReceivedLLC(data31, data33, temperature);
    }
}

void FormPlot::getINI()
{
    int offset31 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET31, "0").toInt();
    int offset33 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET33, "0").toInt();
    if (offset31 != 0) {
        ui->spinBox31Offset->setValue(offset31);
    }
    if (offset33 != 0) {
        ui->spinBox33Offset->setValue(offset33);
    }

    ui->comboBoxAlgorithm->blockSignals(true);
    ui->comboBoxAlgorithm->addItems({
        "Freedom",
        "F15_curves",
        "F15_single",
        "play_mpu6050",
        "F30_curves",
        "F30_single",
        "LLC_curves",
    });
    ui->comboBoxAlgorithm->blockSignals(false);

    int algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
    ui->comboBoxAlgorithm->setCurrentIndex(algorithm);

    double offset = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET, "0.0").toDouble();
    double step = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_STEP, "1.0").toDouble();
    ui->spinBoxOffset->setValue(offset);
    ui->dSpinBoxStep->setValue(step);

    int y_start = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_Y_START, "0").toInt();
    int y_end = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_Y_END, "0").toInt();
    ui->spinBoxStartY->setValue(y_start);
    ui->spinBoxEndY->setValue(y_end);
    int x_start = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_X_START, "0").toInt();
    int x_end = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_X_END, "0").toInt();
    ui->spinBoxStartX->setValue(x_start);
    ui->spinBoxEndX->setValue(x_end);
}

void FormPlot::setINI()
{
    int offset31 = ui->spinBox31Offset->value();
    int offset33 = ui->spinBox33Offset->value();
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET31, QString::number(offset31));
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET33, QString::number(offset33));
}

void FormPlot::init2d()
{
    m_series33 = new QLineSeries();
    m_series31 = new QLineSeries();
    m_peaks = new QScatterSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    m_chart->addSeries(m_series31);
    m_chart->addSeries(m_series33);
    m_chart->addSeries(m_peaks);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_series31->attachAxis(m_axisX);
    m_series31->attachAxis(m_axisY);
    m_series33->attachAxis(m_axisX);
    m_series33->attachAxis(m_axisY);
    m_series31->setColor(Qt::blue);
    m_series33->setColor(Qt::magenta);
    m_series31->setName(tr("curve31"));
    m_series33->setName(tr("curve33"));

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

    m_axisX->setTitleText(tr("index"));
    m_axisX->setRange(0, 0.2);
    m_axisY->setTitleText(tr("Voltage (V)"));
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
    m_chart->setTitle(tr("Live ADC Waveform"));

    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->stackedWidget->addWidget(m_chartView);
}

void FormPlot::initToolButtons()
{
    ui->tBtnCrop->setObjectName("crop");
    ui->tBtnCrop->setIconSize(QSize(24, 24));
    ui->tBtnCrop->setCheckable(true);
    ui->tBtnCrop->setChecked(m_enableCrop);
    ui->tBtnCrop->setToolTip(tr("crop"));
    ui->tBtnCrop->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnZoom->setObjectName("zoom");
    ui->tBtnZoom->setIconSize(QSize(24, 24));
    ui->tBtnZoom->setCheckable(true);
    ui->tBtnZoom->setChecked(m_autoZoom);
    ui->tBtnZoom->setToolTip(tr("Auto Zoom"));
    ui->tBtnZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnImgSave->setObjectName("img_save");
    ui->tBtnImgSave->setIconSize(QSize(24, 24));
    ui->tBtnImgSave->setToolTip(tr("image save (ctrl+s)"));
    ui->tBtnImgSave->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnFindPeak->setObjectName("find_peak");
    ui->tBtnFindPeak->setIconSize(QSize(24, 24));
    ui->tBtnFindPeak->setToolTip(tr("find peaks"));
    ui->tBtnFindPeak->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnPause->setObjectName("pause");
    ui->tBtnPause->setIconSize(QSize(24, 24));
    ui->tBtnPause->setToolTip(tr("pause"));
    ui->tBtnPause->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnFWHM->setObjectName("FWHM");
    ui->tBtnFWHM->setIconSize(QSize(24, 24));
    ui->tBtnFWHM->setToolTip(tr("FWHM"));
    ui->tBtnFWHM->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnFourier->setObjectName("Fourier");
    ui->tBtnFourier->setIconSize(QSize(24, 24));
    ui->tBtnFourier->setToolTip(tr("Fourier"));
    ui->tBtnFourier->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnSNR->setObjectName("SNR");
    ui->tBtnSNR->setIconSize(QSize(24, 24));
    ui->tBtnSNR->setToolTip(tr("Signal-to-noise ratio"));
    ui->tBtnSNR->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnDerivation->setObjectName("Derivation");
    ui->tBtnDerivation->setIconSize(QSize(24, 24));
    ui->tBtnDerivation->setToolTip(tr("Derivation"));
    ui->tBtnDerivation->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnAccumulate->setObjectName("Accumulate");
    ui->tBtnAccumulate->setIconSize(QSize(24, 24));
    ui->tBtnAccumulate->setToolTip(tr("Accumulate"));
    ui->tBtnAccumulate->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnTemperature->setObjectName("Temperature");
    ui->tBtnTemperature->setIconSize(QSize(24, 24));
    ui->tBtnTemperature->setToolTip(tr("Temperature"));
    ui->tBtnTemperature->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnOffset->setCheckable(true);
    ui->tBtnStep->setCheckable(true);
    ui->tBtnFindPeak->setCheckable(true);
    ui->tBtnPause->setCheckable(true);
    ui->tBtnFWHM->setCheckable(true);
    ui->tBtnRangeX->setCheckable(true);
    ui->tBtnRangeY->setCheckable(true);
    ui->tBtnFourier->setCheckable(true);
    ui->tBtnSNR->setCheckable(true);
    ui->tBtnAccumulate->setCheckable(true);
    ui->tBtnDerivation->setCheckable(true);
    ui->tBtnTemperature->setCheckable(true);
}

void FormPlot::init()
{
    init2d();
    initToolButtons();

    getINI();

    QShortcut *shortcut_ImgSave = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    connect(shortcut_ImgSave, &QShortcut::activated, this, &FormPlot::on_tBtnImgSave_clicked);
    m_trajectory = new PeakTrajectory;
    m_fourierTransform = new FourierTransform;
    m_derivation = new Derivation;
    m_accumulate = new Accumulate;
    m_snr = new SignalNoiseRatio;
    m_temperature = new TemperatureView;
    connect(m_trajectory, &PeakTrajectory::windowClose, this, [this]() {
        ui->checkBoxTrajectory->setChecked(false);
        on_checkBoxTrajectory_clicked();
    });
    connect(m_fourierTransform, &FourierTransform::windowClose, this, [this]() {
        m_enableFourier = false;
        ui->tBtnFourier->setChecked(false);
    });
    connect(m_derivation, &Derivation::windowClose, this, [this]() {
        m_enableDerivation = false;
        ui->tBtnDerivation->setChecked(false);
    });
    connect(m_accumulate, &Accumulate::windowClose, this, [this]() {
        m_enableAccumulate = false;
        ui->tBtnAccumulate->setChecked(false);
    });
    connect(m_snr, &SignalNoiseRatio::windowClose, this, [this]() {
        m_enableSNR = false;
        ui->tBtnSNR->setChecked(false);
    });
    connect(m_temperature, &TemperatureView::windowClose, this, [this]() {
        m_enableTemperature = false;
        ui->tBtnTemperature->setChecked(false);
    });
}

void FormPlot::onDataReceivedF30(const QByteArray &data31,
                                 const QByteArray &data33,
                                 const double &temperature)
{
    if (!m_pause) {
        emit newDataReceivedF30(data31, data33, temperature);
    }
}

void FormPlot::onDataReceivedF15(const QByteArray &data31,
                                 const QByteArray &data33,
                                 const double &temperature)
{
    if (!m_pause) {
        emit newDataReceivedF15(data31, data33, temperature);
    }
}

void FormPlot::updateAxis()
{
    double xMin = m_offset;
    double xMax = std::numeric_limits<double>::min();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::min();
    if (ui->tBtnRangeX->isChecked()) {
        m_axisX->setRange(ui->spinBoxStartX->value(), ui->spinBoxEndX->value());
    } else {
        xMax = std::max(m_series31->count(), m_series33->count());
        m_axisX->setRange(xMin, m_offset + xMax);
    }
    if (m_autoZoom) {
        for (int i = 0; i < m_series31->count(); ++i) {
            yMin = std::min(yMin, m_series31->at(i).y());
            yMax = std::max(yMax, m_series31->at(i).y());
        }
        for (int i = 0; i < m_series33->count(); ++i) {
            yMin = std::min(yMin, m_series33->at(i).y());
            yMax = std::max(yMax, m_series33->at(i).y());
        }
        double padding = (yMax - yMin) * 0.1;
        if (padding == 0) {
            padding = 0.1;
        }
        m_axisY->setRange(yMin - padding, yMax + padding);
    } else {
        m_axisY->setRange(ui->spinBoxStartY->value(), ui->spinBoxEndY->value());
    }
}

void FormPlot::updatePlot2d(const QList<QPointF> &data31, const QList<QPointF> &data33)
{
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

void FormPlot::updatePlot4k(const CURVE &curve31,
                            const CURVE &curve33,
                            const double &temperature,
                            bool record)
{
    if (m_pause) {
        return;
    }
    ui->labelTemperature->setText(QString("%1 ℃").arg(temperature, 0, 'f', 4));

    CURVE plot31 = curve31;
    CURVE plot33 = curve33;

    if (m_offset != 0) {
        for (int i = 0; i < curve31.data.size(); ++i) {
            plot31.data[i].setX(m_offset + i * m_step);
            plot31.data[i].setY(curve31.data[i].y());
        }
        for (int i = 0; i < curve33.data.size(); ++i) {
            plot33.data[i].setX(m_offset + i * m_step);
            plot33.data[i].setY(curve33.data[i].y());
        }
    }

    if (record) {
        emit toHistory(curve31, curve33, temperature);
    }

    if (m_enableFourier) {
        auto data = m_fourierTransform->transform(plot31.data);
        if (!data.isEmpty()) {
            plot31.data = data;
        }
    }

    if (m_enableDerivation) {
        m_derivation->derivation(plot31.data, plot33.data);
    }

    if (m_enableAccumulate) {
        auto data = m_accumulate->accumulate(plot31.data);
        if (!data.isEmpty()) {
            plot31.data = data;
        }
    }

    if (m_enableSNR) {
        m_snr->calculate(plot31.data);
    }

    if (m_enableTemperature) {
        m_temperature->appendTemperature(temperature);
    }

    updatePlot2d(plot31.data, plot33.data);
    callFindPeak();
    callCalcFWHM();
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

void FormPlot::closeEvent(QCloseEvent *event)
{
    if (m_trajectory) {
        m_trajectory->close();
    }
    if (m_fourierTransform) {
        m_fourierTransform->close();
    }
    if (m_derivation) {
        m_derivation->close();
    }
    if (m_accumulate) {
        m_accumulate->close();
    }
    if (m_snr) {
        m_snr->close();
    }
    if (m_temperature) {
        m_temperature->close();
    }
    this->close();
}

void FormPlot::on_tBtnCrop_clicked()
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

void FormPlot::on_tBtnZoom_clicked()
{
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);
    if (m_autoZoom) {
        ui->tBtnRangeY->setChecked(false);
        updateAxis();
    }
}

void FormPlot::on_spinBox31Offset_valueChanged(int val)
{
    emit sendOffset31(val);
    setINI();
}

void FormPlot::on_spinBox33Offset_valueChanged(int val)
{
    emit sendOffset33(val);
    setINI();
}

void FormPlot::on_comboBoxAlgorithm_currentIndexChanged(int index)
{
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, QString::number(index));
    emit changeFrameType(index);
}

void FormPlot::on_tBtnImgSave_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save Chart"),
                                                    "",
                                                    "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (!filePath.isEmpty()) {
        saveChartAsImage(filePath);
    }
}

void FormPlot::on_spinBoxStartX_valueChanged(int val)
{
    if (ui->tBtnRangeX->isChecked()) {
        m_axisX->setRange(val, m_axisX->max());
        SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_X_START, QString::number(val));
    }
}

void FormPlot::on_spinBoxEndX_valueChanged(int val)
{
    if (ui->tBtnRangeX->isChecked()) {
        m_axisX->setRange(m_axisX->min(), val);
        SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_X_END, QString::number(val));
    }
}

void FormPlot::on_spinBoxStartY_valueChanged(int val)
{
    if (ui->tBtnRangeY->isChecked()) {
        m_axisY->setRange(val, m_axisY->max());
        SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_Y_START, QString::number(val));
    }
}

void FormPlot::on_spinBoxEndY_valueChanged(int val)
{
    if (ui->tBtnRangeY->isChecked()) {
        m_axisY->setRange(m_axisY->min(), val);
        SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_Y_END, QString::number(val));
    }
}

void FormPlot::on_dSpinBoxStep_valueChanged(double val)
{
    if (ui->tBtnStep->isChecked()) {
        m_step = val;
    }
}

void FormPlot::on_tBtnStep_clicked()
{
    if (ui->tBtnStep->isChecked()) {
        m_step = ui->dSpinBoxStep->value();
        SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_STEP, QString::number(m_step));
    } else {
        m_step = 1;
    }
}

QVector<QPointF> FormPlot::findPeak(int window, double thresholdFactor, double minDist)
{
    QVector<QPointF> peaks;
    int n = m_series31->count();

    QVector<double> values;
    values.reserve(n);
    for (int i = 0; i < n; i++)
        values.append(m_series31->at(i).y());

    double mean = std::accumulate(values.begin(), values.end(), 0.0) / n;
    double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / n - mean * mean);
    double threshold = mean + thresholdFactor * stdev;

    double lastPeakX = -1e9;
    for (int i = window; i < n - window; i++) {
        double yCurr = m_series31->at(i).y();
        if (yCurr < threshold)
            continue;

        bool isPeak = true;
        for (int j = i - window; j <= i + window; j++) {
            if (m_series31->at(j).y() > yCurr) {
                isPeak = false;
                break;
            }
        }

        if (isPeak) {
            double xCurr = m_series31->at(i).x();
            if (xCurr - lastPeakX >= minDist) {
                peaks.append(m_series31->at(i));
                lastPeakX = xCurr;
            }
        }
    }
    return peaks;
}

void FormPlot::callCalcFWHM()
{
    if (m_findFWHM) {
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
            for (int i = m_series31->count() - 1; i >= 1; --i) {
                if (m_series31->at(i).x() >= xPeak)
                    continue;
                if (ui->spinBoxLimit->value() != 0
                    && (xPeak - m_series31->at(i).x() > ui->spinBoxLimit->value()))
                    break;
                double y1 = m_series31->at(i).y();
                double y2 = m_series31->at(i - 1).y();
                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_series31->at(i).x();
                    double x2 = m_series31->at(i - 1).x();
                    // 线性插值
                    xLeft = x1 + (yHalf - y1) * (x2 - x1) / (y2 - y1);
                    break;
                }
            }
            for (int i = 0; i < m_series31->count() - 1; ++i) {
                if (m_series31->at(i).x() <= xPeak)
                    continue;
                if (ui->spinBoxLimit->value() != 0
                    && (xPeak - m_series31->at(i).x() > ui->spinBoxLimit->value()))
                    break;

                double y1 = m_series31->at(i).y();
                double y2 = m_series31->at(i + 1).y();

                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_series31->at(i).x();
                    double x2 = m_series31->at(i + 1).x();
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

void FormPlot::peakTrajectory(const QVector<QPointF> &peaks)
{
    if (!ui->checkBoxTrajectory->isChecked() || peaks.isEmpty() || !m_series31)
        return;

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
    const int idxAlgorithm = ui->comboBoxAlgorithm->currentIndex();
    if (idxAlgorithm == static_cast<int>(SHOW_ALGORITHM::F15_CURVES)) {
        // 在曲线33中找到与该 X 坐标最接近的点
        double y = 0;
        if (m_series33->count() > xPeak) {
            y = m_series33->at(xPeak).y();
        }

        // 转换为 raw 值
        int raw = static_cast<int>((1 << 13) * 1.0 * y / 3.3);
        if (m_trajectory) {
            m_trajectory->appendPeak(raw);
        }
    } else if (idxAlgorithm == static_cast<int>(SHOW_ALGORITHM::F15_SINGLE)) {
        m_trajectory->appendPeak(maxPeak.rx());
    } else if (idxAlgorithm == static_cast<int>(SHOW_ALGORITHM::F30_SINGLE)) {
        m_trajectory->appendPeak(maxPeak.rx());
    } else if (idxAlgorithm == static_cast<int>(SHOW_ALGORITHM::F30_CURVES)) {
        // 在曲线33中找到与该 X 坐标最接近的点
        double y = 0;
        if (m_series33->count() + m_offset > xPeak) {
            y = m_series33->at(xPeak - m_offset).y();
        }

        // 转换为 raw 值
        int raw = y * 0x8000 / 3.3;
        if (m_trajectory) {
            m_trajectory->appendPeak(raw);
        }
    }
}

void FormPlot::callFindPeak()
{
    if (m_findPeak) {
        if (!m_series31 || m_series31->count() < 5) {
            return;
        }

        QVector<QPointF> peaks31 = findPeak(3, 1.0, 5.0);
        if (m_enablePeakTrajectory) {
            peakTrajectory(peaks31);
        }
        m_peaks->clear();
        auto test_31 = m_series31->points();
        auto test_33 = m_series33->points();
        for (const auto &pt : peaks31) {
            m_peaks->append(pt);
            const int idxAlgorithm = ui->comboBoxAlgorithm->currentIndex();
            if (idxAlgorithm == static_cast<int>(SHOW_ALGORITHM::F30_CURVES)) {
                // 在曲线33中找到与该 X 坐标最接近的点
                double y = 0;
                if (m_series33->count() > pt.x() - m_offset) {
                    y = m_series33->at(pt.x() - m_offset).y();
                }

                // 转换为 raw 值
                int raw = y * 0x8000 / 3.3;
                ui->textBrowser->append(
                    QString("peak[%1]-> V: %2, Raw: %3").arg(pt.x()).arg(y).arg(raw));
            }
        }

        m_chart->update();
    } else {
        m_peaks->clear();
    }
}

void FormPlot::on_tBtnFindPeak_clicked()
{
    m_findPeak = !m_findPeak;
    ui->tBtnFindPeak->setChecked(m_findPeak);

    callFindPeak();
}

void FormPlot::on_tBtnPause_clicked()
{
    m_pause = !m_pause;
    ui->tBtnPause->setChecked(m_pause);
}

void FormPlot::on_tBtnOffset_clicked()
{
    if (ui->tBtnOffset->isChecked()) {
        m_offset = ui->spinBoxOffset->value();
        m_trajectory_start += m_offset;
        m_trajectory_end += m_offset;
        SETTING_CONFIG_SET(CFG_GROUP_PLOT,
                           CFG_PLOT_OFFSET,
                           QString::number(ui->spinBoxOffset->value()));
    } else {
        m_trajectory_start -= ui->spinBoxOffset->value();
        m_trajectory_end -= ui->spinBoxOffset->value();
        m_offset = 0;
    }
}

void FormPlot::on_tBtnFWHM_clicked()
{
    m_findFWHM = !m_findFWHM;
    callCalcFWHM();
}

void FormPlot::on_checkBoxTrajectory_clicked()
{
    if (ui->checkBoxTrajectory->isChecked()) {
        m_enablePeakTrajectory = true;
        m_trajectory_start = m_axisX->min();
        m_trajectory_end = m_axisX->max();

        if (ui->tBtnFindPeak->isChecked() == false) {
            on_tBtnFindPeak_clicked();
        }

        QChart *chart = m_chartView->chart();
        QRectF plot = chart->plotArea();

        qreal leftX = plot.left();
        qreal rightX = plot.right();

        m_lineLeft = new DraggableLine(chart, leftX, Qt::green);
        m_lineRight = new DraggableLine(chart, rightX, Qt::darkGreen);
        connect(m_lineLeft, &DraggableLine::xValueChanged, this, [this](qreal x) {
            m_trajectory_start = x;
        });
        connect(m_lineRight, &DraggableLine::xValueChanged, this, [this](qreal x) {
            m_trajectory_end = x;
        });

        chart->scene()->addItem(m_lineLeft);
        chart->scene()->addItem(m_lineRight);
        m_trajectory->show();
    } else {
        m_enablePeakTrajectory = false;
        if (ui->tBtnFindPeak->isChecked() == true) {
            on_tBtnFindPeak_clicked();
        }

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

void FormPlot::on_tBtnRangeX_clicked()
{
    if (ui->tBtnRangeX->isChecked()) {
        m_axisX->setRange(ui->spinBoxStartX->value(), ui->spinBoxEndX->value());
    }
}

void FormPlot::on_tBtnRangeY_clicked()
{
    if (ui->tBtnRangeY->isChecked()) {
        m_axisY->setRange(ui->spinBoxStartY->value(), ui->spinBoxEndY->value());
        ui->tBtnZoom->setChecked(false);
        m_autoZoom = false;
    }
}

void FormPlot::on_tBtnFourier_clicked()
{
    m_enableFourier = !m_enableFourier;
    if (m_enableFourier) {
        m_fourierTransform->show();
    } else {
        m_fourierTransform->hide();
    }
}

void FormPlot::on_tBtnDerivation_clicked()
{
    m_enableDerivation = !m_enableDerivation;
    ui->tBtnDerivation->setChecked(m_enableDerivation);
    if (m_enableDerivation) {
        m_derivation->show();
    } else {
        m_derivation->hide();
    }
}

void FormPlot::on_tBtnAccumulate_clicked()
{
    m_enableAccumulate = !m_enableAccumulate;
    ui->tBtnAccumulate->setChecked(m_enableAccumulate);
    if (m_enableAccumulate) {
        m_accumulate->show();
    } else {
        m_accumulate->hide();
    }
}

void FormPlot::on_tBtnSNR_clicked()
{
    m_enableSNR = !m_enableSNR;
    ui->tBtnSNR->setChecked(m_enableSNR);
    if (m_enableSNR) {
        m_snr->show();
    } else {
        m_snr->hide();
    }
}

void FormPlot::on_tBtnTemperature_clicked()
{
    m_enableTemperature = !m_enableTemperature;
    ui->tBtnTemperature->setChecked(m_enableTemperature);
    if (m_enableTemperature) {
        m_temperature->show();
    } else {
        m_temperature->hide();
    }
}
