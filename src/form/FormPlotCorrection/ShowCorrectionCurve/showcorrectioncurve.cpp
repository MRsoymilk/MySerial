#include "showcorrectioncurve.h"
#include "DataInput/datainput.h"
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

void ShowCorrectionCurve::retranslateUI()
{
    ui->retranslateUi(this);
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
    m_axisX->setTitleText(tr("wavelength"));
    m_axisY->setTitleText(tr("intensity"));
    m_chart->setTitle(tr("correction curve"));
    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->stackedWidget->addWidget(m_chartView);
    ui->stackedWidget->setCurrentWidget(m_chartView);

    m_current_page = 0;

    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, "index");
    m_model->setHeaderData(1, Qt::Horizontal, "threshold");
    ui->tableView->setModel(m_model);

    ui->doubleSpinBoxOffset->setValue(900);
    ui->doubleSpinBoxStep->setValue(1.5);

    m_load_data = false;
    ui->tBtnLoadDataFromInput->setCheckable(true);
    ui->tBtnLoadDataFromCSV->setCheckable(true);
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

void ShowCorrectionCurve::on_tBtnLoadDataFromInput_clicked()
{
    m_load_data = !m_load_data;
    ui->tBtnLoadDataFromInput->setChecked(m_load_data);

    if (m_load_data) {
        DataInput input;
        input.exec();
        auto values = input.getValues();
        if (!values.isEmpty()) {
            m_model->removeRows(0, m_model->rowCount());
        }
        for (int i = 0; i < values.size(); ++i) {
            QList<QStandardItem *> rowItems;
            rowItems << new QStandardItem(QString::number(i * ui->doubleSpinBoxStep->value()
                                                          + ui->doubleSpinBoxOffset->value()));
            rowItems << new QStandardItem(QString::number(values.at(i)));
            m_model->appendRow(rowItems);
        }
        emit useLoadedThreshold(true, values);
    } else {
        emit useLoadedThreshold(false, {});
    }
}

void ShowCorrectionCurve::on_doubleSpinBoxOffset_valueChanged(double offset)
{
    updateIndex();
}

void ShowCorrectionCurve::on_doubleSpinBoxStep_valueChanged(double step)
{
    updateIndex();
}

void ShowCorrectionCurve::updateIndex()
{
    int rows = m_model->rowCount();
    for (int i = 0; i < rows; ++i) {
        double idx = i * ui->doubleSpinBoxStep->value() + ui->doubleSpinBoxOffset->value();

        QStandardItem *item = m_model->item(i, 0);
        if (item) {
            item->setText(QString::number(idx));
        } else {
            m_model->setItem(i, 0, new QStandardItem(QString::number(idx)));
        }
    }
}

void ShowCorrectionCurve::on_tBtnLoadDataFromCSV_clicked()
{
    m_load_data = !m_load_data;
    ui->tBtnLoadDataFromCSV->setChecked(m_load_data);

    if (!m_load_data) {
        emit useLoadedThreshold(false, {});
        return;
    }

    // 选择 CSV 文件
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("选择数据 CSV 文件"),
                                                    "",
                                                    tr("CSV Files (*.csv)"));

    if (filePath.isEmpty()) {
        ui->tBtnLoadDataFromCSV->setChecked(false);
        m_load_data = false;
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
        return;
    }

    QTextStream in(&file);

    QList<double> values;
    m_model->removeRows(0, m_model->rowCount());

    bool firstLine = true; // 跳过表头

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        // 跳过表头行
        if (firstLine) {
            firstLine = false;
            continue;
        }

        QStringList parts = line.split(",");
        if (parts.size() < 2)
            continue;

        double indexVal = parts[0].toDouble();
        double thresholdVal = parts[1].toDouble();

        // 添加到模型
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(QString::number(indexVal));
        rowItems << new QStandardItem(QString::number(thresholdVal));
        m_model->appendRow(rowItems);

        values.append(thresholdVal);
    }

    file.close();

    emit useLoadedThreshold(true, values);
}

void ShowCorrectionCurve::on_btnApplyOption_clicked()
{
    double step = ui->doubleSpinBoxStep->value();
    double offset = ui->doubleSpinBoxOffset->value();
    emit useLoadedThreadsholdOption(offset, step);
}
