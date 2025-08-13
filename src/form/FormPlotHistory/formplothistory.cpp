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
    m_chartView14 = new MyChartView(new QChart(), this);
    m_chartView24 = new MyChartView(new QChart(), this);
    m_chartMix = new MyChartView(new QChart(), this);

    m_chartView14->setRenderHint(QPainter::Antialiasing);
    m_chartView24->setRenderHint(QPainter::Antialiasing);
    m_chartMix->setRenderHint(QPainter::Antialiasing);

    ui->stackedWidget->insertWidget(0, m_chartView14);
    ui->stackedWidget->insertWidget(1, m_chartView24);
    ui->stackedWidget->insertWidget(2, m_chartMix);

    m_chartView14Split = new MyChartView(new QChart(), this);
    m_chartView24Split = new MyChartView(new QChart(), this);
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

    // ui->tBtnPrev14->setIcon(QIcon(":/res/icons/go-prev.png"));
    ui->tBtnPrev14->setObjectName("go-prev");
    ui->tBtnPrev14->setToolTip("prev");
    // ui->tBtnNext14->setIcon(QIcon(":/res/icons/go-next.png"));
    ui->tBtnNext14->setObjectName("go-next");
    ui->tBtnNext14->setToolTip("next");
    // ui->tBtnPrev24->setIcon(QIcon(":/res/icons/go-prev.png"));
    ui->tBtnPrev24->setObjectName("go-prev");
    ui->tBtnPrev24->setToolTip("prev");
    // ui->tBtnNext24->setIcon(QIcon(":/res/icons/go-next.png"));
    ui->tBtnNext24->setObjectName("go-next");
    ui->tBtnNext24->setToolTip("next");

    QString fitting_k = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_K, "0.0");
    QString fitting_b = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_B, "0.0");
    ui->lineEditK->setText(fitting_k);
    ui->lineEditB->setText(fitting_b);
    QString conversion = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_CONVERSION, VAL_ENABLE);
    if (conversion == VAL_ENABLE) {
        ui->checkBoxConversion->setChecked(true);
    } else {
        ui->checkBoxConversion->setChecked(false);
    }
    ui->tBtnFittingKB->setCheckable(true);
    QString fitting_A = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_A, "0.0");
    QString fitting_y0 = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_y0, "0.0");
    QString fitting_w = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_w, "1.0");
    QString fitting_xc = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_xc, "0.0");
    ui->lineEdit_A->setText(fitting_A);
    ui->lineEdit_y0->setText(fitting_y0);
    ui->lineEdit_w->setText(fitting_w);
    ui->lineEdit_xc->setText(fitting_xc);
    ui->tBtnFittingSin->setCheckable(true);
    m_fitting = false;
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
        series24->setName("curve24");

        QLineSeries *series14 = new QLineSeries();
        series14->append(m_p14[m_index_14]);
        series14->setColor(Qt::magenta);
        series14->setName("curve14");

        chart->addSeries(series24);
        chart->addSeries(series14);

        chart->createDefaultAxes();
        setXAxisInteger(chart);
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
            series->setName("curve14");

            chart->addSeries(series);
            chart->createDefaultAxes();
            setXAxisInteger(chart);
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
            series->setName("curve24");

            chart->addSeries(series);
            chart->createDefaultAxes();
            setXAxisInteger(chart);
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
                series->setName("curve14");
                chart->setTitle(tr("curve_14bit"));
                m_chartView14Split->setChart(chart);
            } else {
                series->append(m_p24[m_index_24]);
                series->setColor(Qt::blue);
                series->setName("curve24");
                chart->setTitle(tr("curve_24bit"));
                m_chartView24Split->setChart(chart);
            }
            chart->addSeries(series);
            chart->createDefaultAxes();
            setXAxisInteger(chart);
            ui->stackedWidget->setCurrentIndex(3);
        }
    }
    if (m_fitting) {
        getFittingChart();
        clearFitting();
        drawFitting();
    }
}

