#include "formplothistory.h"
#include "ui_formplothistory.h"

#include <QDir>
#include <QFileDialog>
#include <QLineSeries>
#include <QPixmap>
#include <QValueAxis>
#include <QMenu>
#include <QShortcut>
#include <QTextStream>
#include <QPainter>
#include <QCoreApplication>

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
    m_chart = new QChart();
    m_chart->setTitle(tr("curve_mix"));

    m_line31 = new QLineSeries;
    m_line31->setColor(Qt::blue);
    m_line31->setName("curve31");

    m_line33 = new QLineSeries;
    m_line33->setColor(Qt::magenta);
    m_line33->setName("curve33");

    m_chart->addSeries(m_line31);
    m_chart->addSeries(m_line33);

    m_axisX = new QValueAxis;
    m_axisY = new QValueAxis;

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_line31->attachAxis(m_axisX);
    m_line31->attachAxis(m_axisY);
    m_line33->attachAxis(m_axisX);
    m_line33->attachAxis(m_axisY);

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayMix->addWidget(m_chartView);

    ui->stackedWidget->setCurrentWidget(ui->pageMix);

    QShortcut *shortcut_prev = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcut_prev, &QShortcut::activated, this, &FormPlotHistory::on_tBtnPrev_clicked);

    QShortcut *shortcut_next = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(shortcut_next, &QShortcut::activated, this, &FormPlotHistory::on_tBtnNext_clicked);

    QShortcut *shortcut_delete = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut_delete, &QShortcut::activated, this, &FormPlotHistory::onMenuRemove);

    ui->tBtnToVoltage->setCheckable(true);
    ui->progressBarToPlot->setVisible(false);
}

void FormPlotHistory::updatePlot()
{
    if (m_data.isEmpty() || m_index < 0 || m_index >= m_data.size())
        return;

    const HISTORY_DATA &data = m_data[m_index];

    ui->labelTemperature->setText(QString("%1 ℃").arg(data.temperature));

    m_line31->replace(data.curve31.data);
    m_line33->replace(data.curve33.data);

    m_axisX->setRange(0,
                      std::max(data.curve31.x_max,
                               data.curve33.x_max));

    m_axisY->setRange(std::min(data.curve31.y_min,
                               data.curve33.y_min),
                      std::max(data.curve31.y_max,
                               data.curve33.y_max));

    ui->labelStatus->setText(
        QString("%1/%2").arg(m_index + 1).arg(m_data.size()));
}

void FormPlotHistory::on_tBtnNext_clicked()
{
    if (m_data.isEmpty())
        return;

    if (m_index + 1 < m_data.size())
        ++m_index;

    updatePlot();
}

void FormPlotHistory::on_tBtnPrev_clicked()
{
    if (m_data.isEmpty())
        return;

    if (m_index - 1 >= 0)
        --m_index;

    updatePlot();
}

void FormPlotHistory::closeEvent(QCloseEvent *event)
{
    m_data.clear();
    m_index = 0;

    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotHistory::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);

    QAction *clearAction = menu.addAction("Clear");
    menu.addSeparator();
    QAction *removeAction = menu.addAction("Remove");

    connect(clearAction, &QAction::triggered,
            this, &FormPlotHistory::onMenuClear);

    connect(removeAction, &QAction::triggered,
            this, &FormPlotHistory::onMenuRemove);

    menu.exec(event->globalPos());
}

void FormPlotHistory::onMenuClear()
{
    m_data.clear();
    m_index = 0;
    ui->labelStatus->setText("0/0");
}

void FormPlotHistory::onMenuRemove()
{
    if (m_data.isEmpty())
        return;

    m_data.removeAt(m_index);

    if (m_index >= m_data.size())
        m_index = m_data.size() - 1;

    if (m_index < 0)
        m_index = 0;

    updatePlot();
}

void FormPlotHistory::onHistoryRecv(const CURVE &curve31,
                                    const CURVE &curve33,
                                    const double &temperature,
                                    const QString &)
{
    if (!this->isVisible())
        return;

    HISTORY_DATA data;
    data.curve31 = curve31;
    data.curve33 = curve33;
    data.temperature = temperature;

    m_data.append(data);
    m_index = m_data.size() - 1;

    updatePlot();
}

void FormPlotHistory::onTemperature(double temperature)
{
    ui->labelTemperature->setText(QString("%1 ℃").arg(temperature));
}

void FormPlotHistory::on_lineEditGo_editingFinished()
{
    int val = ui->lineEditGo->text().toInt();

    if (val > 0 && val <= m_data.size()) {
        m_index = val - 1;
        updatePlot();
    }
}

void FormPlotHistory::on_tBtnDumpPlot_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Chart",
        QString("%1.png").arg(m_index + 1),
        "PNG Image (*.png);;JPEG Image (*.jpg)");

    if (filePath.isEmpty())
        return;

    QImage image(m_chartView->size(), QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    m_chartView->render(&painter);
    painter.end();

    image.save(filePath);

    SHOW_AUTO_CLOSE_MSGBOX(this,
                           tr("Export Successful"),
                           tr("Img exported to:\n%1").arg(filePath));
}

void FormPlotHistory::on_toolButtonDumpData_clicked()
{
    if (m_data.isEmpty())
        return;

    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Curve Data"),
        QString("%1.csv").arg(m_index + 1),
        tr("CSV Files (*.csv)"));

    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "index,curve31,curve33\n";

    const HISTORY_DATA &data = m_data[m_index];

    const QList<QPointF> &list31 = data.curve31.data;
    const QList<QPointF> &list33 = data.curve33.data;

    int size = qMax(list31.size(), list33.size());

    for (int i = 0; i < size; ++i) {
        QString x = (i < list31.size())
        ? QString::number(list31[i].x())
        : "";

        QString y31 = (i < list31.size())
                          ? QString::number(list31[i].y())
                          : "";

        QString y33 = (i < list33.size())
                          ? QString::number(list33[i].y())
                          : "";

        out << x << "," << y31 << "," << y33 << "\n";
    }

    file.close();

    SHOW_AUTO_CLOSE_MSGBOX(this,
                           tr("Export Successful"),
                           tr("Data exported to:\n%1").arg(filePath));
}

void FormPlotHistory::toPlot()
{
    if (m_data.isEmpty())
        return;

    const HISTORY_DATA &data = m_data[m_index];

    emit sendToPlot(data.curve31,
                    data.curve33,
                    data.temperature,
                    false);
}

void FormPlotHistory::on_tBtnToPlot_clicked()
{
    toPlot();
}

void FormPlotHistory::on_tBtnToPlotWith_clicked()
{
    if (!ui->checkBoxSendRange->isChecked()) {
        toPlot();
        return;
    }

    int start = ui->spinBoxRangeStart->value();
    int end = ui->spinBoxRangeEnd->value();

    ui->progressBarToPlot->setVisible(true);

    for (int i = start; i <= end; ++i) {

        if (i <= 0 || i > m_data.size())
            continue;

        m_index = i - 1;
        updatePlot();
        toPlot();

        ui->progressBarToPlot->setValue(
            (i - start) * 100.0 / (end - start));

        QCoreApplication::processEvents();
    }

    ui->progressBarToPlot->setVisible(false);
}

void FormPlotHistory::on_tBtnToVoltage_clicked()
{
    m_enableToVoltage = !m_enableToVoltage;
    ui->tBtnToVoltage->setChecked(m_enableToVoltage);
}

void FormPlotHistory::on_tBtnShowData_clicked()
{
}

void FormPlotHistory::on_tBtnDumpRaw_clicked()
{
}
