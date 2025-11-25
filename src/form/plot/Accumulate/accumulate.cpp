#include "accumulate.h"
#include "MyChartView/mychartview.h"
#include "ui_accumulate.h"
#include <Eigen/Dense>
#include <Eigen/QR>

using Eigen::MatrixXd;
using Eigen::VectorXd;

Accumulate::Accumulate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Accumulate)
{
    ui->setupUi(this);
    init();
}

Accumulate::~Accumulate()
{
    delete ui;
}

QList<QPointF> Accumulate::fitSingleCurve(const QList<QPointF> &points, int order)
{
    int n = points.size();
    if (n <= 1)
        return points;

    order = qMin(order, n - 1);
    int m = order + 1;

    MatrixXd V(n, m);
    VectorXd y(n);

    for (int i = 0; i < n; ++i) {
        double x = points[i].x();
        double pow = 1.0;
        for (int j = 0; j < m; ++j) {
            V(i, j) = pow;
            pow *= x;
        }
        y(i) = points[i].y();
    }

    VectorXd coeffs = V.colPivHouseholderQr().solve(y);

    QList<QPointF> fitted;
    fitted.reserve(n);
    for (const auto &p : points) {
        double x = p.x();
        double val = 0.0;
        double pow = 1.0;
        for (int j = 0; j < m; ++j) {
            val += coeffs(j) * pow;
            pow *= x;
        }
        fitted.append(QPointF(x, val));
    }
    return fitted;
}

void Accumulate::updateAvgFittedCurve(const QList<QPointF> &newFitted)
{
    int n = newFitted.size();
    if (m_avgFittedCurve.isEmpty()) {
        m_avgFittedCurve = newFitted;
    } else {
        int total = m_noiseBuffer.size();
        for (int i = 0; i < n; ++i) {
            m_avgFittedCurve[i].setY((m_avgFittedCurve[i].y() * (total - 1) + newFitted[i].y())
                                     / total);
        }
    }
}

void Accumulate::accumulate(const QList<QPointF> &v)
{
    if (v.isEmpty())
        return;

    if (m_enableNoise) {
        --m_count_noise_remain;
        if (m_count_noise_remain <= 0) {
            m_enableNoise = false;
            ui->tBtnNoiseEnable->setChecked(false);
        }

        // 添加新噪声
        m_noiseBuffer.append(v);

        // 绘制原始噪声
        int idx = m_noiseBuffer.size() - 1;
        if (idx >= m_lineNoiseGroups.size()) {
            auto line = new QLineSeries();
            line->setColor(QColor(0, 0, 0, 32));
            line->setName("");
            m_lineNoiseGroups.append(line);
            m_chartNoise->addSeries(line);
            line->attachAxis(m_axisXNoise);
            line->attachAxis(m_axisYNoise);
        }
        m_lineNoiseGroups[idx]->replace(v);

        // 拟合新噪声并增量更新平均拟合曲线
        QList<QPointF> smoothed = gaussianSmooth(v, m_smooth_window);
        QList<QPointF> newFitted = fitSingleCurve(v, m_poly_order);
        updateAvgFittedCurve(newFitted);

        // 绘制平均拟合曲线
        if (!m_lineNoiseFit) {
            m_lineNoiseFit = new QLineSeries();
            m_lineNoiseFit->setColor(Qt::red);
            m_lineNoiseFit->setName("avg_fitting");
            m_chartNoise->addSeries(m_lineNoiseFit);
            m_lineNoiseFit->attachAxis(m_axisXNoise);
            m_lineNoiseFit->attachAxis(m_axisYNoise);
        }
        m_lineNoiseFit->replace(m_avgFittedCurve);
        // 保证在最前面
        m_chartNoise->removeSeries(m_lineNoiseFit);
        m_chartNoise->addSeries(m_lineNoiseFit);
        m_lineNoiseFit->attachAxis(m_axisXNoise);
        m_lineNoiseFit->attachAxis(m_axisYNoise);

        // 更新轴范围
        double xMin = std::numeric_limits<double>::max();
        double xMax = std::numeric_limits<double>::lowest();
        double yMin = std::numeric_limits<double>::max();
        double yMax = std::numeric_limits<double>::lowest();
        for (auto &curve : m_noiseBuffer)
            for (auto &p : curve) {
                xMin = qMin(xMin, p.x());
                xMax = qMax(xMax, p.x());
                yMin = qMin(yMin, p.y());
                yMax = qMax(yMax, p.y());
            }
        for (auto &p : m_avgFittedCurve) {
            yMin = qMin(yMin, p.y());
            yMax = qMax(yMax, p.y());
        }
        m_axisXNoise->setRange(xMin, xMax);
        m_axisYNoise->setRange(yMin, yMax);

        ui->labelCountStatus->setText(QString("%1/%2").arg(m_count_noise_remain).arg(m_count_noise));
    }

    // 信号累加
    if (m_enableAccumulate) {
        ++m_count_acc;
        ui->labelAccStatus->setText(QString("acc %1").arg(m_count_acc));
        QList<QPointF> accumulated;
        int n = v.size();
        for (int i = 0; i < n; ++i) {
            double y = v[i].y();
            if (i < m_avgFittedCurve.size())
                y -= m_avgFittedCurve[i].y();
            if (i < m_accumulatedCurve.size())
                y += m_accumulatedCurve[i].y();
            accumulated.append(QPointF(v[i].x(), y));
        }
        m_accumulatedCurve = accumulated;
        m_lineAcc->replace(m_accumulatedCurve);

        // 更新轴范围
        double xMin = std::numeric_limits<double>::max();
        double xMax = std::numeric_limits<double>::lowest();
        double yMin = std::numeric_limits<double>::max();
        double yMax = std::numeric_limits<double>::lowest();
        for (auto &p : m_accumulatedCurve) {
            xMin = qMin(xMin, p.x());
            xMax = qMax(xMax, p.x());
            yMin = qMin(yMin, p.y());
            yMax = qMax(yMax, p.y());
        }
        m_axisXAcc->setRange(xMin, xMax);
        m_axisYAcc->setRange(yMin, yMax);
    }
}

