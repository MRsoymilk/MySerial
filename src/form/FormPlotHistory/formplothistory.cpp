#include "formplothistory.h"
#include "ui_formplothistory.h"

#include <QDir>
#include <QFileDialog>
#include <QLineSeries>
#include <QPixmap>
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
    delete ui;
}

void FormPlotHistory::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormPlotHistory::init()
{
    m_widgetMix = new QWidget(this);
    m_chartMix = new MyChartView(new QChart(), m_widgetMix);
    m_chartMix->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *vLayMix = new QVBoxLayout(m_widgetMix);
    vLayMix->addWidget(m_chartMix);
    vLayMix->setStretch(0, 1);
    vLayMix->setContentsMargins(0, 0, 0, 0);
    m_widgetMix->setLayout(vLayMix);
    ui->stackedWidget->insertWidget(0, m_widgetMix);

    m_widgetSplit = new QWidget(this);
    m_chartView31Split = new MyChartView(new QChart(), m_widgetSplit);
    m_chartView33Split = new MyChartView(new QChart(), m_widgetSplit);
    m_chartView31Split->setRenderHint(QPainter::Antialiasing);
    m_chartView33Split->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *vLaySplit = new QVBoxLayout(m_widgetSplit);
    vLaySplit->addWidget(m_chartView31Split);
    vLaySplit->addWidget(m_chartView33Split);
    vLaySplit->setStretch(0, 1);
    vLaySplit->setStretch(1, 1);
    vLaySplit->setContentsMargins(0, 0, 0, 0);
    m_widgetSplit->setLayout(vLaySplit);
    ui->stackedWidget->insertWidget(1, m_widgetSplit);
    ui->stackedWidget->setCurrentIndex(0);

    ui->radioButtonMix->setChecked(true);

    ui->tBtnPrev14->setObjectName("go-prev");
    ui->tBtnPrev14->setToolTip(tr("prev"));
    ui->tBtnNext14->setObjectName("go-next");
    ui->tBtnNext14->setToolTip(tr("next"));
    ui->tBtnPrev24->setObjectName("go-prev");
    ui->tBtnPrev24->setToolTip(tr("prev"));
    ui->tBtnNext24->setObjectName("go-next");
    ui->tBtnNext24->setToolTip(tr("next"));

    QShortcut *shortcut_prev = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcut_prev, &QShortcut::activated, this, &FormPlotHistory::on_tBtnPrev14_clicked);
    QShortcut *shortcut_next = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(shortcut_next, &QShortcut::activated, this, &FormPlotHistory::on_tBtnNext14_clicked);
}

void FormPlotHistory::on_tBtnNext14_clicked()
{
    if (m_index_31 + 1 < m_p31.size()) {
        m_index_31++;
    } else {
        m_index_31 = m_p31.size() - 1;
    }
    updatePlot31();
}

void FormPlotHistory::on_tBtnPrev14_clicked()
{
    if (m_index_31 - 1 >= 0) {
        m_index_31--;
    } else {
        m_index_31 = 0;
    }
    updatePlot31();
}

void FormPlotHistory::on_tBtnNext24_clicked()
{
    if (m_index_33 + 1 < m_p33.size()) {
        m_index_33++;
    } else {
        m_index_33 = m_p33.size() - 1;
    }
    updatePlot33();
}

void FormPlotHistory::on_tBtnPrev24_clicked()
{
    if (m_index_33 - 1 >= 0) {
        m_index_33--;
    } else {
        m_index_33 = 0;
    }
    updatePlot33();
}

void FormPlotHistory::updatePlot31()
{
    if (m_index_31 >= m_p31.size() || m_p31.empty()) {
        QChart *chart = m_chartView31Split->chart();
        for (QAbstractSeries *series : chart->series()) {
            chart->removeSeries(series);
            delete series;
        }
        ui->labelStatus31->setText(tr("status"));
        return;
    }

    updatePlot(INDEX_31);
}

void FormPlotHistory::updatePlot33()
{
    if (m_index_33 >= m_p33.size() || m_p33.empty()) {
        QChart *chart = m_chartView33Split->chart();
        for (QAbstractSeries *series : chart->series()) {
            chart->removeSeries(series);
            delete series;
        }
        ui->labelStatus33->setText(tr("status"));
        return;
    }

    updatePlot(INDEX_33);
}

