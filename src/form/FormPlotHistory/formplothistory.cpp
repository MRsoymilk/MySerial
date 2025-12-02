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
    m_chartViewMix = new MyChartView(new QChart(), this);
    m_chartViewMix->setRenderHint(QPainter::Antialiasing);
    ui->gLayMix->addWidget(m_chartViewMix);

    m_chartView31Split = new MyChartView(new QChart(), this);
    m_chartView33Split = new MyChartView(new QChart(), this);
    m_chartView31Split->setRenderHint(QPainter::Antialiasing);
    m_chartView33Split->setRenderHint(QPainter::Antialiasing);
    ui->gLay31->addWidget(m_chartView31Split);
    ui->gLay33->addWidget(m_chartView33Split);
    ui->stackedWidget->setCurrentWidget(ui->pageMix);

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
    QShortcut *shortcut_delete = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut_delete, &QShortcut::activated, this, &FormPlotHistory::onMenuRemove);

    m_chartMix = new QChart();
    m_chartMix->setTitle(tr("curve_mix"));

    m_lineMix31 = new QLineSeries;
    m_lineMix31->setColor(Qt::blue);
    m_lineMix31->setName("curve31");

    m_lineMix33 = new QLineSeries;
    m_lineMix33->setColor(Qt::magenta);
    m_lineMix33->setName("curve33");

    m_chartMix->addSeries(m_lineMix31);
    m_chartMix->addSeries(m_lineMix33);

    m_axisMixX = new QValueAxis;
    m_axisMixY = new QValueAxis;
    m_chartMix->addAxis(m_axisMixX, Qt::AlignBottom);
    m_chartMix->addAxis(m_axisMixY, Qt::AlignLeft);
    m_lineMix31->attachAxis(m_axisMixX);
    m_lineMix31->attachAxis(m_axisMixY);
    m_lineMix33->attachAxis(m_axisMixX);
    m_lineMix33->attachAxis(m_axisMixY);
    m_chartViewMix->setChart(m_chartMix);

    // ------------------ 31 ------------------
    m_splitLine31 = new QLineSeries;
    m_chart31 = new QChart();
    m_chart31->addSeries(m_splitLine31);

    m_axisSplit31X = new QValueAxis;
    m_axisSplit31Y = new QValueAxis;

    m_chart31->addAxis(m_axisSplit31X, Qt::AlignBottom);
    m_chart31->addAxis(m_axisSplit31Y, Qt::AlignLeft);
    m_splitLine31->attachAxis(m_axisSplit31X);
    m_splitLine31->attachAxis(m_axisSplit31Y);

    m_chartView31Split->setChart(m_chart31);

    // ------------------ 33 ------------------
    m_splitLine33 = new QLineSeries;
    m_chart33 = new QChart();
    m_chart33->addSeries(m_splitLine33);

    m_axisSplit33X = new QValueAxis;
    m_axisSplit33Y = new QValueAxis;

    m_chart33->addAxis(m_axisSplit33X, Qt::AlignBottom);
    m_chart33->addAxis(m_axisSplit33Y, Qt::AlignLeft);
    m_splitLine33->attachAxis(m_axisSplit33X);
    m_splitLine33->attachAxis(m_axisSplit33Y);

    m_chartView33Split->setChart(m_chart33);
}

void FormPlotHistory::on_tBtnNext14_clicked()
{
    if (m_p31.empty()) {
        return;
    }
    if (m_index_31 + 1 < m_p31.size()) {
        m_index_31++;
    } else {
        m_index_31 = m_p31.size() - 1;
    }
    updatePlot31();
}

void FormPlotHistory::on_tBtnPrev14_clicked()
{
    if (m_p31.empty()) {
        return;
    }
    if (m_index_31 - 1 >= 0) {
        m_index_31--;
    } else {
        m_index_31 = 0;
    }
    updatePlot31();
}

void FormPlotHistory::on_tBtnNext24_clicked()
{
    if (m_p33.empty()) {
        return;
    }
    if (m_index_33 + 1 < m_p33.size()) {
        m_index_33++;
    } else {
        m_index_33 = m_p33.size() - 1;
    }
    updatePlot33();
}

