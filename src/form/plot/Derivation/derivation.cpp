#include "derivation.h"
#include "MyChartView/mychartview.h"
#include "ui_derivation.h"
#include <Eigen/Dense>

using Eigen::ArrayXd;
using Eigen::VectorXd;

Derivation::Derivation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Derivation)
{
    ui->setupUi(this);
    initChart();
}

Derivation::~Derivation()
{
    delete ui;
}

void Derivation::initChart()
{
    m_chart = new QChart;
    m_chart->setTitle("Derivation");
    m_seriesData33 = new QLineSeries;
    m_seriesData33->setName("curve 33");
    m_seriesData33->setPen(QPen(Qt::magenta, 1));

    m_seriesDeriv = new QLineSeries;
    m_seriesDeriv->setName("Derivation 33");
    QPen penDeriv(Qt::red, 1);
    penDeriv.setStyle(Qt::DotLine);
    m_seriesDeriv->setPen(penDeriv);

    m_seriesData31 = new QLineSeries;
    m_seriesData31->setName("curve 31");
    m_seriesData31->setPen(QPen(Qt::blue, 2));

    m_chart->addSeries(m_seriesData33);
    m_chart->addSeries(m_seriesDeriv);
    m_chart->addSeries(m_seriesData31);

    m_chart->createDefaultAxes();

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayChart->addWidget(m_chartView);

    m_chartExtra = new QChart;
    m_chartExtra->setTitle("Extra");
    m_seriesExtra31 = new QLineSeries;
    m_seriesExtra31->setName("curve 31");
    m_seriesExtra31->setPen(QPen(Qt::blue, 1));
    m_seriesExtra33 = new QLineSeries;
    m_seriesExtra33->setName("curve 33");
    m_seriesExtra33->setPen(QPen(Qt::magenta, 1));
    m_chartExtra->addSeries(m_seriesExtra31);
    m_chartExtra->addSeries(m_seriesExtra33);

    m_axisExtraX = new QValueAxis();
    m_axisExtraY = new QValueAxis();
    m_chartExtra->addAxis(m_axisExtraX, Qt::AlignBottom);
    m_chartExtra->addAxis(m_axisExtraY, Qt::AlignLeft);

    m_chartViewExtra = new MyChartView(m_chartExtra, this);
    ui->gLayExtra->addWidget(m_chartViewExtra);

    ui->tBtnFindPeak->setCheckable(true);
    ui->spinBoxMinLength->setValue(1000);
    ui->horizontalSlider->setValue(1000);
    ui->plainTextEdit->setVisible(false);
}

void Derivation::derivation(const QList<QPointF> &data31, const QList<QPointF> &data33)
{
    m_lastData31 = data31;
    m_lastData33 = data33;

    m_seriesData33->clear();
    m_seriesDeriv->clear();
    m_seriesData31->clear();
    m_seriesExtra31->clear();
    m_seriesExtra33->clear();

    if (m_areaHighlight) {
        m_chart->removeSeries(m_areaHighlight);
        delete m_areaHighlight;
        m_areaHighlight = nullptr;
    }

    int count = qMin(data31.size(), data33.size());
    if (count < 2)
        return;

    VectorXd x(count), y31(count), y33(count);

    for (int i = 0; i < count; ++i) {
        x(i) = data33[i].x();
        y33(i) = data33[i].y();
        y31(i) = data31[i].y();
    }

    double data31MinY = y31.minCoeff() - 1.0;

    Eigen::VectorXd diff(count);
    diff(0) = 0.0;
    diff.tail(count - 1) = y33.tail(count - 1) - y33.head(count - 1);

    Eigen::Array<bool, Eigen::Dynamic, 1> falling = diff.array() < -0.0001; // 判定下降沿

    QList<QPointF> points33, points31, pointsDeriv;

    QLineSeries *upper = new QLineSeries;
    QLineSeries *lower = new QLineSeries;

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::min();
    int idx = 0;

    Eigen::Index maxIndex;
    double y33_max = y33.maxCoeff(&maxIndex);
    QVector<double> extra31, extra33;
    QVector<int> v_idx;
    for (int i = 0; i < count; ++i) {
        yMin = std::min(yMin, std::min(y31(i), y33(i)));
        yMax = std::max(yMax, std::max(y31(i), y33(i)));

        points33.append(QPointF(x(i), y33(i)));
        points31.append(QPointF(x(i), y31(i)));
        pointsDeriv.append(QPointF(x(i), diff(i)));

        upper->append(x(i), y33(i));
        double lowerY = falling(i) ? (y33.minCoeff()) : y33(i);
        lower->append(x(i), lowerY);
        if (falling(i)) {
            if (i > maxIndex) {
                m_seriesExtra31->append(idx, y31(i));
                m_seriesExtra33->append(idx, y33(i));
                extra31.append(y31(i));
                v_idx.append(i);
                idx++;
            }
        }
    }

    // 主图绘制
    m_seriesData33->replace(points33);
    m_seriesData31->replace(points31);
    m_seriesDeriv->replace(pointsDeriv);

    m_areaHighlight = new QAreaSeries(upper, lower);
    m_areaHighlight->setName("Data31 Highlight");
    m_areaHighlight->setPen(Qt::NoPen);
    m_areaHighlight->setBrush(QColor(0, 255, 0, 80));
    m_chart->addSeries(m_areaHighlight);

    m_chart->createDefaultAxes();

    double margin = (yMax - yMin) * 0.10;
    if (margin == 0)
        margin = 1.0;

    m_chart->axes(Qt::Vertical).first()->setRange(yMin - margin, yMax + margin);
    m_chart->axes(Qt::Horizontal).first()->setRange(x(0), x(count - 1));

    m_chartExtra->axes(Qt::Vertical).first()->setRange(yMin - margin, yMax + margin);
    m_chartExtra->axes(Qt::Horizontal).first()->setRange(0, idx);
    if (idx > 1000) {
        CURVE curve31;
        CURVE curve33;
        curve31.data = m_seriesExtra31->points();
        curve31.x_max = curve31.data.size();
        curve31.x_min = 0;
        curve31.y_min = yMin;
        curve31.y_max = yMax;
        curve33.data = m_seriesExtra33->points();
        curve33.x_max = curve31.data.size();
        curve33.x_min = 0;
        curve33.y_min = yMin;
        curve33.y_max = yMax;
        m_curve.push_back({curve31, curve33});
        ++m_current_extra;
        ui->labelPageExtra->setText(QString("%1/%2").arg(m_current_extra + 1).arg(m_curve.size()));
    }
}

