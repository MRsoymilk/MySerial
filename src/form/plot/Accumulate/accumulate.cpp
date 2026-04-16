#include "accumulate.h"

#include <Eigen/Dense>
#include <Eigen/QR>

#include "MyChartView/mychartview.h"
#include "keydef.h"
#include "ui_accumulate.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;

Accumulate::Accumulate(QWidget *parent) : QWidget(parent), ui(new Ui::Accumulate) {
    ui->setupUi(this);
    init();
}

Accumulate::~Accumulate() { delete ui; }

QList<QPointF> Accumulate::fitSingleCurve(const QList<QPointF> &points, int order) {
    int n = points.size();
    if (n <= 1) return points;

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

void Accumulate::updateAvgFittedCurve(const QList<QPointF> &newFitted) {
    if (m_avgFitCount == 0) {
        m_avgFittedCurve = newFitted;
        m_avgFitCount = 1;
        return;
    }

    int n = newFitted.size();

    for (int i = 0; i < n; ++i) {
        double old = m_avgFittedCurve[i].y();
        double now = newFitted[i].y();
        m_avgFittedCurve[i].setY((old * m_avgFitCount + now) / (m_avgFitCount + 1));
    }

    m_avgFitCount++;
}

QList<QPointF> Accumulate::accumulate(const QList<QPointF> &v) {
    if (v.isEmpty()) return {};

    // ==========================
    // 噪声采集
    // ==========================
    if (m_enableNoise) {
        --m_count_noise_remain;

        if (m_count_noise_remain <= 0) {
            m_enableNoise = false;
            ui->tBtnNoiseEnable->setChecked(false);
        }

        m_noiseBuffer.append(v);

        int idx = m_noiseBuffer.size() - 1;

        double ymin = std::numeric_limits<double>::max();
        double ymax = std::numeric_limits<double>::lowest();

        for (auto &p : v) {
            ymin = qMin(ymin, p.y());
            ymax = qMax(ymax, p.y());
        }

        if (idx >= m_noiseMinMaxGroups.size()) {
            auto scatter = new QScatterSeries();

            scatter->setMarkerSize(6);
            scatter->setColor(QColor(0, 0, 0, 120));
            scatter->setName("");

            m_noiseMinMaxGroups.append(scatter);

            m_chartNoise->addSeries(scatter);

            scatter->attachAxis(m_axisXNoiseMinMax);
            scatter->attachAxis(m_axisYNoise);
        }

        QVector<QPointF> pts;
        pts.append(QPointF(idx, ymin));
        pts.append(QPointF(idx, ymax));

        m_noiseMinMaxGroups[idx]->replace(pts);

        // ==========================
        // 拟合噪声
        // ==========================
        QList<QPointF> smoothed = gaussianSmooth(v, m_smooth_window);
        QList<QPointF> newFitted = fitSingleCurve(smoothed, m_poly_order);

        updateAvgFittedCurve(newFitted);

        if (!m_lineNoiseFit) {
            m_lineNoiseFit = new QLineSeries();
            m_lineNoiseFit->setColor(Qt::red);
            m_lineNoiseFit->setName("avg_fitting");

            m_chartNoise->addSeries(m_lineNoiseFit);

            m_lineNoiseFit->attachAxis(m_axisXNoiseFit);
            m_lineNoiseFit->attachAxis(m_axisYNoise);
        }

        m_lineNoiseFit->replace(m_avgFittedCurve);

        // 保证曲线在最上层
        m_chartNoise->removeSeries(m_lineNoiseFit);
        m_chartNoise->addSeries(m_lineNoiseFit);

        m_lineNoiseFit->attachAxis(m_axisXNoiseFit);
        m_lineNoiseFit->attachAxis(m_axisYNoise);

        // ==========================
        // 更新坐标轴
        // ==========================

        double xMin = 0;
        double xMax = m_noiseMinMaxGroups.size();

        double yMin = std::numeric_limits<double>::max();
        double yMax = std::numeric_limits<double>::lowest();

        for (auto series : m_noiseMinMaxGroups) {
            for (auto &p : series->points()) {
                yMin = qMin(yMin, p.y());
                yMax = qMax(yMax, p.y());
            }
        }

        for (auto &p : m_avgFittedCurve) {
            yMin = qMin(yMin, p.y());
            yMax = qMax(yMax, p.y());
        }

        m_axisXNoiseMinMax->setRange(xMin, xMax);
        m_axisYNoise->setRange(yMin, yMax);
        if (!m_avgFittedCurve.isEmpty()) {
            double fitXMin = m_avgFittedCurve.first().x();
            double fitXMax = m_avgFittedCurve.last().x();

            m_axisXNoiseFit->setRange(fitXMin, fitXMax);
        }

        ui->labelCountStatus->setText(QString("%1/%2").arg(m_count_noise_remain).arg(m_count_noise));
    }

    // ==========================
    // 信号累加
    // ==========================
    else if (m_enableAccumulate) {
        ++m_count_acc;

        ui->labelAccStatus->setText(QString("acc %1").arg(m_count_acc));

        QList<QPointF> accumulated;

        int n = v.size();

        if (m_accumulatedCurve.isEmpty()) {
            for (int i = 0; i < n; ++i) m_accumulatedCurve.append({v[i].x(), 0});
        }

        for (int i = 0; i < n; ++i) {
            double y = v[i].y();

            if (!m_avgFittedCurve.isEmpty()) y -= m_avgFittedCurve[i].y();

            y += m_accumulatedCurve[i].y();

            accumulated.append(QPointF(v[i].x(), y));
        }

        m_accumulatedCurve = accumulated;

        if (m_target_count != 0) {
            if (m_count_acc >= m_target_count) {
                if (m_enableDiv) {
                    for (int i = 0; i < accumulated.size(); ++i) accumulated[i].setY(accumulated[i].y() / m_count_acc);
                }

                m_count_acc = 0;
                m_accumulatedCurve.clear();
            } else {
                return {};
            }
        }

        return accumulated;
    }

    // ==========================
    // 基线扣除
    // ==========================
    else if (m_enableBaselineDeduction) {
        QList<QPointF> result;

        int n = v.size();

        for (int i = 0; i < n; ++i) {
            double y = v[i].y() - m_avgFittedCurve[i].y();

            result.append(QPointF(v[i].x(), y));
        }

        return result;
    }

    return v;
}

void Accumulate::closeEvent(QCloseEvent *event) {
    m_accumulatedCurve.clear();
    m_accumulateNoise.clear();
    m_enableAccumulate = false;
    m_enableBaselineDeduction = false;
    m_enableNoise = false;
    ui->tBtnBaselineDeductionEnable->setChecked(false);
    ui->tBtnAccumulateEnable->setChecked(false);
    ui->tBtnNoiseEnable->setChecked(false);
    ui->labelCountStatus->setText("status");
    emit windowClose();
}

void Accumulate::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QAction *clearNoseAction = new QAction("Clear Noise", &menu);
    QAction *clearAccumulateAction = new QAction("Clear Accumulate", &menu);
    menu.addAction(clearNoseAction);
    menu.addSeparator();
    menu.addAction(clearAccumulateAction);

    connect(clearNoseAction, &QAction::triggered, this, &Accumulate::onMenuClearNose);
    connect(clearAccumulateAction, &QAction::triggered, this, &Accumulate::onMenuClearAccumulate);

    menu.exec(event->globalPos());
}