void FormPlotHistory::closeEvent(QCloseEvent *event)
{
    m_p14.clear();
    m_p24.clear();
    updatePlot14();
    updatePlot24();

    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotHistory::onHistoryRecv(const QList<QPointF> &data14, const QList<QPointF> &data24)
{
    if (this->isVisible()) {
        m_p14.append(data14);
        m_p24.append(data24);
        m_index_14 = m_p14.size() - 1;
        m_index_24 = m_p24.size() - 1;
        ui->labelStatus14->setText(QString("%1/%2").arg(m_index_14 + 1).arg(m_p14.size()));
        ui->labelStatus24->setText(QString("%1/%2").arg(m_index_24 + 1).arg(m_p24.size()));
        updatePlot14();
        updatePlot24();
    }
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

void FormPlotHistory::getFittingChart()
{
    if (ui->radioButtonMix->isChecked()) {
        m_chart = m_chartMix->chart();
    } else if (ui->radioButtonSplit->isChecked()) {
        m_chart = m_chartView14Split->chart();
    } else {
        m_chart = m_chartView14->chart();
    }
}

void FormPlotHistory::clearFitting()
{
    QList<QAbstractSeries *> existing = m_chart->series();
    for (QAbstractSeries *s : existing) {
        if (s->name() == "Fitting Line") {
            m_chart->removeSeries(s);
            delete s;
            break;
        }
    }
}

double calcY(double x, double offset, double y0, double A, double xc, double w)
{
    double shiftedX = x + offset;
    return y0 + A * qSin(M_PI * (x - xc) / w);
}

double g_offset;
float g_y0, g_A, g_xc, g_w;

void FormPlotHistory::drawFittingSin()
{
    QString txt_A = ui->lineEdit_A->text();
    QString txt_y0 = ui->lineEdit_y0->text();
    QString txt_w = ui->lineEdit_w->text();
    QString txt_xc = ui->lineEdit_xc->text();

    if (txt_A.isEmpty() || txt_y0.isEmpty() || txt_w.isEmpty() || txt_xc.isEmpty()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Empty Params"), tr("A, y0, w, and xc can not be empty!"));
        return;
    }

    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_A, txt_A);
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_y0, txt_y0);
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_w, txt_w);
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_xc, txt_xc);

    bool okA, okY0, okW, okXc;
    float A = txt_A.toFloat(&okA);
    float y0 = txt_y0.toFloat(&okY0);
    float w = txt_w.toFloat(&okW);
    float xc = txt_xc.toFloat(&okXc);
    g_y0 = y0;
    g_A = A;
    g_xc = xc;
    g_w = w;

    if (!(okA && okY0 && okW && okXc)) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Wrong Params"), tr("Invalid number format."));
        return;
    }

    if (w == 0.0f) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Wrong Params"), tr("w can not be 0!"));
        return;
    }

    const QList<QPointF> &points = m_p14[m_index_14];
    if (points.isEmpty())
        return;

    QLineSeries *line = new QLineSeries();
    line->setColor(Qt::red);
    line->setName("Fitting Line");
    QPen pen(Qt::red);
    pen.setStyle(Qt::DashLine);
    line->setPen(pen);

    for (const QPointF &pt : points) {
        double x = pt.x();
        double y = y0 + A * qSin(M_PI * (x - xc) / w);
        line->append(x, y);
    }

    if (!m_chart)
        return;

    m_chart->addSeries(line);
    QList<QAbstractAxis *> axes = m_chart->axes(Qt::Horizontal);
    if (!axes.isEmpty())
        line->attachAxis(qobject_cast<QValueAxis *>(axes.first()));

    axes = m_chart->axes(Qt::Vertical);
    if (!axes.isEmpty())
        line->attachAxis(qobject_cast<QValueAxis *>(axes.first()));
}

void FormPlotHistory::drawFittingKB()
{
    QString txt_k = ui->lineEditK->text();
    QString txt_b = ui->lineEditB->text();
    if (txt_k.isEmpty() || txt_b.isEmpty()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Empty Params"), tr("k and b can not be empty!"));
        return;
    }

    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_K, txt_k);
    SETTING_CONFIG_SET(CFG_GROUP_PLOT, CFG_PLOT_FITTING_B, txt_b);
    SETTING_CONFIG_SET(CFG_GROUP_PLOT,
                       CFG_PLOT_FITTING_CONVERSION,
                       ui->checkBoxConversion->isChecked() ? VAL_ENABLE : VAL_DISABLE);

    float k = txt_k.toFloat();
    float b = txt_b.toFloat();
    if (ui->checkBoxConversion->isChecked()) {
        k = k / 0x1FFF * 3.3;
        b = b / 0x1FFF * 3.3;
    }
    const QList<QPointF> &points = m_p14[m_index_14];

    QLineSeries *line = new QLineSeries();
    line->setColor(Qt::red);
    line->setName("Fitting Line");
    QPen pen(Qt::red);
    pen.setStyle(Qt::DashLine);
    line->setPen(pen);

    for (const QPointF &pt : points) {
        double x = pt.x();
        double y = k * x + b;
        line->append(x, y);
    }

    if (!m_chart)
        return;

    m_chart->addSeries(line);
    QList<QAbstractAxis *> axes = m_chart->axes(Qt::Horizontal);
    if (!axes.isEmpty())
        line->attachAxis(qobject_cast<QValueAxis *>(axes.first()));

    axes = m_chart->axes(Qt::Vertical);
    if (!axes.isEmpty())
        line->attachAxis(qobject_cast<QValueAxis *>(axes.first()));
}

