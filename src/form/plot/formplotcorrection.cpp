#include "formplotcorrection.h"
#include <QDir>
#include <QFile>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "funcdef.h"
#include "ui_formplotcorrection.h"

FormPlotCorrection::FormPlotCorrection(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlotCorrection)
{
    ui->setupUi(this);
    init();
}

FormPlotCorrection::~FormPlotCorrection()
{
    delete ui;
}

void FormPlotCorrection::closeEvent(QCloseEvent *event)
{
    emit windowClose();
    QWidget::closeEvent(event);
}

QVector<double> FormPlotCorrection::smoothCenteredMovingAverage(const QVector<double> &data,
                                                                int window)
{
    QVector<double> result = data;
    int N = data.size();
    int half = window / 2;

    for (int i = half; i < N - half; ++i) {
        double sum = 0;
        for (int j = -half; j <= half; ++j) {
            sum += data[i + j];
        }
        result[i] = sum / (window);
    }

    return result;
}

QByteArray FormPlotCorrection::wrapKB(const float &k, const float &b)
{
    QByteArray packet;

    // 包头
    packet.append(char(0xDD));
    packet.append(char(0x3C));

    // 数据长度（2字节，大端）
    uint16_t dataLength = 0x13;
    packet.append(char((dataLength >> 8) & 0xFF));
    packet.append(char(dataLength & 0xFF));

    // 指令
    packet.append(char(0x42));

    // k 转换为整数和小数部分
    int32_t k_int = static_cast<int32_t>(k);
    uint32_t k_frac = static_cast<uint32_t>(std::fabs(k - k_int) * 100.0f);

    packet.append(char((k_int >> 24) & 0xFF));
    packet.append(char((k_int >> 16) & 0xFF));
    packet.append(char((k_int >> 8) & 0xFF));
    packet.append(char(k_int & 0xFF));

    packet.append(char((k_frac >> 24) & 0xFF));
    packet.append(char((k_frac >> 16) & 0xFF));
    packet.append(char((k_frac >> 8) & 0xFF));
    packet.append(char(k_frac & 0xFF));

    // b 转换为整数和小数部分
    int32_t b_int = static_cast<int32_t>(b);
    uint32_t b_frac = static_cast<uint32_t>(std::fabs(b - b_int) * 100.0f);

    packet.append(char((b_int >> 24) & 0xFF));
    packet.append(char((b_int >> 16) & 0xFF));
    packet.append(char((b_int >> 8) & 0xFF));
    packet.append(char(b_int & 0xFF));

    packet.append(char((b_frac >> 24) & 0xFF));
    packet.append(char((b_frac >> 16) & 0xFF));
    packet.append(char((b_frac >> 8) & 0xFF));
    packet.append(char(b_frac & 0xFF));

    // 包尾
    packet.append(char(0xCD));
    packet.append(char(0xFF));

    return packet;
}