void FormPlotHistory::on_tBtnPrev24_clicked()
{
    if (m_p33.empty()) {
        return;
    }
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

        m_lineMix31->replace(m_p31[m_index_31].data);
        m_lineMix33->replace(m_p33[m_index_33].data);

        m_axisMixX->setRange(0, std::max(m_p31[m_index_31].x_max, m_p33[m_index_33].x_max));
        m_axisMixY->setRange(std::min(m_p31[m_index_31].y_min, m_p33[m_index_33].y_min),
                             std::max(m_p31[m_index_31].y_max, m_p33[m_index_33].y_max));
        ui->stackedWidget->setCurrentWidget(ui->pageMix);
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

            if (index == INDEX_31) {
                if (m_index_31 < m_p31.size()) {
                    m_splitLine31->replace(m_p31[m_index_31].data);
                    m_axisSplit31X->setRange(m_p31[m_index_31].x_min, m_p31[m_index_31].x_max);
                    m_axisSplit31Y->setRange(m_p31[m_index_31].y_min, m_p31[m_index_31].y_max);
                }
            } else {
                if (m_index_33 < m_p33.size()) {
                    m_splitLine33->replace(m_p33[m_index_33].data);
                    m_axisSplit33X->setRange(m_p33[m_index_33].x_min, m_p33[m_index_33].x_max);
                    m_axisSplit33Y->setRange(m_p33[m_index_33].y_min, m_p33[m_index_33].y_max);
                }
            }

            ui->stackedWidget->setCurrentWidget(ui->pageSplit);
        }
    }
    ui->labelStatus31->setText(QString("%1/%2").arg(m_index_31 + 1).arg(m_p31.size()));
    ui->labelStatus33->setText(QString("%1/%2").arg(m_index_33 + 1).arg(m_p33.size()));
}

void FormPlotHistory::closeEvent(QCloseEvent *event)
{
    m_p31.clear();
    QList<CURVE>().swap(m_p31);
    m_p33.clear();
    QList<CURVE>().swap(m_p33);
    m_temperature.clear();
    updatePlot31();
    updatePlot33();

    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotHistory::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *removeAction = new QAction("remove", &menu);
    menu.addAction(removeAction);

    connect(removeAction, &QAction::triggered, this, &FormPlotHistory::onMenuRemove);

    menu.exec(event->globalPos());
}

void FormPlotHistory::onMenuRemove()
{
    if (m_index_31 > 0 && m_index_33 > 0) {
        m_temperature.removeAt(m_index_33);
        m_p31.removeAt(m_index_31);
        m_p33.removeAt(m_index_33);
        --m_index_31;
        --m_index_33;
        ui->labelStatus31->setText(QString("%1/%2").arg(m_index_31 + 1).arg(m_p31.size()));
        ui->labelStatus33->setText(QString("%1/%2").arg(m_index_33 + 1).arg(m_p33.size()));
        updatePlot31();
        updatePlot33();
    }
}

void FormPlotHistory::onHistoryRecv(const CURVE &data31,
                                    const CURVE &data33,
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
        m_chart = m_chartViewMix->chart();
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
    if (!m_chartViewMix) {
        return;
    }

    QSize size = m_chartViewMix->size();

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    m_chartViewMix->render(&painter);
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
            out << QString(",curve31_%1,curve33_%2").arg(i + 1).arg(i + 1);
        }
        out << "\n";

        int maxPoints = 0;
        for (int i = 0; i < groupCount; ++i) {
            int len31 = (i < m_p31.size()) ? m_p31[i].data.size() : 0;
            int len33 = (i < m_p33.size()) ? m_p33[i].data.size() : 0;
            maxPoints = qMax(maxPoints, qMax(len31, len33));
        }

        for (int i = 0; i < maxPoints; ++i) {
            out << QString::number(i);
            for (int g = 0; g < groupCount; ++g) {
                QString y31 = (g < m_p31.size() && i < m_p31[g].data.size())
                                  ? QString::number(m_p31[g].data[i].y())
                                  : "";
                QString y33 = (g < m_p33.size() && i < m_p33[g].data.size())
                                  ? QString::number(m_p33[g].data[i].y())
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
        out << "index,curve31,curve33\n";

        int size = qMax(m_p31.size() > m_index_31 ? m_p31[m_index_31].data.size() : 0,
                        m_p33.size() > m_index_33 ? m_p33[m_index_33].data.size() : 0);

        const QList<QPointF> &list31 = (m_index_31 < m_p31.size()) ? m_p31[m_index_31].data
                                                                   : QList<QPointF>();
        const QList<QPointF> &list33 = (m_index_33 < m_p33.size()) ? m_p33[m_index_33].data
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
    CURVE curve31;
    CURVE curve33;
    double temperature = 0;
    if (m_index_31 < m_p31.size()) {
        curve31 = m_p31[m_index_31];
    }
    if (m_index_33 < m_p33.size()) {
        curve33 = m_p33[m_index_33];
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

    emit sendToPlot(curve31, curve33, temperature, false);
}
