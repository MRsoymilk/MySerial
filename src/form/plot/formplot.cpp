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
#include "DraggableLine/draggableline.h"
#include "PeakTrajectory/peaktrajectory.h"
#include "funcdef.h"

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
    ui->comboBoxAlgorithm->addItems({"normal", "max_neg_95", "num_660", "play_mpu6050"});
    ui->comboBoxAlgorithm->blockSignals(false);

    int algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
    ui->comboBoxAlgorithm->setCurrentIndex(algorithm);

    double offset = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_OFFSET, "0.0").toDouble();
    double step = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_STEP, "1.0").toDouble();
    ui->spinBoxOffset->setValue(offset);
    ui->dSpinBoxStep->setValue(step);
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
    m_peaks = new QScatterSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    m_chart->addSeries(m_series24);
    m_chart->addSeries(m_series14);
    m_chart->addSeries(m_peaks);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series24->attachAxis(m_axisX);
    m_series24->attachAxis(m_axisY);
    m_series14->attachAxis(m_axisX);
    m_series14->attachAxis(m_axisY);
    m_peaks->attachAxis(m_axisX);
    m_peaks->attachAxis(m_axisY);
    m_series24->setColor(Qt::blue);
    m_series14->setColor(Qt::magenta);
    m_peaks->setColor(Qt::red);
    m_peaks->setName(tr("Peaks"));
    m_peaks->setMarkerSize(5.0);
    m_peaks->setPointLabelsVisible(true);
    m_peaks->setPointLabelsClipping(false);
    m_peaks->setPointLabelsColor(Qt::red);
    m_peaks->setPointLabelsFont(QFont("Arial", 10, QFont::Bold));
    m_peaks->setPointLabelsFormat("(@xPoint, @yPoint)");

    m_axisX->setTitleText(tr("Time (s)"));
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

    ui->tBtnMeasureX->setVisible(false);
    ui->tBtnMeasureY->setVisible(false);
}

void FormPlot::init3d()
{
    m_glWidget = new MyGLCurveWidget();
    ui->stackedWidget->addWidget(m_glWidget);
    ui->stackedWidget->setCurrentWidget(m_chartView);
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

    ui->tBtn3D->setObjectName("3d");
    ui->tBtn3D->setIconSize(QSize(24, 24));
    ui->tBtn3D->setCheckable(true);
    ui->tBtn3D->setChecked(m_show3D);
    ui->tBtn3D->setToolTip(tr("3D"));
    ui->tBtn3D->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

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

    ui->tBtnOffset->setCheckable(true);
    ui->tBtnStep->setCheckable(true);
    ui->tBtnFindPeak->setCheckable(true);
    ui->tBtnPause->setCheckable(true);
    ui->tBtnFWHM->setCheckable(true);
}

void FormPlot::init()
{
    init2d();
    init3d();
    initToolButtons();

    getINI();

    QShortcut *shortcut_ImgSave = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    connect(shortcut_ImgSave, &QShortcut::activated, this, &FormPlot::on_tBtnImgSave_clicked);
}

void FormPlot::onDataReceived4k(const QByteArray &data14,
                                const QByteArray &data24,
                                const double &temperature)
{
    if (!m_pause) {
        emit newDataReceived4k(data14, data24, temperature);
    }
}

void FormPlot::updatePlot2d(const QList<QPointF> &data14,
                            const QList<QPointF> &data24,
                            const double &xMin,
                            const double &xMax,
                            const double &yMin,
                            const double &yMax)
{
    static bool flip = false;
    flip = !flip;
    if (flip) {
        ui->labelUpdateSign->setStyleSheet("background-color: green; color: white;");
    } else {
        ui->labelUpdateSign->setStyleSheet("background-color: blue; color: white;");
    }
    m_series14->replace(data14);
    m_series14->setName(tr("curve14_bit"));
    m_series24->replace(data24);
    m_series24->setName(tr("curve24_bit"));

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
                            const double &yMax,
                            const double &temperature,
                            bool record)
{
    if (m_pause) {
        return;
    }
    int offset = 0;
    if (ui->tBtnOffset->isChecked()) {
        offset = ui->spinBoxOffset->value();
    }

    QList<QPointF> offsetData14 = data14;
    QList<QPointF> offsetData24 = data24;
    for (int i = 0; i < data14.size(); ++i) {
        offsetData14[i].setX(offset + i * m_step);
        offsetData14[i].setY(data14[i].y());
    }
    for (int i = 0; i < data24.size(); ++i) {
        offsetData24[i].setX(offset + i * m_step);
        offsetData24[i].setY(data24[i].y());
    }
    if (record) {
        emit toHistory(offsetData14, offsetData24, temperature);
    }
    ui->labelTemperature->setText(QString("%1 ℃").arg(temperature));
    updatePlot2d(offsetData14,
                 offsetData24,
                 offset,
                 offset + std::max(data14.size(), data24.size()) * m_step,
                 yMin,
                 yMax);
    updatePlot3d(offsetData14,
                 offsetData24,
                 offset,
                 offset + std::max(data14.size(), data24.size()) * m_step,
                 yMin,
                 yMax);

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

void FormPlot::on_spinBox14Offset_valueChanged(int val)
{
    emit sendOffset14(val);
    setINI();
}

void FormPlot::on_spinBox24Offset_valueChanged(int val)
{
    emit sendOffset24(val);
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
                                                    "Save Chart",
                                                    "",
                                                    "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (!filePath.isEmpty()) {
        saveChartAsImage(filePath);
    }
}