void Accumulate::onMenuClearNose() {
    m_noiseBuffer.clear();
    m_avgFittedCurve.clear();
    m_accumulateNoise.clear();
    m_chartNoise->removeAllSeries();
    m_noiseMinMaxGroups.clear();
    m_lineNoiseFit = nullptr;
    m_avgFitCount = 0;
}

void Accumulate::onMenuClearAccumulate() {
    on_tBtnAccumulateEnable_clicked();
    on_tBtnAccumulateEnable_clicked();
}

// 初始化
void Accumulate::init() {
    m_chartNoise = new QChart();
    m_chartNoise->setTitle(tr("Noise"));
    m_chartNoise->legend()->hide();

    m_axisXNoiseFit = new QValueAxis();
    m_axisXNoiseFit->setTitleText(tr("Noise Fit"));
    m_axisXNoiseMinMax = new QValueAxis();
    m_axisXNoiseMinMax->setTitleText(tr("Noise Min,Max"));
    m_axisYNoise = new QValueAxis();

    m_chartNoise->addAxis(m_axisXNoiseMinMax, Qt::AlignTop);
    m_chartNoise->addAxis(m_axisXNoiseFit, Qt::AlignBottom);
    m_chartNoise->addAxis(m_axisYNoise, Qt::AlignLeft);

    m_chartViewNoise = new MyChartView(m_chartNoise);
    m_chartViewNoise->setRenderHint(QPainter::Antialiasing);

    ui->gLayNoise->addWidget(m_chartViewNoise);

    ui->tBtnBaselineDeductionEnable->setCheckable(true);
    ui->tBtnAccumulateEnable->setCheckable(true);
    ui->tBtnNoiseEnable->setCheckable(true);

    ui->spinBoxNoseCount->setValue(50);
    ui->spinBoxPolyOrder->setValue(3);
    ui->spinBoxSmoothWindow->setValue(10);

    ui->tBtnEnableDiv->setCheckable(true);
}

