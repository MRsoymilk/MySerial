#include "showcorrectioncurve.h"
#include "DataInput/datainput.h"
#include "funcdef.h"
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

void ShowCorrectionCurve::updateThresholdStatus(const QString &status)
{
    ui->labelWhereThreshold->setText(status);
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
    if (m_enableRangeY) {
        m_axisY->setRange(ui->spinBoxStartY->value(), ui->spinBoxEndY->value());
    } else {
        m_axisY->setRange(yMin, yMax);
    }
    m_data.push_back(data);
    m_current_page = m_data.size() - 1;
    ui->labelPage->setText(QString("%1 / %2").arg(m_current_page + 1).arg(m_data.size()));
    if (m_enableExternal) {
        callToExternal(data);
    }
}

void ShowCorrectionCurve::callToExternal(const QList<QPointF> &data)
{
    QJsonArray spectrumArray;

    for (const QPointF &pt : data) {
        QJsonObject obj;

        QString wavelengthKey = QString::number(pt.x(), 'f', 6);
        obj.insert(wavelengthKey, pt.y());

        spectrumArray.append(obj);
    }

    QJsonObject info;
    info.insert("spectrum", spectrumArray);
    info.insert("timestamp", TIMESTAMP());
    emit toExternalSpectral(info);
}

void ShowCorrectionCurve::closeEvent(QCloseEvent *event)
{
    m_data.clear();
    QList<QList<QPointF>>().swap(m_data);
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

    int offset = SETTING_CONFIG_GET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_OFFSET, "900").toInt();
    double step = SETTING_CONFIG_GET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_STEP, "1").toDouble();
    int count = SETTING_CONFIG_GET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_COUNT, "800").toInt();
    ui->doubleSpinBoxOffset->setValue(offset);
    ui->doubleSpinBoxStep->setValue(step);
    ui->spinBoxCount->setValue(count);

    m_load_data = false;

    ui->tBtnRangeY->setCheckable(true);
    int start = SETTING_CONFIG_GET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_Y_MIN, "0").toInt();
    int end = SETTING_CONFIG_GET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_Y_MAX, "9999").toInt();
    ui->spinBoxStartY->setValue(start);
    ui->spinBoxEndY->setValue(end);

    QShortcut *shortcut_prev = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcut_prev, &QShortcut::activated, this, &ShowCorrectionCurve::on_tBtnPrev_clicked);
    QShortcut *shortcut_next = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(shortcut_next, &QShortcut::activated, this, &ShowCorrectionCurve::on_tBtnNext_clicked);

    ui->tBtnExternal->setCheckable(true);
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
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error"), tr("can't open file %1").arg(filePath));
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

    on_btnApplyOption_clicked();
    emit useLoadedThreshold(true, values);
    ui->tabWidget->setCurrentWidget(ui->tabCurve);
}

void ShowCorrectionCurve::on_btnApplyOption_clicked()
{
    double step = ui->doubleSpinBoxStep->value();
    double offset = ui->doubleSpinBoxOffset->value();
    int count = ui->spinBoxCount->value();
    QJsonObject option;
    option["step"] = step;
    option["offset"] = offset;
    option["count"] = count;
    emit useLoadedThreadsholdOption(option);
    SETTING_CONFIG_SET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_OFFSET, QString::number(offset));
    SETTING_CONFIG_SET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_STEP, QString::number(step));
    SETTING_CONFIG_SET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_COUNT, QString::number(count));
}

void ShowCorrectionCurve::on_tBtnRangeY_clicked()
{
    if (ui->tBtnRangeY->isChecked()) {
        m_enableRangeY = true;
        m_axisY->setRange(ui->spinBoxStartY->value(), ui->spinBoxEndY->value());
        SETTING_CONFIG_SET(CFG_GROUP_CORRECTION,
                           CFG_CORRECTION_CURVE_Y_MIN,
                           QString::number(ui->spinBoxStartY->value()));
        SETTING_CONFIG_SET(CFG_GROUP_CORRECTION,
                           CFG_CORRECTION_CURVE_Y_MAX,
                           QString::number(ui->spinBoxEndY->value()));
    } else {
        m_enableRangeY = false;
    }
}

