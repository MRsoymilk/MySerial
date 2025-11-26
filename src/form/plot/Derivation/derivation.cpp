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
}

void Derivation::derivation(const QList<QPointF> &data31, const QList<QPointF> &data33)
{
    m_lastData31 = data31;
    m_lastData33 = data33;

    m_seriesData33->clear();
    m_seriesDeriv->clear();
    m_seriesData31->clear();

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

    Eigen::Array<bool, Eigen::Dynamic, 1> falling = diff.array() < -0.0001;

    QList<QPointF> points33, points31, pointsDeriv;

    QLineSeries *upper = new QLineSeries;
    QLineSeries *lower = new QLineSeries;

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::min();

    for (int i = 0; i < count; ++i) {
        yMin = std::min(yMin, std::min(y31(i), y33(i)));
        yMax = std::max(yMax, std::max(y31(i), y33(i)));

        points33.append(QPointF(x(i), y33(i)));
        points31.append(QPointF(x(i), y31(i)));
        pointsDeriv.append(QPointF(x(i), diff(i)));

        upper->append(x(i), y31(i));

        double ly = falling(i) ? data31MinY : y31(i);
        lower->append(x(i), ly);
    }

    m_seriesData33->replace(points33);
    m_seriesData31->replace(points31);
    m_seriesDeriv->replace(pointsDeriv);

    m_areaHighlight = new QAreaSeries(upper, lower);
    m_areaHighlight->setName("Data31 Highlight");
    m_areaHighlight->setPen(Qt::NoPen);
    m_areaHighlight->setBrush(QColor(0, 255, 0, 80));
    m_chart->addSeries(m_areaHighlight);

    m_chart->createDefaultAxes();

    double margin = (yMax - yMin) * 0.1;
    if (margin == 0)
        margin = 1.0;

    m_chart->axes(Qt::Vertical).first()->setRange(yMin - margin, yMax + margin);
    m_chart->axes(Qt::Horizontal).first()->setRange(x(0), x(count - 1));
}
