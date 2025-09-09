#include "showcorrectioncurve.h"
#include "ui_showcorrectioncurve.h"

ShowCorrectionCurve::ShowCorrectionCurve(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowCorrectionCurve)
{
    ui->setupUi(this);
    init();
}

ShowCorrectionCurve::~ShowCorrectionCurve()
{
    delete ui;
}

void ShowCorrectionCurve::updatePlot(const QList<QPointF> &data,
                                     const double &xMin,
                                     const double &xMax,
                                     const double &yMin,
                                     const double &yMax,
                                     const double &temperature)
{
    ui->labelTemperature->setText(QString("%1 ℃").arg(temperature));
    m_line->replace(data);
    m_axisX->setRange(xMin, xMax);
    m_axisY->setRange(yMin, yMax);
}

void ShowCorrectionCurve::init()
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
    m_axisX->setTitleText("wavelength");
    m_axisY->setTitleText("intensity");
    m_chart->setTitle("correction curve");
    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->stackedWidget->addWidget(m_chartView);
    ui->stackedWidget->setCurrentWidget(m_chartView);
}
