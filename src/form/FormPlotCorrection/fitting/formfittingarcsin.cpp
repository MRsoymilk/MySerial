#include "formfittingarcsin.h"
#include "CalculateKB/calculatekb.h"
#include "funcdef.h"
#include "ui_formfittingarcsin.h"

#include <QFileDialog>
#include <QJsonArray>
#include <QMenu>
#include "datadef.h"
#include "httpclient.h"

FormFittingArcSin::FormFittingArcSin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormFittingArcSin)
{
    ui->setupUi(this);
    init();
}

FormFittingArcSin::~FormFittingArcSin()
{
    delete ui;
}

void FormFittingArcSin::updateParams()
{
    on_btnUpdateFormula_clicked();
}

QJsonObject FormFittingArcSin::getParams()
{
    QJsonObject params;
    params.insert("formula", "arcsin");
    params.insert("offset", 900.0);
    params.insert("step", 1.5);
    params.insert("k1", m_formula_y.k1);
    params.insert("b1", m_formula_y.b1);
    params.insert("l_k", m_formula_lambda_l.k);
    params.insert("l_b", m_formula_lambda_l.b);
    params.insert("l_d", m_formula_lambda_l.d);
    params.insert("l_alpha", m_formula_lambda_l.alpha);
    params.insert("r_k", m_formula_lambda_r.k);
    params.insert("r_b", m_formula_lambda_r.b);
    params.insert("r_d", m_formula_lambda_r.d);
    params.insert("r_alpha", m_formula_lambda_r.alpha);
    params.insert("k2", m_formula_y.k2);
    params.insert("b2", m_formula_y.b2);
    return params;
}

void FormFittingArcSin::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormFittingArcSin::init()
{
    m_modelThreshold = new QStandardItemModel(this);
    m_modelThreshold->setColumnCount(2);
    m_modelThreshold->setHeaderData(0, Qt::Horizontal, tr("index"));
    m_modelThreshold->setHeaderData(1, Qt::Horizontal, tr("fitting curve Raw(14bit)"));
    ui->tableViewFittingCurveData->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    ui->tableViewFittingCurveData->setModel(m_modelThreshold);
    ui->tableViewFittingCurveData->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableViewFittingCurveData,
            &QWidget::customContextMenuRequested,
            this,
            &FormFittingArcSin::showContextMenu);

    m_modelPoints = new QStandardItemModel(this);
    m_modelPoints->setColumnCount(2);
    m_modelPoints->setHeaderData(0, Qt::Horizontal, tr("lambda"));
    m_modelPoints->setHeaderData(1, Qt::Horizontal, tr("14bit(raw)"));
    ui->tableViewPoints->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewPoints->setModel(m_modelPoints);

    ui->spinBoxNum->setValue(535);
    ui->doubleSpinBoxStart->setValue(900);
    ui->doubleSpinBoxStep->setValue(1.5);

    m_urlFitArcSin = SETTING_CONFIG_GET(CFG_GROUP_SETTING,
                                        CFG_SETTING_FIT_ARCSIN_URL,
                                        URL_FITTING_ARCSIN);
    ui->labelFormula_y_lambda->setTextFormat(Qt::RichText);
    ui->labelFormula_y_lambda->setText(
        "y_lambda = arcsin(&lambda; / (2 &middot; d &middot; cos(&alpha; / 2))) - &alpha;/2");
    ui->labelFormula_y->setTextFormat(Qt::RichText);
    ui->labelFormula_y->setText("y = k &middot; y<sub>&lambda;</sub> + b");
}
void FormFittingArcSin::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *exportAllAction = new QAction(tr("Export Threshold to CSV"), this);
    connect(exportAllAction, &QAction::triggered, this, &FormFittingArcSin::exportThresholdToCSV);
    contextMenu.addAction(exportAllAction);

    contextMenu.exec(ui->tableViewFittingCurveData->viewport()->mapToGlobal(pos));
}

