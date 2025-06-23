#include "formplothistory.h"
#include "ui_formplothistory.h"

#include <QLineSeries>
#include <QValueAxis>
#include "funcdef.h"

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

void FormPlotHistory::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormPlotHistory::init()
{
    m_chartView14 = new QChartView(this);
    m_chartView24 = new QChartView(this);
    m_chartMix = new QChartView(this);

    m_chartView14->setRenderHint(QPainter::Antialiasing);
    m_chartView24->setRenderHint(QPainter::Antialiasing);
    m_chartMix->setRenderHint(QPainter::Antialiasing);

    ui->stackedWidget->insertWidget(0, m_chartView14);
    ui->stackedWidget->insertWidget(1, m_chartView24);
    ui->stackedWidget->insertWidget(2, m_chartMix);

    m_chartView14Split = new QChartView(this);
    m_chartView24Split = new QChartView(this);
    m_chartView14Split->setRenderHint(QPainter::Antialiasing);
    m_chartView24Split->setRenderHint(QPainter::Antialiasing);

    m_charSplit = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_charSplit);
    layout->addWidget(m_chartView14Split);
    layout->addWidget(m_chartView24Split);
    layout->setStretch(0, 1);
    layout->setStretch(1, 1);
    layout->setContentsMargins(0, 0, 0, 0);
    m_charSplit->setLayout(layout);
    ui->stackedWidget->insertWidget(3, m_charSplit);
    ui->stackedWidget->setCurrentIndex(0);

    ui->radioButtonMix->setChecked(true);

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

    updatePlot(INDEX_14);
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

    updatePlot(INDEX_24);
}

void FormPlotHistory::updatePlot(int index)
{
    bool isMix = ui->radioButtonMix->isChecked();
    bool isSplit = ui->radioButtonSplit->isChecked();

    if (isMix) {
        if (index == INDEX_14) {
            if (m_index_14 < m_p24.size()) {
                m_index_24 = m_index_14;
            } else {
                return;
            }
        } else if (index == INDEX_24) {
            if (m_index_24 < m_p14.size()) {
                m_index_14 = m_index_24;
            } else {
                return;
            }
        }
        QChart *chart = new QChart();

        QLineSeries *series24 = new QLineSeries();
        series24->append(m_p24[m_index_24]);
        series24->setColor(Qt::blue);

        QLineSeries *series14 = new QLineSeries();
        series14->append(m_p14[m_index_14]);
        series14->setColor(Qt::magenta);

        chart->addSeries(series24);
        chart->addSeries(series14);

        chart->createDefaultAxes();
        chart->setTitle(tr("curve_mix"));

        m_chartMix->setChart(chart);
        ui->stackedWidget->setCurrentWidget(m_chartMix);
        ui->labelStatus14->setText(QString("%1/%2").arg(m_index_14 + 1).arg(m_p14.size()));
        ui->labelStatus24->setText(QString("%1/%2").arg(m_index_24 + 1).arg(m_p24.size()));
    } else {
        if (index == INDEX_14) {
            QChart *chart = new QChart();
            QLineSeries *series = new QLineSeries();
            series->setColor(Qt::magenta);

            series->append(m_p14[m_index_14]);

            chart->addSeries(series);
            chart->createDefaultAxes();
            chart->setTitle(tr("curve_14bit"));

            m_chartView14->setChart(chart);
            if (!isSplit) {
                ui->stackedWidget->setCurrentWidget(m_chartView14);
            }
            ui->labelStatus14->setText(QString("%1/%2").arg(m_index_14 + 1).arg(m_p14.size()));
        } else if (index == INDEX_24) {
            QChart *chart = new QChart();
            QLineSeries *series = new QLineSeries();

            series->append(m_p24[m_index_24]);
            series->setColor(Qt::blue);

            chart->addSeries(series);
            chart->createDefaultAxes();
            chart->setTitle(tr("curve_24bit"));

            m_chartView24->setChart(chart);
            if (!isSplit) {
                ui->stackedWidget->setCurrentWidget(m_chartView24);
            }
            ui->labelStatus24->setText(QString("%1/%2").arg(m_index_24 + 1).arg(m_p24.size()));
        }
        if (isSplit) {
            QChart *chart = new QChart();
            QLineSeries *series = new QLineSeries();
            if (index == INDEX_14) {
                series->append(m_p14[m_index_14]);
                series->setColor(Qt::magenta);
                chart->setTitle(tr("curve_14bit"));
                m_chartView14Split->setChart(chart);
            } else {
                series->append(m_p24[m_index_24]);
                series->setColor(Qt::blue);
                chart->setTitle(tr("curve_24bit"));
                m_chartView24Split->setChart(chart);
            }
            chart->addSeries(series);
            chart->createDefaultAxes();
            ui->stackedWidget->setCurrentIndex(3);
        }
    }
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
        updatePlot(INDEX_14);
    }
}

void FormPlotHistory::on_lineEdit24Go_editingFinished()
{
    int val = ui->lineEdit24Go->text().toInt();
    if (val > 0 && val <= m_p24.size()) {
        m_index_24 = val - 1;
        updatePlot(INDEX_24);
    }
}

void FormPlotHistory::on_radioButtonMix_clicked()
{
    updatePlot(INDEX_14);
}

void FormPlotHistory::on_radioButtonSplit_clicked()
{
    updatePlot(INDEX_14);
}

void FormPlotHistory::on_toolButtonFitting_clicked()
{
    // 14bit
    if (m_index_14 >= m_p14.size() || m_p14[m_index_14].isEmpty()) {
        return;
    }

    QString txt_k = ui->lineEditK->text();
    QString txt_b = ui->lineEditB->text();
    if (txt_k.isEmpty() || txt_b.isEmpty()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Empty Params"), tr("k and b can not be empty!"));
        return;
    }

    float k = txt_k.toFloat();
    float b = txt_b.toFloat();
    if (ui->checkBoxConversion->isChecked()) {
        k = k / 0x1FFF * 3.3;
        b = b / 0x1FFF * 3.3;
    }
    const QList<QPointF> &points = m_p14[m_index_14];
    double xMin = points.first().x();
    double xMax = points.last().x();
    if (xMin > xMax)
        std::swap(xMin, xMax);

    QLineSeries *line = new QLineSeries();
    line->setColor(Qt::red);
    line->setName("Fitting Line");
    for (const QPointF &pt : points) {
        double x = pt.x();
        double y = k * x + b;
        line->append(x, y);
    }

    QChart *chart = nullptr;
    if (ui->radioButtonMix->isChecked()) {
        chart = m_chartMix->chart();
    } else if (ui->radioButtonSplit->isChecked()) {
        chart = m_chartView14Split->chart();
    } else {
        chart = m_chartView14->chart();
    }

    if (!chart)
        return;

    QList<QAbstractSeries *> existing = chart->series();
    for (QAbstractSeries *s : existing) {
        if (s->name() == "Fitting Line") {
            chart->removeSeries(s);
            delete s;
            break;
        }
    }

    chart->addSeries(line);
    QList<QAbstractAxis *> axes = chart->axes(Qt::Horizontal);
    if (!axes.isEmpty())
        line->attachAxis(qobject_cast<QValueAxis *>(axes.first()));

    axes = chart->axes(Qt::Vertical);
    if (!axes.isEmpty())
        line->attachAxis(qobject_cast<QValueAxis *>(axes.first()));
}