void FormPlotHistory::drawFitting()
{
    if (m_index_14 >= m_p14.size() || m_p14[m_index_14].isEmpty()) {
        LOG_WARN("curve 14 is empty!");
        return;
    }
    if (ui->tBtnFittingKB->isChecked()) {
        drawFittingKB();
    } else if (ui->tBtnFittingSin->isChecked()) {
        drawFittingSin();
    }
}

void FormPlotHistory::on_toolButtonDumpPlot_clicked()
{
    if (!ui->radioButtonMix->isChecked()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export Failed"), tr("Only support Mix!"));
    }
    QString dftName = QString("%1.png").arg(m_index_14 + 1);
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
        int groupCount = qMax(m_p14.size(), m_p24.size());
        for (int i = 0; i < groupCount; ++i) {
            out << QString(",14bit%1,24bit%2").arg(i + 1).arg(i + 1);
        }
        out << "\n";

        int maxPoints = 0;
        for (int i = 0; i < groupCount; ++i) {
            int len14 = (i < m_p14.size()) ? m_p14[i].size() : 0;
            int len24 = (i < m_p24.size()) ? m_p24[i].size() : 0;
            maxPoints = qMax(maxPoints, qMax(len14, len24));
        }

        for (int i = 0; i < maxPoints; ++i) {
            out << QString::number(i);
            for (int g = 0; g < groupCount; ++g) {
                QString y14 = (g < m_p14.size() && i < m_p14[g].size())
                                  ? QString::number(m_p14[g][i].y())
                                  : "";
                QString y24 = (g < m_p24.size() && i < m_p24[g].size())
                                  ? QString::number(m_p24[g][i].y())
                                  : "";
                out << "," << y14 << "," << y24;
            }
            out << "\n";
        }

        file.close();

        SHOW_AUTO_CLOSE_MSGBOX(this,
                               tr("Export Successful"),
                               tr("All data exported to:\n%1").arg(filePath));
    } else {
        QString dftName = QString("%1.csv").arg(m_index_14 + 1);
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

        int size = qMax(m_p14.size() > m_index_14 ? m_p14[m_index_14].size() : 0,
                        m_p24.size() > m_index_24 ? m_p24[m_index_24].size() : 0);

        const QList<QPointF> &list14 = (m_index_14 < m_p14.size()) ? m_p14[m_index_14]
                                                                   : QList<QPointF>();
        const QList<QPointF> &list24 = (m_index_24 < m_p24.size()) ? m_p24[m_index_24]
                                                                   : QList<QPointF>();

        for (int i = 0; i < size; ++i) {
            QString indexStr = QString::number(i);
            QString y14Str = (i < list14.size()) ? QString::number(list14[i].y()) : "";
            QString y24Str = (i < list24.size()) ? QString::number(list24[i].y()) : "";
            out << indexStr << "," << y14Str << "," << y24Str << "\n";
        }

        file.close();
        SHOW_AUTO_CLOSE_MSGBOX(this,
                               tr("Export Successful"),
                               tr("Data exported to:\n%1").arg(filePath));
    }
}

void FormPlotHistory::on_tBtnFittingSin_clicked()
{
    m_fitting = !m_fitting;
    ui->tBtnFittingSin->setChecked(m_fitting);
    getFittingChart();
    if (m_fitting) {
        clearFitting();
        drawFitting();
    } else {
        clearFitting();
    }
}

void FormPlotHistory::on_tBtnFittingKB_clicked()
{
    // 14bit
    m_fitting = !m_fitting;
    ui->tBtnFittingKB->setChecked(m_fitting);
    getFittingChart();
    if (m_fitting) {
        clearFitting();
        drawFitting();
    } else {
        clearFitting();
    }
}