void FormFittingArcSin::exportThresholdToCSV()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Export Threshold Data to CSV"),
                                                "data_threshold.csv",
                                                tr("CSV Files (*.csv)"));
    if (path.isEmpty()) {
        LOG_WARN("CSV path is empty!");
        return;
    }
    if (!path.endsWith(".csv", Qt::CaseInsensitive)) {
        path.append(".csv");
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_WARN("Could not open file {} for writing!", path);
        return;
    }

    QTextStream out(&file);

    // --- 写入表头 ---
    QStringList headers;
    for (int col = 0; col < m_modelThreshold->columnCount(); ++col) {
        headers << m_modelThreshold->headerData(col, Qt::Horizontal).toString();
    }
    out << headers.join(",") << "\n";

    // --- 写入每行数据 ---
    for (int row = 0; row < m_modelThreshold->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < m_modelThreshold->columnCount(); ++col) {
            QModelIndex idx = m_modelThreshold->index(row, col);
            rowData << m_modelThreshold->data(idx).toString();
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    LOG_INFO("Exported all data to CSV: {}", path.toStdString());
}

void FormFittingArcSin::on_btnSendR_clicked()
{
    double k = ui->doubleSpinBoxR_k->value();
    double b = ui->doubleSpinBoxR_b->value();

    QByteArray byteArray;
    const QByteArray header = QByteArray::fromHex("DD3C000746");
    const QByteArray tail = QByteArray::fromHex("CDFF");
    int32_t s_k = static_cast<int32_t>(std::round(k * 1000.0));
    int32_t s_b = static_cast<int32_t>(std::round(b * 10.0));
    byteArray.append(header);
    byteArray.append((s_k >> 8) & 0xFF);
    byteArray.append((s_k >> 0) & 0xFF);
    byteArray.append((s_b >> 8) & 0xFF);
    byteArray.append((s_b >> 0) & 0xFF);
    byteArray.append(tail);
    LOG_INFO("send R = kx +b: {}", FORMAT_HEX(byteArray));
    LOG_INFO("k: {}", k);
    LOG_INFO("b: {}", b);
    emit sendSin(byteArray);
}

void FormFittingArcSin::on_btnCalculate_k1b1k2b2_clicked()
{
    CalculateKB cal;
    cal.exec();
    QJsonObject result = cal.getResult();

    QJsonObject objSlope = result["slope_fit"].toObject();
    QJsonObject objIntercept = result["intercept_fit"].toObject();
    {
        double k = objSlope["k"].toDouble();
        double b = objSlope["b"].toDouble();
        double mse = objSlope["mse"].toDouble();
        double mae = objSlope["mae"].toDouble();
        double r2 = objSlope["r2"].toDouble();
        ui->lineEdit_k1->setText(QString::number(k));
        ui->lineEdit_b1->setText(QString::number(b));
        ui->textEditLog->append(QString("slope: y = %1 * T + %2").arg(k).arg(b));
        ui->textEditLog->append(QString("loss: mse: %1, mae: %2, r2: %3").arg(mse).arg(mae).arg(r2));

        m_formula_y.k1 = k;
        m_formula_y.b1 = b;
    }
    {
        double k = objIntercept["k"].toDouble();
        double b = objIntercept["b"].toDouble();
        double mse = objIntercept["mse"].toDouble();
        double mae = objIntercept["mae"].toDouble();
        double r2 = objIntercept["r2"].toDouble();
        ui->lineEdit_k2->setText(QString::number(k));
        ui->lineEdit_b2->setText(QString::number(b));
        ui->textEditLog->append(QString("intercept: y = %1 * T + %2").arg(k).arg(b));
        ui->textEditLog->append(QString("loss: mse: %1, mae: %2, r2: %3").arg(mse).arg(mae).arg(r2));

        m_formula_y.k2 = k;
        m_formula_y.b2 = b;
    }
}