void FormPlotHistory::updatePlot(int index)
{
    bool isMix = ui->radioButtonMix->isChecked();
    bool isSplit = ui->radioButtonSplit->isChecked();
    auto setXAxisInteger = [](QChart *chart) {
        QList<QAbstractAxis *> axesX = chart->axes(Qt::Horizontal);
        if (!axesX.isEmpty()) {
            QValueAxis *axisX = qobject_cast<QValueAxis *>(axesX.first());
            if (axisX) {
                axisX->setLabelFormat("%d");
                axisX->setTickType(QValueAxis::TicksFixed);
            }
        }
    };
    if (isMix) {
        if (index == INDEX_31) {
            if (m_index_31 < m_p33.size()) {
                m_index_33 = m_index_31;
            } else {
                return;
            }
        } else if (index == INDEX_33) {
            if (m_index_33 < m_p31.size()) {
                m_index_31 = m_index_33;
            } else {
                return;
            }
        }
        ui->labelTemperature->setText(QString("%1 ℃").arg(m_temperature.at(m_index_33)));
        QChart *chart = new QChart();

        QLineSeries *series33 = new QLineSeries();
        series33->append(m_p33[m_index_33]);
        series33->setColor(Qt::magenta);
        series33->setName(tr("curve33"));

        QLineSeries *series31 = new QLineSeries();
        series31->append(m_p31[m_index_31]);
        series31->setColor(Qt::blue);
        series31->setName(tr("curve31"));

        chart->setTitle(tr("curve_mix"));
        m_chartMix->setChart(chart);
        chart->addSeries(series33);
        chart->addSeries(series31);
        chart->createDefaultAxes();
        setXAxisInteger(chart);
        ui->stackedWidget->setCurrentWidget(m_widgetMix);
    } else {
        if (isSplit) {
            if (index == INDEX_31) {
                if (m_index_31 < m_p33.size()) {
                    m_index_33 = m_index_31;
                } else {
                    return;
                }
            } else if (index == INDEX_33) {
                if (m_index_33 < m_p31.size()) {
                    m_index_31 = m_index_33;
                } else {
                    return;
                }
            }
            QChart *chart = new QChart();
            QLineSeries *series = new QLineSeries();
            if (index == INDEX_31) {
                series->append(m_p31[m_index_31]);
                series->setColor(Qt::blue);
                series->setName(tr("curve_31"));
                chart->setTitle(tr("curve_31"));
                m_chartView31Split->setChart(chart);
            } else {
                series->append(m_p33[m_index_33]);
                series->setColor(Qt::magenta);
                series->setName(tr("curve_33"));
                chart->setTitle(tr("curve_33"));
                m_chartView33Split->setChart(chart);
            }
            chart->addSeries(series);
            chart->createDefaultAxes();
            setXAxisInteger(chart);
            ui->stackedWidget->setCurrentWidget(m_widgetSplit);
        }
    }
    ui->labelStatus31->setText(QString("%1/%2").arg(m_index_31 + 1).arg(m_p31.size()));
    ui->labelStatus33->setText(QString("%1/%2").arg(m_index_33 + 1).arg(m_p33.size()));
}

void FormPlotHistory::closeEvent(QCloseEvent *event)
{
    m_p31.clear();
    m_p33.clear();
    m_temperature.clear();
    updatePlot31();
    updatePlot33();

    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotHistory::onHistoryRecv(const QList<QPointF> &data31,
                                    const QList<QPointF> &data33,
                                    const double &temperature)
{
    if (this->isVisible()) {
        m_p31.append(data31);
        m_p33.append(data33);
        m_temperature.append(temperature);
        m_index_31 = m_p31.size() - 1;
        m_index_33 = m_p33.size() - 1;
        ui->labelStatus31->setText(QString("%1/%2").arg(m_index_31 + 1).arg(m_p31.size()));
        ui->labelStatus33->setText(QString("%1/%2").arg(m_index_33 + 1).arg(m_p33.size()));
        updatePlot31();
        updatePlot33();
        ui->labelTemperature->setText(QString("%1 ℃").arg(temperature));
    }
}

void FormPlotHistory::onTemperature(double temperature)
{
    if (this->isVisible()) {
        ui->labelTemperature->setText(QString("%1 ℃").arg(temperature));
    }
}

void FormPlotHistory::on_lineEdit14Go_editingFinished()
{
    int val = ui->lineEdit14Go->text().toInt();
    if (val > 0 && val <= m_p31.size()) {
        m_index_31 = val - 1;
        updatePlot(INDEX_31);
    }
}

void FormPlotHistory::on_lineEdit24Go_editingFinished()
{
    int val = ui->lineEdit24Go->text().toInt();
    if (val > 0 && val <= m_p33.size()) {
        m_index_33 = val - 1;
        updatePlot(INDEX_33);
    }
}

void FormPlotHistory::on_radioButtonMix_clicked()
{
    updatePlot(INDEX_31);
}

void FormPlotHistory::on_radioButtonSplit_clicked()
{
    updatePlot(INDEX_31);
}

void FormPlotHistory::getFittingChart()
{
    if (ui->radioButtonMix->isChecked()) {
        m_chart = m_chartMix->chart();
    } else if (ui->radioButtonSplit->isChecked()) {
        m_chart = m_chartView31Split->chart();
    }
}

void FormPlotHistory::on_toolButtonDumpPlot_clicked()
{
    if (!ui->radioButtonMix->isChecked()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export Failed"), tr("Only support Mix!"));
    }
    QString dftName = QString("%1.png").arg(m_index_31 + 1);
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    "Save Chart",
                                                    dftName,
                                                    "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (!m_chartMix) {
        return;
    }

    QSize size = m_chartMix->size();

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    m_chartMix->render(&painter);
    painter.end();

    image.save(filePath);
    SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export Successful"), tr("Img exported to:\n%1").arg(filePath));
}

