#include "singlecurvewindow.h"
#include <QVBoxLayout>

SingleCurveWindow::SingleCurveWindow(QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口属性，使其成为独立窗口
    this->setWindowTitle("Data31 独立视图");
    this->resize(800, 500);

    // 初始化布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_chartView = new QChartView(this);
    layout->addWidget(m_chartView);

    initChart();
}

SingleCurveWindow::~SingleCurveWindow()
{
    // Qt 的对象树会自动清理 children，但 Chart 对象建议手动管理或指定 parent
}

void SingleCurveWindow::initChart()
{
    m_chart = new QChart;
    m_chart->setTitle("提取的目标数据 (Data31) - 下降沿高亮");

    m_seriesLine = new QLineSeries;
    m_seriesLine->setName("Data31");
    m_seriesLine->setPen(QPen(Qt::black, 2)); // 黑色主线

    m_chart->addSeries(m_seriesLine);
    m_chart->createDefaultAxes();

    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

void SingleCurveWindow::updateChart(const QList<QPointF> &linePoints,
                                    const QList<QPointF> &lowerPoints)
{
    // 1. 准备过滤后的容器
    QList<QPointF> filteredLine;
    QList<QPointF> filteredLower;

    int count = qMin(linePoints.size(), lowerPoints.size());
    bool isRecording = false; // 标记当前是否在记录片段

    for (int i = 0; i < count; ++i) {
        // 【核心逻辑】
        // 如果 linePoints 和 lowerPoints 的 Y 值有显著差异，说明该点处于高亮（下降沿）状态
        // 之前的逻辑是：下降沿时 lower = minY，非下降沿时 lower = upper
        double diff = qAbs(linePoints[i].y() - lowerPoints[i].y());

        // 阈值判断 (大于 0.001 视为有差异，即下降沿)
        if (diff > 0.001) {
            filteredLine.append(linePoints[i]);
            filteredLower.append(lowerPoints[i]);
            isRecording = true;
        } else {
            // 如果是非下降沿数据，我们需要“断开”连线
            // 只有当之前正在记录时，才插入一个 NaN 点，避免插入过多的无效点
            if (isRecording) {
                double nan = std::numeric_limits<double>::quiet_NaN();
                QPointF breakPoint(linePoints[i].x(), nan);

                filteredLine.append(breakPoint);
                filteredLower.append(breakPoint);
                isRecording = false;
            }
        }
    }

    // 2. 更新线条数据 (此时 series 只包含下降沿的片段)
    m_seriesLine->replace(filteredLine);

    // 3. 构建高亮区域 (重建 AreaSeries 以匹配断开的线条)
    if (m_areaHighlight) {
        m_chart->removeSeries(m_areaHighlight);
        delete m_areaHighlight;
        m_areaHighlight = nullptr;
    }

    QLineSeries *upper = new QLineSeries();
    upper->append(filteredLine); // 使用过滤后的数据

    QLineSeries *lower = new QLineSeries();
    lower->append(filteredLower); // 使用过滤后的下边界

    m_areaHighlight = new QAreaSeries(upper, lower);
    m_areaHighlight->setName("下降沿区间");
    m_areaHighlight->setPen(QPen(Qt::NoPen));
    m_areaHighlight->setBrush(QBrush(QColor(255, 0, 0, 80))); // 红色半透明

    m_chart->addSeries(m_areaHighlight);

    // 5. 重新调整坐标轴
    m_chart->createDefaultAxes();

    // 6. 轴范围设置
    if (!linePoints.isEmpty()) {
        // X轴：使用原始数据的完整范围，保持时间轴的上下文
        m_chart->axes(Qt::Horizontal)
            .first()
            ->setRange(linePoints.first().x(), linePoints.last().x());

        // Y轴：自适应当前可见的片段
        if (!filteredLine.isEmpty()) {
            double min = std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::lowest();
            bool hasValidPoints = false;

            for (auto p : filteredLine) {
                // 跳过 NaN 点
                if (qIsNaN(p.y()))
                    continue;

                if (p.y() < min)
                    min = p.y();
                if (p.y() > max)
                    max = p.y();
                hasValidPoints = true;
            }

            if (hasValidPoints) {
                double margin = (max - min) * 0.1;
                if (margin == 0)
                    margin = 1.0;
                m_chart->axes(Qt::Vertical).first()->setRange(min - margin, max + margin);
            }
        }
    }
}
