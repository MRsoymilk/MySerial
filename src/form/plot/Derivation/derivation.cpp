#include "derivation.h"
#include "MyChartView/mychartview.h" // 自定义视图
#include "SingleCurveWindow/singlecurvewindow.h"
#include "ui_derivation.h"
#include <limits>

Derivation::Derivation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Derivation)
{
    ui->setupUi(this);
    initChart();
}

Derivation::~Derivation()
{
    if (m_subWindow) {
        m_subWindow->close();
        delete m_subWindow;
    }
    delete ui;
}

void Derivation::initChart()
{
    m_chart = new QChart;
    m_chart->setTitle("基于 Data33 导数高亮 Data31");
    m_seriesData33 = new QLineSeries;
    m_seriesData33->setName("Data33 (参考波形)");
    m_seriesData33->setPen(QPen(Qt::magenta, 1));

    // 2. 导数曲线 (红色虚线)
    m_seriesDeriv = new QLineSeries;
    m_seriesDeriv->setName("Data33 导数");
    QPen penDeriv(Qt::red, 1);
    penDeriv.setStyle(Qt::DotLine);
    m_seriesDeriv->setPen(penDeriv);

    // 3. Data31: 目标数据
    m_seriesData31 = new QLineSeries;
    m_seriesData31->setName("Data31 (目标数据)");
    m_seriesData31->setPen(QPen(Qt::blue, 2));

    // 添加顺序决定图层遮挡关系
    m_chart->addSeries(m_seriesData33);
    m_chart->addSeries(m_seriesDeriv);
    m_chart->addSeries(m_seriesData31);

    m_chart->createDefaultAxes();

    // 设置 View
    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayChart->addWidget(m_chartView);

    // 初始化按钮状态
    ui->tBtnExtraCurve->setCheckable(true);
    ui->tBtnExtraCurve->setChecked(false);

    m_subWindow = new SingleCurveWindow();
}

void Derivation::derivation(const QList<QPointF> &data31, const QList<QPointF> &data33)
{
    m_lastData31 = data31;
    m_lastData33 = data33;

    // 1. 数据清理
    m_seriesData33->clear();
    m_seriesDeriv->clear();
    m_seriesData31->clear();

    if (m_areaHighlight) {
        m_chart->removeSeries(m_areaHighlight);
        delete m_areaHighlight;
        m_areaHighlight = nullptr;
    }

    // 2. 基础校验
    int count = qMin(data31.size(), data33.size());
    if (count < 2)
        return;

    // 3. 准备容器
    QList<QPointF> points33, points31, pointsDeriv;

    // 寻找 Data31 的底部基准线
    double data31MinY = std::numeric_limits<double>::max();
    for (const auto &p : data31) {
        if (p.y() < data31MinY)
            data31MinY = p.y();
    }
    data31MinY -= 1.0; // 稍微往下一点，视觉更好

    // 4. 准备高亮区域边界
    QLineSeries *upper = new QLineSeries;
    QLineSeries *lower = new QLineSeries;
    QList<QPointF> subWindowLowerPoints; // 独立窗口用的下边界数据

    // --- 初始化第0个点 ---
    points33.append(data33[0]);
    points31.append(data31[0]);

    upper->append(data31[0]);
    lower->append(data31[0]);
    subWindowLowerPoints.append(data31[0]); // 【修复】子窗口数据也要初始化

    // 5. 遍历计算
    for (int i = 1; i < count; ++i) {
        // A. 收集数据
        points33.append(data33[i]);
        points31.append(data31[i]);

        // B. 计算导数
        double diff = data33[i].y() - data33[i - 1].y();
        pointsDeriv.append(QPointF(data33[i].x(), diff));

        // C. 高亮判断 (下降沿)
        bool isFalling = diff < -0.0001;

        upper->append(data31[i]);

        if (isFalling) {
            // 【修复】定义 bottomPoint 变量
            QPointF bottomPoint(data31[i].x(), data31MinY);

            lower->append(bottomPoint);
            subWindowLowerPoints.append(bottomPoint);
        } else {
            lower->append(data31[i]);
            subWindowLowerPoints.append(data31[i]);
        }
    }

    // 6. 更新图表数据
    m_seriesData33->replace(points33);
    m_seriesData31->replace(points31);
    m_seriesDeriv->replace(pointsDeriv);

    // 7. 创建高亮区域
    m_areaHighlight = new QAreaSeries(upper, lower);
    m_areaHighlight->setName("Data31 高亮区");
    m_areaHighlight->setPen(QPen(Qt::NoPen));
    m_areaHighlight->setBrush(QBrush(QColor(0, 255, 0, 80))); // 绿色半透明
    m_chart->addSeries(m_areaHighlight);

    // 重置坐标轴
    m_chart->createDefaultAxes();

    // 8. 优化坐标轴范围
    double yMin = data31MinY;
    double yMax = std::numeric_limits<double>::lowest();

    for (auto p : points31)
        if (p.y() > yMax)
            yMax = p.y();
    for (auto p : points33)
        if (p.y() > yMax)
            yMax = p.y();

    double margin = (yMax - yMin) * 0.1;
    if (margin == 0)
        margin = 1.0;

    m_chart->axes(Qt::Vertical).first()->setRange(yMin - margin, yMax + margin);
    m_chart->axes(Qt::Horizontal).first()->setRange(data33.first().x(), data33.last().x());

    // 9. 处理独立窗口逻辑
    if (m_enableExtraCurve) {
        m_seriesData33->hide(); // 隐藏主图的 Data33

        if (m_subWindow) {
            // 更新子窗口数据
            m_subWindow->updateChart(points31, subWindowLowerPoints);

            if (!m_subWindow->isVisible()) {
                m_subWindow->show();
            }
        }
    } else {
        m_seriesData33->show(); // 显示主图的 Data33
        if (m_subWindow) {
            m_subWindow->hide(); // 可选：关闭按钮时隐藏子窗口
        }
    }
}

void Derivation::on_tBtnExtraCurve_clicked()
{
    // 切换状态
    m_enableExtraCurve = ui->tBtnExtraCurve->isChecked();

    // 如果有缓存数据，立即刷新界面以应用显示/隐藏逻辑
    if (!m_lastData31.isEmpty() && !m_lastData33.isEmpty()) {
        derivation(m_lastData31, m_lastData33);
    }
}