void FormFittingArcSin::on_btnFittingArcSin_clicked()
{
    if (!m_modelPoints)
        return;

    QJsonArray lambdaArray;
    QJsonArray yArray;

    for (int row = 0; row < m_modelPoints->rowCount(); ++row) {
        QString lambda = m_modelPoints->item(row, 0) ? m_modelPoints->item(row, 0)->text() : "";
        QString raw14 = m_modelPoints->item(row, 1) ? m_modelPoints->item(row, 1)->text() : "";

        lambdaArray.append(lambda.toDouble()); // λ
        yArray.append(raw14.toInt());          // y (raw14)
    }

    QJsonObject objRequest;
    objRequest["lambda_x"] = lambdaArray;
    objRequest["y"] = yArray;
    objRequest["k"] = (m_formula_y.k1 * m_formula_y.T + m_formula_y.b1) / 8.5 / 1000.0;
    objRequest["b"] = (m_formula_y.k2 * m_formula_y.T + m_formula_y.b2) / 1000.0;
    // objRequest["d0"] = 1.667;
    // objRequest["alpha0"] = 30;

    QJsonDocument doc(objRequest);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    HttpClient *client = new HttpClient;
    connect(client, &HttpClient::success, this, [&](const QJsonDocument &resp) {
        QJsonObject result = resp.object();
        ui->textEditLog->append(QString("[%1]:").arg(TIMESTAMP_0()));
        QString jsonStr = resp.toJson(QJsonDocument::Indented);
        ui->textEditLog->append(jsonStr);
        {
            QJsonObject left = result["params_left"].toObject();
            double k = left["k"].toDouble();
            double b = left["b"].toDouble();
            double d = left["d"].toDouble();
            double alpha = left["alpha"].toDouble();
            m_formula_lambda_l.k = k;
            m_formula_lambda_l.b = b;
            m_formula_lambda_l.d = d;
            m_formula_lambda_l.alpha = alpha;
        }
        {
            QJsonObject right = result["params_right"].toObject();
            double k = right["k"].toDouble();
            double b = right["b"].toDouble();
            double d = right["d"].toDouble();
            double alpha = right["alpha"].toDouble();
            m_formula_lambda_r.k = k;
            m_formula_lambda_r.b = b;
            m_formula_lambda_r.d = d;
            m_formula_lambda_r.alpha = alpha;
        }
        updateFormulaLambda();
    });

    client->post(m_urlFitArcSin, objRequest);
}

void FormFittingArcSin::on_btnSendFormula_clicked()
{
    QByteArray byteArray;
    const QByteArray header = QByteArray::fromHex("DD3C006351");
    const QByteArray tail = QByteArray::fromHex("CDFF");

    auto appendScaled = [&](QByteArray &arr, double value) {
        int64_t scaled = static_cast<int64_t>(std::round(value * 1000.0));
        // 转大端序
        for (int i = 7; i >= 0; --i) {
            arr.append(static_cast<char>((scaled >> (i * 8)) & 0xFF));
        }
    };

    byteArray.append(header);
    appendScaled(byteArray, m_formula_y.k1);
    appendScaled(byteArray, m_formula_y.b1);
    appendScaled(byteArray, m_formula_lambda_l.k);
    appendScaled(byteArray, m_formula_lambda_l.b);
    appendScaled(byteArray, m_formula_lambda_l.d);
    appendScaled(byteArray, m_formula_lambda_l.alpha);
    appendScaled(byteArray, m_formula_lambda_r.k);
    appendScaled(byteArray, m_formula_lambda_r.b);
    appendScaled(byteArray, m_formula_lambda_r.d);
    appendScaled(byteArray, m_formula_lambda_r.alpha);
    appendScaled(byteArray, m_formula_y.k2);
    appendScaled(byteArray, m_formula_y.b2);
    byteArray.append(tail);

    LOG_INFO("send formula: {}", FORMAT_HEX(byteArray));
    LOG_INFO("k1 {}", m_formula_y.k1);
    LOG_INFO("b1 {}", m_formula_y.b1);
    LOG_INFO("left k {}", m_formula_lambda_l.k);
    LOG_INFO("left b {}", m_formula_lambda_l.b);
    LOG_INFO("left d {}", m_formula_lambda_l.d);
    LOG_INFO("left alpha {}", m_formula_lambda_l.alpha);
    LOG_INFO("right k {}", m_formula_lambda_r.k);
    LOG_INFO("right b {}", m_formula_lambda_r.b);
    LOG_INFO("right d {}", m_formula_lambda_r.d);
    LOG_INFO("right alpha {}", m_formula_lambda_r.alpha);
    LOG_INFO("k2 {}", m_formula_y.k2);
    LOG_INFO("b2 {}", m_formula_y.b2);
    emit sendSin(byteArray);
}