// 初始化
void Accumulate::init()
{
    m_chartNoise = new QChart();
    m_chartNoise->setTitle(tr("Noise"));
    m_chartNoise->legend()->hide();

    m_axisXNoise = new QValueAxis();
    m_axisYNoise = new QValueAxis();
    m_chartNoise->addAxis(m_axisXNoise, Qt::AlignBottom);
    m_chartNoise->addAxis(m_axisYNoise, Qt::AlignLeft);

    m_chartViewNoise = new MyChartView(m_chartNoise);
    m_chartViewNoise->setRenderHint(QPainter::Antialiasing);
    ui->gLayNoise->addWidget(m_chartViewNoise);

    m_chartAcc = new QChart();
    m_chartAcc->setTitle(tr("Accumulate"));
    m_chartAcc->legend()->hide();

    m_axisXAcc = new QValueAxis();
    m_axisYAcc = new QValueAxis();
    m_chartAcc->addAxis(m_axisXAcc, Qt::AlignBottom);
    m_chartAcc->addAxis(m_axisYAcc, Qt::AlignLeft);

    m_lineAcc = new QLineSeries();
    m_chartAcc->addSeries(m_lineAcc);
    m_lineAcc->attachAxis(m_axisXAcc);
    m_lineAcc->attachAxis(m_axisYAcc);

    m_chartViewAcc = new MyChartView(m_chartAcc);
    m_chartViewAcc->setRenderHint(QPainter::Antialiasing);
    ui->gLayAccumulate->addWidget(m_chartViewAcc);

    ui->tBtnAccumulateEnable->setCheckable(true);
    ui->tBtnNoiseEnable->setCheckable(true);

    ui->spinBoxNoseCount->setValue(50);
    ui->spinBoxPolyOrder->setValue(3);
    ui->spinBoxSmoothWindow->setValue(10);
}

QList<QPointF> Accumulate::gaussianSmooth(const QList<QPointF> &points, int window)
{
    if (points.isEmpty() || window <= 1)
        return points;

    QList<QPointF> result;
    int n = points.size();
    int radius = window / 2;
    double sigma = window / 3.0;

    // 高斯权重
    QVector<double> weights(window);
    double sumWeights = 0.0;
    for (int i = 0; i < window; ++i) {
        int x = i - radius;
        double w = std::exp(-(x * x) / (2 * sigma * sigma));
        weights[i] = w;
        sumWeights += w;
    }

    // 归一化（可省略，不影响结果）
    for (int i = 0; i < window; ++i)
        weights[i] /= sumWeights;

    // 平滑处理
    for (int i = 0; i < n; ++i) {
        double ySum = 0.0;
        double wSum = 0.0;

        for (int k = -radius; k <= radius; ++k) {
            int idx = i + k;
            if (idx >= 0 && idx < n) {
                double w = weights[k + radius];
                ySum += points[idx].y() * w;
                wSum += w;
            }
        }

        result.append(QPointF(points[i].x(), ySum / wSum));
    }

    return result;
}

// 开关
void Accumulate::on_tBtnNoiseEnable_clicked()
{
    m_enableNoise = !m_enableNoise;
    ui->tBtnNoiseEnable->setChecked(m_enableNoise);
    if (m_enableNoise) {
        m_count_noise = m_count_noise_remain = ui->spinBoxNoseCount->value();
        m_poly_order = ui->spinBoxPolyOrder->value();
        m_smooth_window = ui->spinBoxSmoothWindow->value();
        if (m_smooth_window % 2 == 0) {
            m_smooth_window += 1;
            ui->spinBoxSmoothWindow->setValue(m_smooth_window);
        }
        m_noiseBuffer.clear();
        m_avgFittedCurve.clear();
        m_accumulateNoise.clear();

        m_chartNoise->removeAllSeries();
        m_lineNoiseGroups.clear();
        m_lineNoiseFit = nullptr;
    }
}

void Accumulate::on_tBtnAccumulateEnable_clicked()
{
    m_enableAccumulate = !m_enableAccumulate;
    ui->tBtnAccumulateEnable->setChecked(m_enableAccumulate);
    if (m_enableAccumulate) {
        m_accumulatedCurve.clear();
        m_count_acc = 0;
        ui->labelAccStatus->setText(QString("acc %1").arg(m_count_acc));
    }
}