void FormPlotHistory::on_toolButtonDumpData_clicked()
{
    if (!ui->radioButtonMix->isChecked()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export Failed"), tr("Only support Mix!"));
    }
    if (ui->checkBoxAll->isChecked()) {
        QString filePath = QFileDialog::getSaveFileName(this,
                                                        tr("Save Curve Data"),
                                                        "_all.csv",
                                                        tr("CSV Files (*.csv)"));
        if (filePath.isEmpty()) {
            return;
        }
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            SHOW_AUTO_CLOSE_MSGBOX(this, tr("Failed to Open File"), file.errorString());
            return;
        }

        QTextStream out(&file);

        out << "index";
        int groupCount = qMax(m_p31.size(), m_p33.size());
        for (int i = 0; i < groupCount; ++i) {
            out << QString(",14bit%1,24bit%2").arg(i + 1).arg(i + 1);
        }
        out << "\n";

        int maxPoints = 0;
        for (int i = 0; i < groupCount; ++i) {
            int len31 = (i < m_p31.size()) ? m_p31[i].size() : 0;
            int len33 = (i < m_p33.size()) ? m_p33[i].size() : 0;
            maxPoints = qMax(maxPoints, qMax(len31, len33));
        }

        for (int i = 0; i < maxPoints; ++i) {
            out << QString::number(i);
            for (int g = 0; g < groupCount; ++g) {
                QString y31 = (g < m_p31.size() && i < m_p31[g].size())
                                  ? QString::number(m_p31[g][i].y())
                                  : "";
                QString y33 = (g < m_p33.size() && i < m_p33[g].size())
                                  ? QString::number(m_p33[g][i].y())
                                  : "";
                out << "," << y31 << "," << y33;
            }
            out << "\n";
        }

        file.close();

        SHOW_AUTO_CLOSE_MSGBOX(this,
                               tr("Export Successful"),
                               tr("All data exported to:\n%1").arg(filePath));
    } else {
        QString dftName = QString("%1.csv").arg(m_index_31 + 1);
        QString filePath = QFileDialog::getSaveFileName(this,
                                                        tr("Save Curve Data"),
                                                        dftName,
                                                        tr("CSV Files (*.csv)"));
        if (filePath.isEmpty()) {
            return;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            SHOW_AUTO_CLOSE_MSGBOX(this, tr("Failed to Open File"), file.errorString());
            return;
        }

        QTextStream out(&file);
        out << "index,14bit,24bit\n";

        int size = qMax(m_p31.size() > m_index_31 ? m_p31[m_index_31].size() : 0,
                        m_p33.size() > m_index_33 ? m_p33[m_index_33].size() : 0);

        const QList<QPointF> &list31 = (m_index_31 < m_p31.size()) ? m_p31[m_index_31]
                                                                   : QList<QPointF>();
        const QList<QPointF> &list33 = (m_index_33 < m_p33.size()) ? m_p33[m_index_33]
                                                                   : QList<QPointF>();

        for (int i = 0; i < size; ++i) {
            QString indexStr = (i < list33.size()) ? QString::number(list33[i].x()) : "";
            QString y31Str = (i < list31.size()) ? QString::number(list31[i].y()) : "";
            QString y33Str = (i < list33.size()) ? QString::number(list33[i].y()) : "";
            out << indexStr << "," << y31Str << "," << y33Str << "\n";
        }

        file.close();
        SHOW_AUTO_CLOSE_MSGBOX(this,
                               tr("Export Successful"),
                               tr("Data exported to:\n%1").arg(filePath));
    }
}

void FormPlotHistory::on_tBtnToPlot_clicked()
{
    getFittingChart();
    QList<QPointF> data31;
    QList<QPointF> data33;
    double temperature = 0;
    if (m_index_31 < m_p31.size()) {
        data31 = m_p31[m_index_31];
    }
    if (m_index_33 < m_p33.size()) {
        data33 = m_p33[m_index_33];
    }
    if (m_index_33 < m_temperature.size()) {
        temperature = m_temperature[m_index_33];
    }

    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;
    auto axesX = m_chart->axes(Qt::Horizontal);
    if (!axesX.isEmpty()) {
        axisX = qobject_cast<QValueAxis *>(axesX.first());
    }

    auto axesY = m_chart->axes(Qt::Vertical);
    if (!axesY.isEmpty()) {
        axisY = qobject_cast<QValueAxis *>(axesY.first());
    }

    if (!axisX || !axisY) {
        qWarning() << "No valid axes found!";
        return;
    }

    double xMin = axisX->min();
    double xMax = axisX->max();
    double yMin = axisY->min();
    double yMax = axisY->max();

    emit sendToPlot(data31, data33, xMin, xMax, yMin, yMax, temperature, false);
}