void FormFittingArcSin::on_btnSetTemperatureT_clicked()
{
    m_formula_y.T = ui->doubleSpinBoxTemperatureT->value();
    ui->lineEdit_T->setText(QString::number(m_formula_y.T));
    ui->lineEdit_T_->setText(QString::number(m_formula_y.T));
}

void FormFittingArcSin::updateFormulaLambda()
{
    if (ui->radioButtonLeft->isChecked()) {
        ui->lineEdit_k_lambda->setText(QString::number(m_formula_lambda_l.k));
        ui->lineEdit_b_lambda->setText(QString::number(m_formula_lambda_l.b));
        ui->lineEdit_d->setText(QString::number(m_formula_lambda_l.d));
        ui->lineEdit_alpha->setText(QString::number(m_formula_lambda_l.alpha));
        ui->lineEdit_alpha_->setText(QString::number(m_formula_lambda_l.alpha));
    } else {
        ui->lineEdit_k_lambda->setText(QString::number(m_formula_lambda_r.k));
        ui->lineEdit_b_lambda->setText(QString::number(m_formula_lambda_r.b));
        ui->lineEdit_d->setText(QString::number(m_formula_lambda_r.d));
        ui->lineEdit_alpha->setText(QString::number(m_formula_lambda_r.alpha));
        ui->lineEdit_alpha_->setText(QString::number(m_formula_lambda_r.alpha));
    }
}
void FormFittingArcSin::on_btnUpdateFormula_clicked()
{
    QString params = ui->lineEditFormula->text();
    if (!params.isEmpty()) {
        QByteArray byteArray = QByteArray::fromHex(params.toUtf8());

        // 校验长度：header(5) + 8字节 * 12个参数 + tail(2)
        if (byteArray.size() < 5 + 8 * 12 + 2) {
            qWarning() << "参数长度不正确";
            return;
        }

        int idx = 5; // 跳过 header "DD3C002351"

        auto readDouble = [&](const QByteArray &arr, int &pos) -> double {
            if (pos + 8 > arr.size())
                return 0.0;
            int64_t val = 0;
            for (int i = 0; i < 8; ++i) {
                val = (val << 8) | static_cast<uint8_t>(arr[pos + i]);
            }
            pos += 8;
            return static_cast<double>(val) / 1000.0;
        };

        m_formula_y.k1 = readDouble(byteArray, idx);
        m_formula_y.b1 = readDouble(byteArray, idx);
        m_formula_lambda_l.k = readDouble(byteArray, idx);
        m_formula_lambda_l.b = readDouble(byteArray, idx);
        m_formula_lambda_l.d = readDouble(byteArray, idx);
        m_formula_lambda_l.alpha = readDouble(byteArray, idx);
        m_formula_lambda_r.k = readDouble(byteArray, idx);
        m_formula_lambda_r.b = readDouble(byteArray, idx);
        m_formula_lambda_r.d = readDouble(byteArray, idx);
        m_formula_lambda_r.alpha = readDouble(byteArray, idx);
        m_formula_y.k2 = readDouble(byteArray, idx);
        m_formula_y.b2 = readDouble(byteArray, idx);

        LOG_INFO("generate params from {} to\n"
                 "k1: {}, b1: {},\n"
                 "l_k: {}, l_b: {}, l_d: {}, l_alpha: {},\n"
                 "r_k: {}, r_b: {}, r_d: {}, r_alpha: {},\n"
                 "k2: {}, b2: {}",
                 params,
                 m_formula_y.k1,
                 m_formula_y.b1,
                 m_formula_lambda_l.k,
                 m_formula_lambda_l.b,
                 m_formula_lambda_l.d,
                 m_formula_lambda_l.alpha,
                 m_formula_lambda_r.k,
                 m_formula_lambda_r.b,
                 m_formula_lambda_r.d,
                 m_formula_lambda_r.alpha,
                 m_formula_y.k2,
                 m_formula_y.b2);

        ui->lineEdit_k1->setText(QString::number(m_formula_y.k1));
        ui->lineEdit_b1->setText(QString::number(m_formula_y.b1));
        updateFormulaLambda();
        ui->lineEdit_k2->setText(QString::number(m_formula_y.k2));
        ui->lineEdit_b2->setText(QString::number(m_formula_y.b2));

    } else {
        if (ui->radioButtonLeft->isChecked()) {
            m_formula_lambda_l.k = ui->lineEdit_k_lambda->text().toDouble();
            m_formula_lambda_l.b = ui->lineEdit_b_lambda->text().toDouble();
            m_formula_lambda_l.d = ui->lineEdit_d->text().toDouble();
            m_formula_lambda_l.alpha = ui->lineEdit_alpha->text().toDouble();
        } else {
            m_formula_lambda_r.k = ui->lineEdit_k_lambda->text().toDouble();
            m_formula_lambda_r.b = ui->lineEdit_b_lambda->text().toDouble();
            m_formula_lambda_r.d = ui->lineEdit_d->text().toDouble();
            m_formula_lambda_r.alpha = ui->lineEdit_alpha->text().toDouble();
        }

        m_formula_y.k1 = ui->lineEdit_k1->text().toDouble();
        m_formula_y.k2 = ui->lineEdit_k2->text().toDouble();
        m_formula_y.b1 = ui->lineEdit_b1->text().toDouble();
        m_formula_y.b2 = ui->lineEdit_b2->text().toDouble();

        m_formula_y.T = ui->lineEdit_T->text().toDouble();
    }
}

