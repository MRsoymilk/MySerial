#include "formplothistory.h"
#include "ui_formplothistory.h"

#include <QLineSeries>

FormPlotHistory::FormPlotHistory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlotHistory)
{
    ui->setupUi(this);
    init();
}

FormPlotHistory::~FormPlotHistory()
{
    if (m_chartView14) {
        delete m_chartView14;
    }
    if (m_chartView24) {
        delete m_chartView24;
    }
    delete ui;
}

void FormPlotHistory::init()
{
    m_chartView14 = new QChartView(this);
    m_chartView24 = new QChartView(this);

    m_chartView14->setRenderHint(QPainter::Antialiasing);
    m_chartView24->setRenderHint(QPainter::Antialiasing);

    ui->stackedWidget->insertWidget(0, m_chartView14);
    ui->stackedWidget->insertWidget(1, m_chartView24);
    ui->stackedWidget->setCurrentIndex(0);

    ui->tBtnPrev14->setIcon(QIcon(":/res/icons/go-prev.png"));
    ui->tBtnPrev14->setToolTip("prev");
    ui->tBtnNext14->setIcon(QIcon(":/res/icons/go-next.png"));
    ui->tBtnNext14->setToolTip("next");
    ui->tBtnPrev24->setIcon(QIcon(":/res/icons/go-prev.png"));
    ui->tBtnPrev24->setToolTip("prev");
    ui->tBtnNext24->setIcon(QIcon(":/res/icons/go-next.png"));
    ui->tBtnNext24->setToolTip("next");
}

void FormPlotHistory::updateData(const QList<QList<QPointF> > &p14,
                                 const QList<QList<QPointF> > &p24)
{
    m_p14 = p14;
    m_p24 = p24;
    m_index_14 = 0;
    m_index_24 = 0;
    updatePlot14();
    updatePlot24();
}

void FormPlotHistory::on_tBtnNext14_clicked()
{
    if (m_index_14 + 1 < m_p14.size()) {
        m_index_14++;
    } else {
        m_index_14 = m_p14.size() - 1;
    }
    updatePlot14();
}

void FormPlotHistory::on_tBtnPrev14_clicked()
{
    if (m_index_14 - 1 >= 0) {
        m_index_14--;
    } else {
        m_index_14 = 0;
    }
    updatePlot14();
}

void FormPlotHistory::on_tBtnNext24_clicked()
{
    if (m_index_24 + 1 < m_p24.size()) {
        m_index_24++;
    } else {
        m_index_24 = m_p24.size() - 1;
    }
    updatePlot24();
}

void FormPlotHistory::on_tBtnPrev24_clicked()
{
    if (m_index_24 - 1 >= 0) {
        m_index_24--;
    } else {
        m_index_24 = 0;
    }
    updatePlot24();
}

void FormPlotHistory::updatePlot14()
{
    if (m_index_14 >= m_p14.size() || m_p14.empty()) {
        QChart *chart = m_chartView14->chart();
        for (QAbstractSeries *series : chart->series()) {
            chart->removeSeries(series);
            delete series;
        }
        ui->labelStatus14->setText("status");
        return;
    }

    QChart *chart = new QChart();
    QLineSeries *series = new QLineSeries();
    series->setColor(Qt::magenta);

    series->append(m_p14[m_index_14]);

    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("curve_14bit");

    m_chartView14->setChart(chart);
    ui->stackedWidget->setCurrentWidget(m_chartView14);
    ui->labelStatus14->setText(QString("%1/%2").arg(m_index_14 + 1).arg(m_p14.size()));
}

void FormPlotHistory::updatePlot24()
{
    if (m_index_24 >= m_p24.size() || m_p24.empty()) {
        QChart *chart = m_chartView24->chart();
        for (QAbstractSeries *series : chart->series()) {
            chart->removeSeries(series);
            delete series;
        }
        ui->labelStatus24->setText("status");
        return;
    }

    QChart *chart = new QChart();
    QLineSeries *series = new QLineSeries();

    series->append(m_p24[m_index_24]);
    series->setColor(Qt::blue);

    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("curve_24bit");

    m_chartView24->setChart(chart);
    ui->stackedWidget->setCurrentWidget(m_chartView24);
    ui->labelStatus24->setText(QString("%1/%2").arg(m_index_24 + 1).arg(m_p24.size()));
}

void FormPlotHistory::closeEvent(QCloseEvent *event)
{
    m_p14.clear();
    m_p24.clear();
    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotHistory::on_lineEdit14Go_editingFinished()
{
    int val = ui->lineEdit14Go->text().toInt();
    if (val > 0 && val <= m_p14.size()) {
        m_index_14 = val - 1;
        updatePlot14();
    }
}

void FormPlotHistory::on_lineEdit24Go_editingFinished()
{
    int val = ui->lineEdit24Go->text().toInt();
    if (val > 0 && val <= m_p24.size()) {
        m_index_24 = val - 1;
        updatePlot24();
    }
}
