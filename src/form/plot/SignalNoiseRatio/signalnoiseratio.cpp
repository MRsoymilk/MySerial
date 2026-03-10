#include "signalnoiseratio.h"
#include "MyChartView/mychartview.h"
#include "ui_signalnoiseratio.h"
#include "keydef.h"
#include "funcdef.h"

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
        ui->lineEditSignalNoiseSNRLinear->setText(tr("Insufficient data"));
        return;
    }

    QVector<double> y;
    y.reserve(data.size());
    for (const auto &p : data)
        y.append(p.y());

    int signalIndex = 0;
    double signalValue = 0.0;

    // ===== 找到信号峰值 =====
    if (m_enableIdx) {
        signalIndex = qBound(0, m_idx, y.size() - 1);
        signalValue = y[signalIndex];
    } else {
        auto itMax = std::max_element(y.begin(), y.end());
        signalValue = *itMax;
        signalIndex = std::distance(y.begin(), itMax);
    }
    m_vSignal.push_back(signalValue);
    ui->lineEditPeakValue->setText(tr("Index=%1, Value=%2").arg(signalIndex).arg(signalValue, 0, 'f', 4));

    if(m_tab_idx == MODE_SIGNAL_NOISE) {
        // ===== 提取噪声区域 =====
        const int excludeRadius = ui->spinBoxExcludeRadius->value();
        QVector<double> noise;
        noise.reserve(y.size());

        for (int i = 0; i < y.size(); i++) {
            if (qAbs(i - signalIndex) > excludeRadius)
                noise.append(y[i]);
        }

        if (noise.size() < 2) {
            ui->lineEditSignalNoiseSNRLinear->setText(tr("Noise insufficient"));
            return;
        }

        double noise_avg = std::accumulate(noise.begin(), noise.end(), 0.0) / noise.size();
        double variance = 0.0;
        for (double v : noise) {
            variance += (v - noise_avg) * (v - noise_avg);
        }

        variance /= (noise.size() - 1);

        double noise_std = std::sqrt(variance);

        if (noise_std <= 1e-12)
            noise_std = 1e-12;

        m_vNoiseStd.append(noise_std);

        double snrLinear = signalValue / noise_std;
        double snrDb = 20.0 * std::log10(snrLinear);

        ui->lineEditSignalNoiseNoiseStd->setText(QString::number(noise_std, 'f', 2));
        ui->lineEditSignalNoiseSNRLinear->setText(QString::number(snrLinear, 'f', 2));
        ui->lineEditSignalNoiseSNRdB->setText(QString::number(snrDb, 'f', 2) + " dB");
    }
    else if(m_tab_idx == MODE_SIGNAL_SIGNAL) {
        double val_avg = 0;
        for(auto val : m_vSignal) {
            val_avg += val;
        }
        val_avg /= m_vSignal.size();
        double val_std = 0;
        for(int i = 0; i < m_vSignal.size(); ++i) {
            val_std = std::sqrt((m_vSignal[i] - val_avg) * (m_vSignal[i] - val_avg) / m_vSignal.size());
        }
        double SNR = val_avg / val_std;
        ui->lineEditSignalSignalAvgSignal->setText(QString::number(val_avg));
        ui->lineEditSignalSignalStdSignal->setText(QString::number(val_std));
        ui->lineEditSignalSignalSNR->setText(QString::number(SNR));
    }

    // ===== 曲线更新 =====
    m_line->append(m_vSignal.size(), signalValue);
    m_axisX->setRange(0, m_vSignal.size());
    m_axisY->setRange(
        std::min(m_axisY->min(), signalValue),
        std::max(m_axisY->max(), signalValue));
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

    ui->labelSNR_linear->setToolTip(tr("peak / noise(std)"));
    m_tab_idx = static_cast<IDX_SNR>(SETTING_CONFIG_GET(CFG_GROUP_SNR, CFG_SNR_TAB_IDX, "1").toInt());
    if(m_tab_idx == MODE_SIGNAL_NOISE) {
        ui->tabWidget->setCurrentWidget(ui->tabSignalNoise);
    }
    else if(m_tab_idx == MODE_SIGNAL_SIGNAL) {
        ui->tabWidget->setCurrentWidget(ui->tabSignalSignal);
    }
    int exclude_radius = SETTING_CONFIG_GET(CFG_GROUP_SNR, CFG_SNR_EXCLUDE_RADIUS, "10").toInt();
    ui->spinBoxExcludeRadius->setValue(exclude_radius);
}

void SignalNoiseRatio::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);

    QAction *clearChartAction = menu.addAction(tr("Clear Chart"));
    menu.addSeparator();
    QAction *exportChartAction = menu.addAction(tr("Export Chart"));

    QAction *selectedAction = menu.exec(event->globalPos());
    if (!selectedAction)
        return;

    if (selectedAction == clearChartAction) {
        clearChart();
    } else if(selectedAction == exportChartAction) {
        exportChart();
    }
}

void SignalNoiseRatio::clearChart() {
    m_vSignal.clear();
    m_vNoiseStd.clear();
    m_line->clear();

    m_axisX->setRange(0, m_vSignal.size() + 100);
    m_axisY->setRange(0, 0);
}

void SignalNoiseRatio::exportChart() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export CSV"),
        "snr_data.csv",
        tr("CSV Files (*.csv)")
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, TITLE_ERROR, tr("Cannot open file."));
        return;
    }

    QTextStream out(&file);

    if(m_tab_idx == MODE_SIGNAL_NOISE) {
        out << "Index,Signal,NoiseStd\n";
        for (int i = 0; i < m_vSignal.size(); i++) {
            double signal = m_vSignal[i];
            double noise = m_vNoiseStd[i];
            out << i + 1 << ","
                << signal << ","
                << noise << "\n";
        }
    }
    else if(m_tab_idx == MODE_SIGNAL_SIGNAL){
        out << "Index,Signal\n";
        for (int i = 0; i < m_vSignal.size(); i++) {
            double signal = m_vSignal[i];
            out << i + 1 << ","
                << signal << "\n";
        }
    }
    file.close();

    QMessageBox::information(this,
                             TITLE_INFO,
                             tr("CSV exported successfully."));
}

void SignalNoiseRatio::on_tabWidget_currentChanged(int index)
{
    m_tab_idx = static_cast<IDX_SNR>(index);
    SETTING_CONFIG_SET(CFG_GROUP_SNR, CFG_SNR_TAB_IDX, QString::number(index));
    clearChart();
}


void SignalNoiseRatio::on_spinBoxExcludeRadius_textChanged(const QString &val)
{
    SETTING_CONFIG_SET(CFG_GROUP_SNR, CFG_SNR_EXCLUDE_RADIUS, val);
}