void Derivation::closeEvent(QCloseEvent *event)
{
    emit windowClose();
}

void Derivation::updateExtraCurve()
{
    auto curves = m_curve[m_current_extra];
    m_seriesExtra31->replace(curves.curve31.data);
    m_seriesExtra33->replace(curves.curve33.data);
    m_chartExtra->axes(Qt::Vertical).first()->setRange(curves.curve31.y_min, curves.curve31.y_max);
    m_chartExtra->axes(Qt::Horizontal).first()->setRange(curves.curve33.x_min, curves.curve33.x_max);
    ui->labelPageExtra->setText(QString("%1/%2").arg(m_current_extra + 1).arg(m_curve.size()));
    if (m_enableFindPeak) {
        callFindPeak(curves.curve31.data, curves.curve33.data);
    }
}

void Derivation::on_tBtnNextExtra_clicked()
{
    if (m_current_extra - 1 >= 0) {
        --m_current_extra;
    } else {
        return;
    }
    updateExtraCurve();
}

void Derivation::on_tBtnPrevExtra_clicked()
{
    if (m_current_extra + 1 < m_curve.size()) {
        ++m_current_extra;
    } else {
        return;
    }
    updateExtraCurve();
}

void Derivation::on_horizontalSlider_valueChanged(int value)
{
    ui->spinBoxMinLength->blockSignals(true);
    ui->spinBoxMinLength->setValue(value);
    ui->spinBoxMinLength->blockSignals(false);
}

void Derivation::on_spinBoxMinLength_valueChanged(int value)
{
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value);
    ui->horizontalSlider->blockSignals(false);
}

void Derivation::callFindPeak(const QList<QPointF> &points31, const QList<QPointF> &points33)
{
    // 自动进入 checked 情况：开始找峰
    if (points31.isEmpty()) {
        return;
    }

    double maxVal = -1e12;
    int maxIdx = -1;

    for (int i = 0; i < points31.size(); ++i) {
        if (points31[i].y() > maxVal) {
            maxVal = points31[i].y();
            maxIdx = i;
        }
    }

    if (maxIdx < 0) {
        return;
    }

    double corresponding33 = points33[maxIdx].y();

    if (!m_peakMarker) {
        m_peakMarker = new QScatterSeries;
        m_peakMarker->setMarkerSize(5);
        m_peakMarker->setColor(Qt::red);
        m_peakMarker->setName("Peak");
        m_chartExtra->addSeries(m_peakMarker);

        m_peakMarker->attachAxis(m_axisExtraX);
        m_peakMarker->attachAxis(m_axisExtraY);
    }

    m_peakMarker->clear();
    m_peakMarker->append(points31[maxIdx].x(), maxVal);

    ui->plainTextEdit->appendPlainText(QString("Peak [%1] → 31: %2, 33: %3 = %4")
                                           .arg(maxIdx)
                                           .arg(maxVal)
                                           .arg(corresponding33)
                                           .arg(corresponding33 * 0x8000 / 2.5));
}

void Derivation::on_tBtnFindPeak_clicked()
{
    m_enableFindPeak = !m_enableFindPeak;
    ui->tBtnFindPeak->setChecked(m_enableFindPeak);
    ui->plainTextEdit->setVisible(m_enableFindPeak);
    if (m_enableFindPeak) {
        QList<QPointF> points31 = m_seriesExtra31->points();
        QList<QPointF> points33 = m_seriesExtra33->points();
        callFindPeak(points31, points33);
    } else {
        if (m_peakMarker) {
            m_chartExtra->removeSeries(m_peakMarker);
            delete m_peakMarker;
            m_peakMarker = nullptr;
        }
        ui->plainTextEdit->appendPlainText("Peak removed.");
        return;
    }
}