void FormPlotCorrection::onEpochCorrection(const QVector<double> &v14, const QVector<double> &v24)
{
    if (!m_start) {
        ui->btnStart->setStyleSheet("");
        return;
    }
    QString status = QString("===== %1/%2 =====").arg(++m_current_round).arg(m_ini.round);
    LOG_INFO("status: {}", status);
    ui->labelStatus->setText(status);
    ui->textEdit->append(status);
    bool bSend = false;
    if (m_current_round >= m_ini.round) {
        ui->btnStart->setStyleSheet("");
        bSend = true;
        m_start = false;
    }

    // find v14 {min, max, idx_min, idx_max}
    double min_v14 = std::numeric_limits<double>::max();
    double max_v14 = std::numeric_limits<double>::lowest();
    int idx_min = -1;
    int idx_max = -1;

    for (int i = 0; i < v14.size(); ++i) {
        if (v14[i] < min_v14) {
            min_v14 = v14[i];
            idx_min = i;
        }
        if (v14[i] > max_v14) {
            max_v14 = v14[i];
            idx_max = i;
        }
    }
    ui->textEdit->append("curve14:");
    ui->textEdit->append(QString("max %1 at %2").arg(max_v14).arg(idx_max));
    ui->textEdit->append(QString("min %1 at %2").arg(min_v14).arg(idx_min));
    LOG_INFO("curve14:\n{}\n{}",
             QString("max %1 at %2").arg(max_v14).arg(idx_max),
             QString("min %1 at %2").arg(min_v14).arg(idx_min));
    // find v24[idx_min] -> v24[idx_max] peak
    ui->textEdit->append("curve24:");
    LOG_INFO("curve24:");
    QVector<double> peak;
    QVector<int> peak_location;
    bool isPeekFound = false;
    int index_peak = 1;
    if (idx_min > idx_max) {
        std::swap(idx_min, idx_max);
    }

    const int window = 3;               // 平滑窗口大小
    const int min_distance = 3;         // 峰值间最小间隔
    const double min_prominence = 0.05; // 峰值最低突出度

    // 中值滤波平滑
    QVector<double> smoothed = smoothCenteredMovingAverage(v24, min_distance);

    static int file_idx = 1;
    QDir dir;
    if (!dir.exists("correction")) {
        dir.mkpath("correction");
    }
    QFile file(QString("correction/smoothed_%1.csv").arg(file_idx++));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "index,original,smoothed\n";
        for (int i = idx_min; i <= idx_max; ++i) {
            out << i << "," << v24[i] << "," << smoothed[i] << "\n";
        }
        file.close();
        qDebug() << "Saved smoothed data to smoothed.csv";
    } else {
        qWarning() << "Failed to open file for writing!";
    }

    QVector<QPointF> v_14_correspond;
    // 查找尖峰
    int last_peak_index = -min_distance;
    for (int i = idx_min + 1; i < idx_max - 1; ++i) {
        if (smoothed[i] > smoothed[i - 1] && smoothed[i] > smoothed[i + 1]) {
            // 检查是否满足最小突出度
            double left = smoothed[i - 1];
            double right = smoothed[i + 1];
            double base = (left + right) / 2.0;
            double prominence = smoothed[i] - base;

            if (prominence >= min_prominence && i - last_peak_index >= min_distance) {
                peak.append(v24[i]); // 使用原始数据作为峰值
                peak_location.append(i);
                last_peak_index = i;
                QString msg = QString("peak_%1: %2 at %3").arg(index_peak).arg(v24[i]).arg(i);
                ui->textEdit->append(msg);
                LOG_INFO(msg);
                index_peak++;
                isPeekFound = true;
                QPointF d14_correspond;
                d14_correspond.setX(i);
                d14_correspond.setY(v14[i]);
                v_14_correspond.push_back(d14_correspond);
            }
        }
    }
    m_v14.push_back(v_14_correspond);

    if (!isPeekFound) {
        QString msg = "no peak found!";
        ui->textEdit->append(msg);
        LOG_WARN(msg);
    }
    // try to fitting y = kx + b
    QString msg = "try to fitting y = kx + b";
    ui->textEdit->append(msg);
    LOG_INFO(msg);
    float avg_k = 0.0;
    float avg_b = 0.0;
    int avg_count = 0;
    for (int i = 0; i < m_v14.size(); ++i) {
        int peak_size = m_v14.at(i).size();
        if (peak_size < 2) {
            QString msg = QString("group %1 is invalid").arg(i + 1);
            ui->textEdit->append(msg);
            LOG_WARN(msg);
            continue;
        }
        QPointF p1 = m_v14[i].front();
        QPointF p2 = m_v14[i].back();
        float k = (p2.y() - p1.y()) / (p2.x() - p1.x());
        float b = p1.y() - k * p1.x();
        float k_conversion = k / 3.3 * 0x1FFF;
        float b_conversion = b / 3.3 * 0x1FFF;
        avg_k += k_conversion;
        avg_b += b_conversion;
        avg_count += 1;
        QString msg = QString("k: %1 -> %2\n"
                              "b: %3 -> %4")
                          .arg(k)
                          .arg(k_conversion)
                          .arg(b)
                          .arg(b_conversion);
        ui->textEdit->append(msg);
        LOG_INFO(msg);
    }
    avg_k /= avg_count;
    avg_b /= avg_count;
    // y = avg_k * x + avg_b
    drawKB(avg_k, avg_b);

    ui->labelValK->setText(QString("%1").arg(avg_k));
    ui->labelValB->setText(QString("%1").arg(avg_b));

    if (bSend) {
        auto packet = wrapKB(avg_k, avg_b);
        emit sendKB(packet);
        LOG_INFO("send kb bytes: {}", packet.toHex());

        QString msg = QString("send k: %1, b: %2").arg(avg_k).arg(avg_b);
        ui->textEdit->append(msg);
        LOG_INFO(msg);
    }
    QCoreApplication::processEvents();
}

void FormPlotCorrection::init()
{
    m_current_round = 0;
    m_start = false;
    ui->comboBoxAlgorithm->addItem("fitting_kb");
    int round = SETTING_CONFIG_GET(CFG_GROUP_CORRECTION, CFG_CORRECTION_ROUND, "8").toInt();
    ui->spinBoxRound->setValue(round);
    m_ini.round = round;
    QStringList algorithm = SETTING_CONFIG_GET(CFG_GROUP_CORRECTION,
                                               CFG_CORRECTION_ALGORITHM,
                                               "fitting_kb")
                                .split(",");
    ui->comboBoxAlgorithm->addItems(algorithm);

    drawKB();
}

void FormPlotCorrection::drawKB(const float &k, const float &b)
{
    QLineSeries *series = new QLineSeries();
    for (double x = -1.0; x <= 1.0; x += 0.01) {
        series->append(x, k * x + b);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString("fitting y = %1 * x + %2").arg(k).arg(b));

    // 设置X轴范围为 [-1, 1]
    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(-1.0, 1.0);
    axisX->setTitleText("X");

    // 根据 y = kx + b 计算Y轴范围
    double y_min = k * (-1.0) + b;
    double y_max = k * 1.0 + b;
    if (y_min > y_max)
        std::swap(y_min, y_max);
    double y_padding = (y_max - y_min) * 0.1 + 0.01; // 添加 10% 余量，避免上下贴边
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(y_min - y_padding, y_max + y_padding);
    axisY->setTitleText("Y");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // 清除旧图表
    QLayoutItem *child;
    while ((child = ui->gLayPlot->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    QChartView *chartView = new QChartView(chart, ui->widget);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->gLayPlot->addWidget(chartView);
}

void FormPlotCorrection::on_btnStart_clicked()
{
    m_current_round = 0;
    m_start = true;
    ui->labelStatus->setText("0/0");
    ui->textEdit->clear();
    m_v14.clear();
    ui->labelValK->setText("0.0");
    ui->labelValB->setText("0.0");
    ui->btnStart->setStyleSheet("background-color: green; color: white;");
}

void FormPlotCorrection::on_spinBoxRound_valueChanged(int val)
{
    m_ini.round = val;
    SETTING_CONFIG_SET(CFG_GROUP_CORRECTION, CFG_CORRECTION_ROUND, QString::number(val));
}
