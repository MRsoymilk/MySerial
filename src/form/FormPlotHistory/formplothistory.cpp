#include "formplothistory.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QLineSeries>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QShortcut>
#include <QTextStream>
#include <QValueAxis>

#include "ShowData/showdata.h"
#include "funcdef.h"
#include "ui_formplothistory.h"

FormPlotHistory::FormPlotHistory(QWidget *parent) : QWidget(parent), ui(new Ui::FormPlotHistory) {
    ui->setupUi(this);
    init();
}

FormPlotHistory::~FormPlotHistory() { delete ui; }

void FormPlotHistory::retranslateUI() { ui->retranslateUi(this); }

void FormPlotHistory::init() {
    m_showData = new ShowData;
    m_showData->setVisible(false);
    connect(m_showData, &ShowData::windowClose, this, [=]() {
        ui->tBtnShowData->setChecked(false);
        m_enableShowData = false;
    });

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

    m_line31->setUseOpenGL(true);
    m_line33->setUseOpenGL(true);
    m_chartView->setRenderHint(QPainter::Antialiasing, false);

    ui->stackedWidget->setCurrentWidget(ui->pageMix);

    QShortcut *shortcut_prev = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcut_prev, &QShortcut::activated, this, &FormPlotHistory::on_tBtnPrev_clicked);

    QShortcut *shortcut_next = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(shortcut_next, &QShortcut::activated, this, &FormPlotHistory::on_tBtnNext_clicked);

    QShortcut *shortcut_delete = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut_delete, &QShortcut::activated, this, &FormPlotHistory::onMenuRemove);

    ui->progressBarToPlot->setVisible(false);

    ui->tBtnNext->setObjectName("go-next");
    ui->tBtnPrev->setObjectName("go-prev");
    ui->tBtnDumpData->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->tBtnDumpPlot->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->tBtnDumpRaw->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->tBtnToPlot->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->tBtnToPlotWith->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->tBtnShowData->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->tBtnToVoltage->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

void FormPlotHistory::updatePlot() {
    if (m_data.isEmpty() || m_index < 0 || m_index >= m_data.size()) return;

    const MY_DATA &data = m_data[m_index];

    ui->labelTemperature->setText(QString("%1 ℃").arg(data.temperature));

    if (m_enableToVoltage) {
        m_line31->replace(data.curve31.data);
        m_line33->replace(data.curve33.data);

        m_axisX->setRange(0, std::max(data.curve31.x_max, data.curve33.x_max));

        m_axisY->setRange(std::min(data.curve31.y_min, data.curve33.y_min),
                          std::max(data.curve31.y_max, data.curve33.y_max));
    } else {
        m_line31->replace(data.curve31.raw.data);
        m_line33->replace(data.curve33.raw.data);

        m_axisX->setRange(0, std::max(data.curve31.raw.x_max, data.curve33.raw.x_max));

        m_axisY->setRange(std::min(data.curve31.raw.y_min, data.curve33.raw.y_min),
                          std::max(data.curve31.raw.y_max, data.curve33.raw.y_max));
    }

    ui->labelStatus->setText(QString("%1/%2").arg(m_index + 1).arg(m_data.size()));
}

void FormPlotHistory::on_tBtnNext_clicked() {
    if (m_data.isEmpty()) return;

    if (m_index + 1 < m_data.size()) ++m_index;

    updatePlot();
}

void FormPlotHistory::on_tBtnPrev_clicked() {
    if (m_data.isEmpty()) return;

    if (m_index - 1 >= 0) --m_index;

    updatePlot();
}

void FormPlotHistory::closeEvent(QCloseEvent *event) {
    if (m_showData) {
        m_showData->close();
    }

    m_data.clear();
    m_index = 0;

    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotHistory::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    QAction *clearAction = menu.addAction(tr("Clear"));
    menu.addSeparator();
    QAction *removeAction = menu.addAction(tr("Remove"));

    connect(clearAction, &QAction::triggered, this, &FormPlotHistory::onMenuClear);

    connect(removeAction, &QAction::triggered, this, &FormPlotHistory::onMenuRemove);

    menu.exec(event->globalPos());
}

void FormPlotHistory::onMenuClear() {
    m_data.clear();
    m_index = 0;
    ui->labelStatus->setText("0/0");
}

void FormPlotHistory::onMenuRemove() {
    if (m_data.isEmpty()) return;

    m_data.removeAt(m_index);

    if (m_index >= m_data.size()) m_index = m_data.size() - 1;

    if (m_index < 0) m_index = 0;

    updatePlot();
}

void FormPlotHistory::onHistoryRecv(const MY_DATA &my_data) {
    if (!this->isVisible()) return;

    m_data.append(my_data);
    m_index = m_data.size() - 1;

    updatePlot();
}

void FormPlotHistory::onTemperature(double temperature) {
    ui->labelTemperature->setText(QString("%1 ℃").arg(temperature));
}

void FormPlotHistory::on_lineEditGo_editingFinished() {
    int val = ui->lineEditGo->text().toInt();

    if (val > 0 && val <= m_data.size()) {
        m_index = val - 1;
        updatePlot();
    }
}