double FormFittingArcSin::calculate(const double &idx)
{
    double y_lambda = 0.0;
    if (idx <= 1310) {
        y_lambda = m_formula_lambda_l.k
                       * (qAsin(idx / 1000.0
                                / (2 * m_formula_lambda_l.d
                                   * qCos(M_PI / 180.0 * m_formula_lambda_l.alpha / 2)))
                          - m_formula_lambda_l.alpha / 2)
                   + m_formula_lambda_l.b;
    } else {
        y_lambda = m_formula_lambda_r.k
                       * (qAsin(idx / 1000.0
                                / (2 * m_formula_lambda_r.d
                                   * qCos(M_PI / 180.0 * m_formula_lambda_r.alpha / 2)))
                          - m_formula_lambda_r.alpha / 2)
                   + m_formula_lambda_r.b;
    }

    double y = (m_formula_y.k1 * m_formula_y.T + m_formula_y.b1) / 8.5 / 1000 * y_lambda
               + (m_formula_y.k2 * m_formula_y.T + m_formula_y.b2) / 1000;
    return y;
}

void FormFittingArcSin::on_btnGenerateThreshold_clicked()
{
    double start = ui->doubleSpinBoxStart->value();
    double step = ui->doubleSpinBoxStep->value();
    int num = ui->spinBoxNum->value();
    for (int i = 0; i < num; ++i) {
        double x = i * step + start;
        double y = calculate(x);
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(QString::number(x, 'f', 2));
        rowItems << new QStandardItem(QString::number(std::round(y)));
        m_modelThreshold->appendRow(rowItems);
    }
    ui->tabWidget->setCurrentWidget(ui->tabFittingCurveData);
}

void FormFittingArcSin::on_tBtnAdd_clicked()
{
    if (!m_modelPoints)
        return;

    int row = m_modelPoints->rowCount();
    m_modelPoints->insertRow(row);

    m_modelPoints->setItem(row, 0, new QStandardItem("0.0")); // lambda
    m_modelPoints->setItem(row, 1, new QStandardItem("0"));   // 14bit(raw)

    QModelIndex index = m_modelPoints->index(row, 0);
    ui->tableViewPoints->setCurrentIndex(index);
    ui->tableViewPoints->edit(index);
}

void FormFittingArcSin::on_tBtnDelete_clicked()
{
    if (!m_modelPoints)
        return;

    // 获取当前选中的行
    QModelIndex index = ui->tableViewPoints->currentIndex();
    if (!index.isValid())
        return;

    int row = index.row();
    m_modelPoints->removeRow(row);
}

void FormFittingArcSin::on_radioButtonRight_clicked()
{
    updateFormulaLambda();
}

void FormFittingArcSin::on_radioButtonLeft_clicked()
{
    updateFormulaLambda();
}