QList<QPointF> Accumulate::gaussianSmooth(const QList<QPointF> &points, int window) {
    if (points.isEmpty() || window <= 1) return points;

    QList<QPointF> result;

    int n = points.size();
    int radius = window / 2;

    double sigma = window / 3.0;

    QVector<double> weights(window);

    double sumWeights = 0.0;

    for (int i = 0; i < window; ++i) {
        int x = i - radius;

        double w = std::exp(-(x * x) / (2 * sigma * sigma));

        weights[i] = w;

        sumWeights += w;
    }

    for (int i = 0; i < window; ++i) weights[i] /= sumWeights;

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
void Accumulate::on_tBtnNoiseEnable_clicked() {
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
        m_noiseMinMaxGroups.clear();
        m_lineNoiseFit = nullptr;
        m_avgFitCount = 0;
    }
}

void Accumulate::on_tBtnAccumulateEnable_clicked() {
    m_enableAccumulate = !m_enableAccumulate;
    ui->tBtnAccumulateEnable->setChecked(m_enableAccumulate);
    if (m_enableAccumulate) {
        m_enableBaselineDeduction = false;
        ui->tBtnBaselineDeductionEnable->setChecked(false);
        m_accumulatedCurve.clear();
        m_count_acc = 0;
        ui->labelAccStatus->setText(QString("acc %1").arg(m_count_acc));
    }
}

void Accumulate::on_tBtnBaselineDeductionEnable_clicked() {
    if (m_avgFittedCurve.isEmpty()) {
        QMessageBox::warning(nullptr, TITLE_WARNING, tr("Please enable noise fitting first!"));
        ui->tBtnBaselineDeductionEnable->setChecked(false);
        return;
    }
    m_enableBaselineDeduction = !m_enableBaselineDeduction;
    ui->tBtnBaselineDeductionEnable->setChecked(m_enableBaselineDeduction);
    if (m_enableBaselineDeduction) {
        m_enableAccumulate = false;
        ui->tBtnAccumulateEnable->setChecked(false);
    }
}

void Accumulate::on_spinBoxCount_valueChanged(int count) { m_target_count = count; }

void Accumulate::on_tBtnEnableDiv_clicked() {
    m_enableDiv = !m_enableDiv;
    ui->tBtnEnableDiv->setChecked(m_enableDiv);
}