void FormPlotHistory::on_tBtnDumpPlot_clicked() {
    QString filePath = QFileDialog::getSaveFileName(this, "Save Chart", QString("%1.png").arg(m_index + 1),
                                                    "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (filePath.isEmpty()) return;

    m_line31->setUseOpenGL(false);
    m_line33->setUseOpenGL(false);

    m_chartView->repaint();
    QCoreApplication::processEvents();

    QImage image(m_chartView->size(), QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    m_chartView->render(&painter);
    painter.end();

    m_line31->setUseOpenGL(true);
    m_line33->setUseOpenGL(true);

    image.save(filePath);

    SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export Successful"), tr("Img exported to:\n%1").arg(filePath));
}

void FormPlotHistory::on_tBtnDumpData_clicked() {
    if (m_data.isEmpty()) {
        QMessageBox::warning(this, TITLE_WARNING, "No data to export.");
        return;
    }

    QString fileName = QString("data_%1.csv").arg(m_index + 1);
    if (ui->checkBoxDataAll->isChecked()) {
        fileName = "data_all.csv";
    }
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Curve Data"), fileName, "CSV Files (*.csv)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);

    QList<MY_DATA> data_todo;
    if (ui->checkBoxDataAll->isChecked())
        data_todo = m_data;
    else
        data_todo = {m_data[m_index]};

    out << "index";
    for (int i = 0; i < data_todo.size(); ++i) {
        out << ",gVol" << (i + 1) << "_curve31"
            << ",gVol" << (i + 1) << "_curve33"
            << ",gRaw" << (i + 1) << "_curve31"
            << ",gRaw" << (i + 1) << "_curve33";
    }
    out << "\n";

    int maxSize = 0;
    for (const auto &data : data_todo) {
        maxSize = qMax(maxSize, qMax(data.curve31.data.size(), data.curve33.data.size()));
    }

    // ===== 按行写入 =====
    for (int row = 0; row < maxSize; ++row) {
        out << row;
        for (const auto &data : data_todo) {
            const auto &vol_list31 = data.curve31.data;
            const auto &vol_list33 = data.curve33.data;
            const auto &raw_list31 = data.curve31.raw.data;
            const auto &raw_list33 = data.curve33.raw.data;
            QString yVol_31 = (row < vol_list31.size()) ? QString::number(vol_list31[row].y()) : "";
            QString yVol_33 = (row < vol_list33.size()) ? QString::number(vol_list33[row].y()) : "";
            QString yRaw_31 = (row < raw_list31.size()) ? QString::number(raw_list31[row].y()) : "";
            QString yRaw_33 = (row < raw_list33.size()) ? QString::number(raw_list33[row].y()) : "";
            out << "," << yVol_31 << "," << yVol_33 << "," << yRaw_31 << "," << yRaw_33;
        }
        out << "\n";
    }

    file.close();

    SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export Successful"), tr("Data exported to:\n%1").arg(filePath));
}

void FormPlotHistory::toPlot() {
    if (m_data.isEmpty()) {
        return;
    }

    const MY_DATA &data = m_data[m_index];

    emit sendToPlot(data, false);
}

void FormPlotHistory::on_tBtnToPlot_clicked() { toPlot(); }

void FormPlotHistory::on_tBtnToPlotWith_clicked() {
    if (!ui->checkBoxSendRange->isChecked()) {
        toPlot();
        return;
    }

    int start = ui->spinBoxRangeStart->value();
    int end = ui->spinBoxRangeEnd->value();

    ui->progressBarToPlot->setVisible(true);

    for (int i = start; i <= end; ++i) {
        if (i <= 0 || i > m_data.size()) continue;

        m_index = i - 1;
        updatePlot();
        toPlot();

        ui->progressBarToPlot->setValue((i - start) * 100.0 / (end - start));

        QCoreApplication::processEvents();
    }

    ui->progressBarToPlot->setVisible(false);
}

void FormPlotHistory::on_tBtnToVoltage_clicked() {
    m_enableToVoltage = !m_enableToVoltage;
    ui->tBtnToVoltage->setChecked(m_enableToVoltage);
    updatePlot();
}

void FormPlotHistory::on_tBtnShowData_clicked() {
    m_enableShowData = !m_enableShowData;
    ui->tBtnShowData->setChecked(m_enableShowData);
    m_showData->setVisible(m_enableShowData);
    if (m_enableShowData) {
        if (!m_data.empty()) {
            m_showData->showData(m_data[m_index]);
        }
    }
}

void FormPlotHistory::on_tBtnDumpRaw_clicked() {
    if (m_data.isEmpty()) {
        QMessageBox::warning(this, TITLE_WARNING, tr("No data to export."));
        return;
    }

    QString fileName = QString("raw_%1.csv").arg(m_index + 1);
    if (ui->checkBoxRawAll->isChecked()) {
        fileName = "raw_all.csv";
    }
    QString filePath = QFileDialog::getSaveFileName(this, "Save Raw Data", fileName, "Text Files (*.txt)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, TITLE_ERROR, tr("Cannot open file."));
        return;
    }

    QTextStream out(&file);

    QList<MY_DATA> data_todo;
    if (ui->checkBoxRawAll->isChecked()) {
        data_todo = m_data;
    } else {
        data_todo = {m_data.at(m_index)};
    }

    for (const MY_DATA &data : data_todo) {
        if (data.frame.isEmpty()) continue;

        out << data.frame << "\n";
    }

    file.close();

    QMessageBox::information(this, TITLE_INFO, tr("Export finished."));
}
