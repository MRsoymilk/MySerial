#include "formplot.h"
#include "ui_formplot.h"

#include <QTimer>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>

FormPlot::FormPlot(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlot)
{
    ui->setupUi(this);
    init();
}

FormPlot::~FormPlot()
{
    delete ui;
}

void FormPlot::init()
{
    m_series = new QLineSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_time = 0;

    m_chart->addSeries(m_series);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    m_axisX->setTitleText("Time (s)");
    m_axisX->setRange(0, 0.2);

    m_axisY->setTitleText("Voltage (V)");
    m_axisY->setRange(-2.5, 2.5);

    m_chart->legend()->hide();
    m_chart->setTitle("Live ADC Waveform");

    QChartView *view = new QChartView(m_chart);
    view->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(view);
}

void FormPlot::updateData(const QByteArray &data)
{
    QByteArray payload = data.mid(5, data.size() - 7);
    qDebug() << "payload: " << payload.size();
    if (payload.size() % 3 != 0) {
        qWarning() << "Invalid data length";
        return;
    }

    int numPoints = payload.size() / 3;
    m_series->clear();
    for (int i = 0; i < numPoints; ++i) {
        int idx = i * 3;
        quint32 raw = (quint8) payload[idx] << 16 | (quint8) payload[idx + 1] << 8
                      | (quint8) payload[idx + 2];
        qint32 signedRaw = static_cast<qint32>(raw);
        if (signedRaw & 0x800000)
            signedRaw |= 0xFF000000;

        double voltage = (signedRaw / double(1 << 23)) * 2.5;

        m_series->append(m_time + m_T * i, voltage);
    }
    m_axisX->setRange(m_time, m_time + numPoints * m_T);
    m_time += numPoints * m_T;
}

void FormPlot::onDataReceived(const QByteArray &data)
{
    updateData(data);
}
