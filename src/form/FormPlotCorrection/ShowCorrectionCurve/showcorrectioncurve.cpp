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
    m_data.push_back(data);
    m_current_page = m_data.size() - 1;
    ui->labelPage->setText(QString("%1 / %2").arg(m_current_page + 1).arg(m_data.size()));
}

void ShowCorrectionCurve::closeEvent(QCloseEvent *event)
{
    m_data.clear();
    m_current_page = 0;
    emit windowClose();
    QWidget::closeEvent(event);
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

    m_current_page = 0;
}

void ShowCorrectionCurve::on_tBtnPrev_clicked()
{
    if (m_current_page <= 0) {
        return; // 已经是第一页
    }
    --m_current_page;
    m_line->replace(m_data[m_current_page]);
    ui->labelPage->setText(QString("%1 / %2").arg(m_current_page + 1).arg(m_data.size()));
}

void ShowCorrectionCurve::on_tBtnNext_clicked()
{
    if (m_current_page >= m_data.size() - 1) {
        return; // 已经是最后一页
    }
    ++m_current_page;
    m_line->replace(m_data[m_current_page]);
    ui->labelPage->setText(QString("%1 / %2").arg(m_current_page + 1).arg(m_data.size()));
}