void FormPlot::on_spinBoxFrom_valueChanged(int val)
{
    int to = ui->spinBoxTo->value();
    if (to == 0) {
        to = m_axisX->max();
    }
    if (val < to) {
        m_axisX->setRange(val, to);
    }
}

void FormPlot::on_spinBoxTo_valueChanged(int val)
{
    int from = ui->spinBoxFrom->value();
    if (from == 0) {
        from = m_axisX->min();
    }
    if (from < val) {
        m_axisX->setRange(from, val);
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
    int n = m_series24->count();

    QVector<double> values;
    values.reserve(n);
    for (int i = 0; i < n; i++)
        values.append(m_series24->at(i).y());

    double mean = std::accumulate(values.begin(), values.end(), 0.0) / n;
    double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / n - mean * mean);
    double threshold = mean + thresholdFactor * stdev;

    double lastPeakX = -1e9;
    for (int i = window; i < n - window; i++) {
        double yCurr = m_series24->at(i).y();
        if (yCurr < threshold)
            continue;

        bool isPeak = true;
        for (int j = i - window; j <= i + window; j++) {
            if (m_series24->at(j).y() > yCurr) {
                isPeak = false;
                break;
            }
        }

        if (isPeak) {
            double xCurr = m_series24->at(i).x();
            if (xCurr - lastPeakX >= minDist) {
                peaks.append(m_series24->at(i));
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
            for (int i = m_series24->count() - 1; i >= 1; --i) {
                if (m_series24->at(i).x() >= xPeak)
                    continue;
                double y1 = m_series24->at(i).y();
                double y2 = m_series24->at(i - 1).y();
                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_series24->at(i).x();
                    double x2 = m_series24->at(i - 1).x();
                    // 线性插值
                    xLeft = x1 + (yHalf - y1) * (x2 - x1) / (y2 - y1);
                    break;
                }
            }
            for (int i = 0; i < m_series24->count() - 1; ++i) {
                if (m_series24->at(i).x() <= xPeak)
                    continue;
                double y1 = m_series24->at(i).y();
                double y2 = m_series24->at(i + 1).y();
                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_series24->at(i).x();
                    double x2 = m_series24->at(i + 1).x();
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

void FormPlot::peakTrajectory(const QVector<QPointF> &peaks24)
{
    if (!ui->checkBoxTrajectory->isChecked() || peaks24.isEmpty() || !m_series14)
        return;

    // 找到 peaks24 中 Y 最大的点
    QPointF maxPeak = QPointF(0, std::numeric_limits<float>::lowest());
    for (const QPointF &p : peaks24) {
        if (p.x() < m_trajectory_start || p.x() > m_trajectory_end) {
            continue;
        }
        if (p.y() > maxPeak.y()) {
            maxPeak = p;
        }
    }

    int xPeak = maxPeak.x();

    // 在曲线14中找到与该 X 坐标最接近的点
    double y14 = 0;
    if (m_series14->count() > xPeak) {
        y14 = m_series14->at(xPeak).y();
    }

    // 转换为 raw 值
    int raw = static_cast<int>((1 << 13) * 1.0 * y14 / 3.3);
    if (m_trajectory) {
        m_trajectory->appendPeak(raw);
    }
}

void FormPlot::callFindPeak()
{
    if (m_findPeak) {
        if (!m_series24 || m_series24->count() < 5) {
            return;
        }

        QVector<QPointF> peaks24 = findPeak(3, 1.0, 5.0);
        if (ui->checkBoxTrajectory->isChecked()) {
            peakTrajectory(peaks24);
        }
        m_peaks->clear();
        for (const auto &pt : peaks24) {
            m_peaks->append(pt);
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
        SETTING_CONFIG_SET(CFG_GROUP_PLOT,
                           CFG_PLOT_OFFSET,
                           QString::number(ui->spinBoxOffset->value()));
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
        if (!m_trajectory) {
            m_trajectory = new PeakTrajectory;
            connect(m_trajectory, &QObject::destroyed, this, [this](QObject *) {
                ui->checkBoxTrajectory->setChecked(false);
                m_trajectory = nullptr;
            });
            m_trajectory->show();
        }

        QChart *chart = m_chartView->chart();
        QRectF plot = chart->plotArea();

        qreal leftX = plot.left();
        qreal rightX = plot.right();
        m_trajectory_start = leftX;
        m_trajectory_end = rightX;

        m_lineLeft = new DraggableLine(chart, leftX, Qt::green);
        m_lineRight = new DraggableLine(chart, rightX, Qt::darkGreen);
        connect(m_lineLeft, &DraggableLine::xValueChanged, this, [this](qreal x) {
            qDebug() << "Left line X:" << x;
            m_trajectory_start = x;
        });
        connect(m_lineRight, &DraggableLine::xValueChanged, this, [this](qreal x) {
            qDebug() << "Right line X:" << x;
            m_trajectory_end = x;
        });

        chart->scene()->addItem(m_lineLeft);
        chart->scene()->addItem(m_lineRight);
    } else {
        if (m_trajectory) {
            m_trajectory->close();
            m_trajectory->deleteLater();
        }
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