void ShowCorrectionCurve::on_spinBoxStartY_valueChanged(int val)
{
    if (ui->tBtnRangeY->isChecked()) {
        m_axisY->setRange(val, m_axisY->max());
        SETTING_CONFIG_SET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_Y_MIN, QString::number(val));
    }
}

void ShowCorrectionCurve::on_spinBoxEndY_valueChanged(int val)
{
    if (ui->tBtnRangeY->isChecked()) {
        m_axisY->setRange(m_axisY->min(), val);
        SETTING_CONFIG_SET(CFG_GROUP_CORRECTION, CFG_CORRECTION_CURVE_Y_MAX, QString::number(val));
    }
}

void ShowCorrectionCurve::on_tBtnExportCurve_clicked()
{
    if (m_data.isEmpty()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error"), tr("no curve data to export."));
        return;
    }

    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Export Curve to CSV"),
                                                QDir::homePath() + "/curves_export.csv",
                                                "CSV Files (*.csv)");

    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error"), tr("Failed to open export file"));
        return;
    }

    QTextStream out(&file);

    out << "index";
    for (int i = 0; i < m_data.size(); ++i) {
        out << ",x_" << (i + 1) << ",y_" << (i + 1);
    }
    out << "\n";

    int maxLen = 0;
    for (const auto &curve : m_data)
        maxLen = std::max(maxLen, static_cast<int>(curve.size()));

    for (int row = 0; row < maxLen; ++row) {
        out << row;

        for (const auto &curve : m_data) {
            if (row < curve.size()) {
                out << "," << curve[row].x() << "," << curve[row].y();
            } else {
                out << ",,";
            }
        }
        out << "\n";
    }

    file.close();
}

void ShowCorrectionCurve::on_tBtnClear_clicked()
{
    m_data.clear();
    m_current_page = 0;
    ui->labelPage->setText(QString("%1 / %2").arg(m_current_page).arg(m_data.size()));
}

void ShowCorrectionCurve::on_tBtnExportRaw_clicked()
{
    if (m_data.isEmpty()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error"), tr("No raw data to export."));
        return;
    }

    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Export raw to txt"),
                                                QDir::homePath() + "/curves_export.txt",
                                                "TXT Files (*.txt)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error"), tr("Failed to open export file."));
        return;
    }

    QTextStream out(&file);

    const QByteArray header = QByteArray::fromHex("DE3A064331");
    const QByteArray tail = QByteArray::fromHex("CEFF");

    for (const auto &curve : m_data) {
        QByteArray payload;

        for (int i = 0; i < curve.size(); ++i) {
            quint16 raw = static_cast<quint16>(std::round(curve[i].y()));
            payload.append(raw >> 8 & 0xFF);
            payload.append(raw & 0xFF);
        }

        QByteArray packet;
        packet.append(header);
        packet.append(payload);
        packet.append(tail);

        for (int i = 0; i < packet.size(); ++i) {
            out << QString("%1 ").arg(static_cast<quint8>(packet[i]), 2, 16, QChar('0')).toUpper();
        }
        out << "\n";
    }

    file.close();
    SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error"), tr("Export finished: %1").arg(path));
}

void ShowCorrectionCurve::on_tBtnExternal_clicked()
{
    m_enableExternal = !m_enableExternal;
    ui->tBtnExternal->setChecked(m_enableExternal);
}

void ShowCorrectionCurve::on_tBtnInterpolation_clicked()
{
    m_enableInterpolation = !m_enableInterpolation;
    emit useLoadedThreadsholdOption({{"interpolation", m_enableInterpolation}});
}
