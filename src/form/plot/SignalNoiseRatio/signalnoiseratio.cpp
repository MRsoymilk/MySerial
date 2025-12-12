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

    QVector<double> y;
    y.reserve(data.size());
    for (const auto &p : data)
        y.append(p.y());

    int signalIndex = 0;
    double signalValue = 0.0;

    if (m_enableIdx) {
        signalIndex = qBound(0, m_idx, y.size() - 1);
        signalValue = y[signalIndex];
    } else {
        auto itMax = std::max_element(y.begin(), y.end());
        signalValue = *itMax;
        signalIndex = std::distance(y.begin(), itMax);
    }

    const int excludeRadius = 10;
    QVector<double> noise;
    noise.reserve(y.size());

    for (int i = 0; i < y.size(); i++) {
        if (qAbs(i - signalIndex) > excludeRadius)
            noise.append(y[i]);
    }

    if (noise.size() < 2) {
        ui->lineEditSNRLinear->setText("Noise insufficient");
        return;
    }

    double meanNoise = std::accumulate(noise.begin(), noise.end(), 0.0) / noise.size();

    double variance = 0.0;
    for (double v : noise)
        variance += (v - meanNoise) * (v - meanNoise);

    variance /= (noise.size() - 1);
    double noiseStd = std::sqrt(variance);
    if (noiseStd <= 0)
        noiseStd = 1e-12;

    double snrLinear = signalValue / noiseStd;
    double snrDb = 20.0 * std::log10(snrLinear);

    ui->lineEditPeakValue->setText(
        QString("Index=%1, Value=%2").arg(signalIndex).arg(signalValue, 0, 'f', 4));
    ui->lineEditNoiseValue->setText(QString::number(noiseStd, 'f', 4));
    ui->lineEditSNRLinear->setText(QString::number(snrLinear, 'f', 2));
    ui->lineEditSNRdB->setText(QString::number(snrDb, 'f', 2) + " dB");

    m_dataSignal.push_back(signalValue);
    m_dataNoise.push_back(noiseStd);
    {
        // ========== 计算平均 Signal ==========
        double meanSignal = 0.0;
        double meanNoise = 0.0;

        double sumSig = 0, sumNoise = 0;
        for (int i = 0; i < m_dataSignal.size(); i++) {
            sumSig += m_dataSignal[i];
            sumNoise += m_dataNoise[i];
        }
        meanSignal = sumSig / m_dataSignal.size();
        meanNoise = sumNoise / m_dataNoise.size();

        // ========== 计算 Signal 标准差 ==========
        double varSignal = 0.0;
        for (double v : m_dataSignal)
            varSignal += (v - meanSignal) * (v - meanSignal);
        varSignal /= qMax(1, m_dataSignal.size() - 1);
        double stdSignal = std::sqrt(varSignal);

        // ========== 计算 Noise 标准差 ==========
        double varNoise = 0.0;
        for (double v : m_dataNoise)
            varNoise += (v - meanNoise) * (v - meanNoise);
        varNoise /= qMax(1, m_dataNoise.size() - 1);
        double stdNoise = std::sqrt(varNoise);

        // ========== 计算平均 SNR ==========
        double avgSNR = 0.0;
        for (int i = 0; i < m_dataSignal.size(); i++)
            avgSNR += m_dataSignal[i] / m_dataNoise[i];
        avgSNR /= m_dataSignal.size();
        double avgSNRdB = 20.0 * std::log10(avgSNR);

        ui->labelAvgInfo->setText(QString("Count: %1\n"
                                          "Avg Signal: %2 (Std: %3)\n"
                                          "Avg Noise : %4 (Std: %5)\n"
                                          "Avg SNR   : %6\n"
                                          "SNR: %7")
                                      .arg(m_dataSignal.size())
                                      .arg(meanSignal, 0, 'f', 4)
                                      .arg(stdSignal, 0, 'f', 4)
                                      .arg(meanNoise, 0, 'f', 4)
                                      .arg(stdNoise, 0, 'f', 4)
                                      .arg(avgSNR, 0, 'f', 2)
                                      .arg(meanSignal / stdNoise));
    }
    m_line->append(m_dataSignal.size(), signalValue);
    m_axisX->setRange(0, m_dataSignal.size());
    m_axisY->setRange(std::min(m_axisY->min(), signalValue), std::max(m_axisY->max(), signalValue));
}

void SignalNoiseRatio::closeEvent(QCloseEvent *event)
{
    emit windowClose();
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
        m_dataSignal.clear();
        m_dataNoise.clear();
        m_line->clear();

        m_axisX->setRange(0, m_dataSignal.size() + 100);
        m_axisY->setRange(0, 0);
    }
}
