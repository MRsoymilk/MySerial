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
    m_scatter = new QScatterSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    m_chart->addSeries(m_series24);
    m_chart->addSeries(m_series14);
    m_chart->addSeries(m_scatter);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series24->attachAxis(m_axisX);
    m_series24->attachAxis(m_axisY);
    m_series14->attachAxis(m_axisX);
    m_series14->attachAxis(m_axisY);
    m_scatter->attachAxis(m_axisX);
    m_scatter->attachAxis(m_axisY);
    m_series24->setColor(Qt::blue);
    m_series14->setColor(Qt::magenta);
    m_scatter->setColor(Qt::red);
    m_scatter->setName("Series24 Peaks");

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
    ui->tBtnCrop->setObjectName("crop");
    ui->tBtnCrop->setIconSize(QSize(24, 24));
    ui->tBtnCrop->setCheckable(true);
    ui->tBtnCrop->setChecked(m_enableCrop);
    ui->tBtnCrop->setToolTip("crop");
    ui->tBtnCrop->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnZoom->setObjectName("zoom");
    ui->tBtnZoom->setIconSize(QSize(24, 24));
    ui->tBtnZoom->setCheckable(true);
    ui->tBtnZoom->setChecked(m_autoZoom);
    ui->tBtnZoom->setToolTip("Auto Zoom");
    ui->tBtnZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtn3D->setObjectName("3d");
    ui->tBtn3D->setIconSize(QSize(24, 24));
    ui->tBtn3D->setCheckable(true);
    ui->tBtn3D->setChecked(m_show3D);
    ui->tBtn3D->setToolTip("3D");
    ui->tBtn3D->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnImgSave->setObjectName("img_save");
    ui->tBtnImgSave->setIconSize(QSize(24, 24));
    ui->tBtnImgSave->setToolTip("image save (ctrl+s)");
    ui->tBtnImgSave->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnFindPeak->setObjectName("find_peak");
    ui->tBtnFindPeak->setIconSize(QSize(24, 24));
    ui->tBtnFindPeak->setToolTip("find peaks");
    ui->tBtnFindPeak->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnPause->setObjectName("pause");
    ui->tBtnPause->setIconSize(QSize(24, 24));
    ui->tBtnPause->setToolTip("pause");
    ui->tBtnPause->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnOffset->setCheckable(true);
    ui->tBtnStep->setCheckable(true);
    ui->tBtnFindPeak->setCheckable(true);
    ui->tBtnPause->setCheckable(true);
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

void FormPlot::onDataReceived4k(const QByteArray &data14, const QByteArray &data24)
{
    if (m_pause) {
        emit newDataReceived4k(data14, data24);
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
    m_series14->setName("curve14_bit");
    m_series24->replace(data24);
    m_series24->setName("curve24_bit");

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
    emit toHistory(offsetData14, offsetData24);
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
    } else {
        m_step = 1;
    }
}

void FormPlot::on_tBtnFindPeak_clicked()
{
    m_findPeak = !m_findPeak;
    ui->tBtnFindPeak->setChecked(m_findPeak);

    if (m_findPeak) {
        if (!m_series24 || m_series24->count() < 5) {
            return;
        }

        auto findPeaksRobust = [](QLineSeries *series,
                                  int window,
                                  double thresholdFactor,
                                  double minDist) {
            QVector<QPointF> peaks;
            int n = series->count();

            QVector<double> values;
            values.reserve(n);
            for (int i = 0; i < n; i++)
                values.append(series->at(i).y());

            double mean = std::accumulate(values.begin(), values.end(), 0.0) / n;
            double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
            double stdev = std::sqrt(sq_sum / n - mean * mean);
            double threshold = mean + thresholdFactor * stdev;

            double lastPeakX = -1e9;
            for (int i = window; i < n - window; i++) {
                double yCurr = series->at(i).y();
                if (yCurr < threshold)
                    continue;

                bool isPeak = true;
                for (int j = i - window; j <= i + window; j++) {
                    if (series->at(j).y() > yCurr) {
                        isPeak = false;
                        break;
                    }
                }

                if (isPeak) {
                    double xCurr = series->at(i).x();
                    if (xCurr - lastPeakX >= minDist) {
                        peaks.append(series->at(i));
                        lastPeakX = xCurr;
                    }
                }
            }
            return peaks;
        };

        QVector<QPointF> peaks24 = findPeaksRobust(m_series24, 3, 1.0, 5.0);

        if (m_scatter) {
            m_chart->removeSeries(m_scatter);
            delete m_scatter;
            m_scatter = nullptr;
        }
        for (auto label : m_peakLabels) {
            m_chartView->scene()->removeItem(label);
            delete label;
        }
        m_peakLabels.clear();

        if (!peaks24.isEmpty()) {
            m_scatter = new QScatterSeries();
            m_scatter->setName("Series24 Peaks");
            m_scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            m_scatter->setMarkerSize(10.0);
            m_scatter->setColor(Qt::red);

            for (const auto &pt : peaks24) {
                m_scatter->append(pt);

                QString text = QString("(%1, %2)").arg(pt.x(), 0, 'f', 2).arg(pt.y(), 0, 'f', 2);
                QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(text);
                label->setBrush(Qt::red);
                label->setFont(QFont("Arial", 10, QFont::Bold));
                label->setPos(m_chart->mapToPosition(pt) + QPointF(5, -15));
                m_chartView->scene()->addItem(label);
                m_peakLabels.append(label);
            }

            m_chart->addSeries(m_scatter);
            m_scatter->attachAxis(m_axisX);
            m_scatter->attachAxis(m_axisY);
        }

        m_chart->update();

    } else {
        if (m_scatter) {
            m_chart->removeSeries(m_scatter);
            delete m_scatter;
            m_scatter = nullptr;
        }
        for (auto label : m_peakLabels) {
            m_chartView->scene()->removeItem(label);
            delete label;
        }
        m_peakLabels.clear();
        m_chart->update();
    }
}

void FormPlot::on_tBtnPause_clicked()
{
    m_pause = !m_pause;
    ui->tBtnPause->setChecked(m_pause);
}
