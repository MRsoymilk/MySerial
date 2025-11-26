#include "signalnoiseratio.h"
#include "MyChartView/mychartview.h"
#include "ui_signalnoiseratio.h"

SignalNoiseRatio::SignalNoiseRatio(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SignalNoiseRatio)
{
    ui->setupUi(this);
    init();
}

SignalNoiseRatio::~SignalNoiseRatio()
{
    delete ui;
}

void SignalNoiseRatio::calculate(const QList<QPointF> &data)
{
    if (data.size() < 10) {
        ui->lineEditSNRLinear->setText("Insufficient data");
        return;
    }

    QVector<double> yValues;
    yValues.reserve(data.size());
    for (const auto &p : data)
        yValues.append(p.y());

    double signalValue = 0.0;
    int signalIndex = 0;
    if (m_enableIdx) {
        signalIndex = qBound(0, m_idx, yValues.size() - 1);
        signalValue = yValues[signalIndex];
    } else {
        auto itMax = std::max_element(yValues.begin(), yValues.end());
        signalValue = *itMax;
        signalIndex = std::distance(yValues.begin(), itMax);
    }

    const int excludeRadius = 5;

    QVector<double> noise;
    noise.reserve(yValues.size());
    for (int i = 0; i < yValues.size(); ++i) {
        if (qAbs(i - signalIndex) > excludeRadius)
            noise.append(yValues[i]);
    }

    double mean = 0;
    for (double v : noise)
        mean += v;
    mean /= noise.size();

    double variance = 0;
    for (double v : noise)
        variance += (v - mean) * (v - mean);
    variance /= noise.size();

    double noiseStd = qSqrt(variance);
    if (noiseStd <= 0.0)
        noiseStd = 1e-12;

    double snrLinear = signalValue / noiseStd;
    double snrDb = 20.0 * qLn(snrLinear) / qLn(10.0);

    ui->lineEditPeakValue->setText(
        QString("Index: %1, Value: %2").arg(signalIndex).arg(signalValue, 0, 'f', 4));
    ui->lineEditNoiseValue->setText(QString::number(noiseStd, 'f', 4));
    ui->lineEditSNRLinear->setText(QString::number(snrLinear, 'f', 4));
    ui->lineEditSNRdB->setText(QString::number(snrDb, 'f', 2) + " dB");

    m_data.push_back(signalValue);
    m_line->append(m_data.size(), signalValue);
    m_axisX->setRange(0, m_data.size());
    double ymin = std::min(m_axisY->min(), signalValue);
    double ymax = std::max(m_axisY->max(), signalValue);
    m_axisY->setRange(ymin, ymax);
}

void SignalNoiseRatio::on_checkBoxUseIdx_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::Checked) {
        m_enableIdx = true;
        m_idx = ui->spinBoxIdx->value();
    } else {
        m_enableIdx = false;
    }
}

void SignalNoiseRatio::init()
{
    m_line = new QLineSeries();

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
    m_axisY->setRange(0, 0);
    m_chart->setTitle(tr("value trajectory"));
    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayChart->addWidget(m_chartView);

    ui->labelSNR_linear->setToolTip("peak / noise(std)");
}

void SignalNoiseRatio::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);

    QAction *clearChartAction = menu.addAction(tr("Clear Chart"));

    QAction *selectedAction = menu.exec(event->globalPos());
    if (!selectedAction)
        return;

    if (selectedAction == clearChartAction) {
        m_data.clear();
        m_line->clear();

        m_axisX->setRange(0, m_data.size() + 100);
        m_axisY->setRange(0, 0);
    }
}
