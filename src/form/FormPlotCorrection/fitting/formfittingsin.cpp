#include "formfittingsin.h"
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QPair>
#include <QtMath>
#include <optional>

#include <QMouseEvent>
#include <QtMath>
#include "CalculateKB/calculatekb.h"
#include "ImageViewer/imageviewer.h"
#include "datadef.h"
#include "funcdef.h"
#include "httpclient.h"
#include "ui_formfittingsin.h"

ImageViewer *imageViewer = nullptr;
QString m_urlCalculate;
QString m_urlFindPeak;
FormFittingSin::FormFittingSin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormFittingSin)
{
    ui->setupUi(this);
    init();

    ui->labelPlotSin->installEventFilter(this);
    ui->labelPlotPeak->installEventFilter(this);
}

FormFittingSin::~FormFittingSin()
{
    delete ui;
}

void FormFittingSin::init()
{
    ui->labelFormula->setText("y = (k<sub>1</sub> &middot; T + b<sub>1</sub>) / 8.5 * "
                              "[y<sub>0</sub> + A &middot; sin(&pi; (x - "
                              "x<sub>c</sub>) / w)] + k<sub>2</sub> &middot; T + b<sub>2</sub>");
    ui->labelFormula->setTextFormat(Qt::RichText);
    ui->labelFormula->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
    ui->labelPlotSin->setToolTip(tr("Double click to zoom."));
    ui->labelPlotSin->setScaledContents(true);
    ui->labelPlotPeak->setToolTip(tr("Double click to zoom."));
    ui->labelPlotPeak->setScaledContents(true);

    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(3);
    m_model->setHeaderData(0, Qt::Horizontal, "index");
    m_model->setHeaderData(1, Qt::Horizontal, "fitting curve V(14bit)");
    m_model->setHeaderData(2, Qt::Horizontal, "fitting curve Raw(14bit)");
    ui->tableViewFittingCurveData->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    ui->tableViewFittingCurveData->setModel(m_model);
    ui->tableViewFittingCurveData->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableViewFittingCurveData,
            &QWidget::customContextMenuRequested,
            this,
            &FormFittingSin::showContextMenu);

    m_urlCalculate = SETTING_CONFIG_GET(CFG_GROUP_SETTING, CFG_SETTING_FIT_SIN_URL, URL_FITTING_SIN);
    m_urlFindPeak = SETTING_CONFIG_GET(CFG_GROUP_SETTING, CFG_SETTING_FIND_PEAK_URL, URL_FIND_PEAK);
    m_sin_fixed = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    ui->doubleSpinBoxStep->setValue(1.5);
    ui->spinBoxNum->setValue(535);
}

void FormFittingSin::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *exportAllAction = new QAction(tr("Export All to CSV"), this);
    connect(exportAllAction, &QAction::triggered, this, &FormFittingSin::exportAllToCSV);
    contextMenu.addAction(exportAllAction);

    contextMenu.exec(ui->tableViewFittingCurveData->viewport()->mapToGlobal(pos));
}

void FormFittingSin::exportAllToCSV()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Export All Data to CSV"),
                                                "data_all.csv",
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
    for (int col = 0; col < m_model->columnCount(); ++col) {
        headers << m_model->headerData(col, Qt::Horizontal).toString();
    }
    out << headers.join(",") << "\n";

    // --- 写入每行数据 ---
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < m_model->columnCount(); ++col) {
            QModelIndex idx = m_model->index(row, col);
            rowData << m_model->data(idx).toString();
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    LOG_INFO("Exported all data to CSV: {}", path.toStdString());
}

void FormFittingSin::packageRawData()
{
    QByteArray byteArray;

    // 包头
    const QByteArray header = QByteArray::fromHex("DD3C043542");
    byteArray.append(header);

    for (const qint32 val : m_threshold_table) {
        // 转成2字节（short），注意大小端顺序，这里假设低字节在前（小端）
        quint16 raw16 = static_cast<quint16>(val & 0xFFFF);

        char lowByte = raw16 & 0xFF;
        char highByte = (raw16 >> 8) & 0xFF;

        byteArray.append(lowByte);
        byteArray.append(highByte);
    }

    // 包尾
    const QByteArray tail = QByteArray::fromHex("CDFF");
    byteArray.append(tail);
    // send file
    QFile file("threshold.hex");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing"));
        return;
    }

    QByteArray hexData = byteArray.toHex().toUpper();

    qint64 bytesWritten = file.write(hexData);
    file.close();

    if (bytesWritten != hexData.size()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to write complete data"));
    } else {
        QMessageBox::information(this, tr("Success"), tr("File saved successfully"));
    }
    emit sendSin(byteArray);
}

QByteArray FormFittingSin::packageRawData(const QVector<QPointF> &points)
{
    QByteArray byteArray;

    // 包头
    const QByteArray header = QByteArray::fromHex("DD3C043542");
    byteArray.append(header);

    for (const QPointF &pt : points) {
        qint32 raw = qRound(pt.y() / 3.3 * (1 << 13));
        // 转成2字节（short），注意大小端顺序，这里假设低字节在前（小端）
        quint16 raw16 = static_cast<quint16>(raw & 0xFFFF);

        char lowByte = raw16 & 0xFF;
        char highByte = (raw16 >> 8) & 0xFF;

        byteArray.append(lowByte);
        byteArray.append(highByte);
    }

    // 包尾
    const QByteArray tail = QByteArray::fromHex("CDFF");
    byteArray.append(tail);

    return byteArray;
}

void FormFittingSin::fillFixedFittingCurveData(const double &start)
{
    m_threshold_table.clear();
    m_model->removeRows(0, m_model->rowCount());
    int length = ui->spinBoxNum->value();
    double step = ui->doubleSpinBoxStep->value();
    for (int i = 0; i < length; ++i) {
        double x = i * step + start;
        double y = m_k
                       * (m_sin_fixed.y0
                          + m_sin_fixed.A * qSin(M_PI * (x - m_sin_fixed.xc) / m_sin_fixed.w))
                   + m_b;
        qint32 signedRaw = qRound(y / 3.3 * (1 << 13));
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(QString::number(x, 'f', 2));
        rowItems << new QStandardItem(QString::number(y, 'f', 6));
        rowItems << new QStandardItem(QString::number(signedRaw));
        m_threshold_table.push_back(signedRaw);
        m_model->appendRow(rowItems);
    }
}

void FormFittingSin::fillFittingCurveData()
{
    m_model->removeRows(0, m_model->rowCount());

    int startX = qRound(m_sin.xc);
    int periodLength = qRound(2 * m_sin.w);
    int endX = startX + periodLength;

    double minAbsY = std::numeric_limits<double>::max();
    int xClosest = startX;
    double yClosest = 0;

    for (double x = startX; x < endX; x += 1) {
        double y = m_sin.y0 + m_sin.A * qSin(M_PI * (x - m_sin.xc) / m_sin.w);

        // 更新最小值
        if (std::abs(y) < minAbsY) {
            minAbsY = std::abs(y);
            xClosest = x;
            yClosest = y;
        }

        qint32 signedRaw = qRound(y / 3.3 * (1 << 13));

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(QString::number(x, 'f', 2));
        rowItems << new QStandardItem(QString::number(y, 'f', 6));
        rowItems << new QStandardItem(QString::number(signedRaw));

        m_model->appendRow(rowItems);
    }

    ui->textBrowserSinLog->append(
        QString("Closest point to x-axis: x = %1, y = %2").arg(xClosest).arg(yClosest));
}

bool FormFittingSin::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->labelPlotSin && event->type() == QEvent::MouseButtonDblClick) {
        if (!m_pixSin.isNull()) {
            if (!imageViewer) {
                imageViewer = new ImageViewer(this);
                imageViewer->setAttribute(Qt::WA_DeleteOnClose);
                imageViewer = nullptr;
            }
            ImageViewer *viewer = new ImageViewer(this);
            viewer->setAttribute(Qt::WA_DeleteOnClose);
            viewer->setImage(m_pixSin);
            viewer->show();
        }
        return true;
    }
    if (obj == ui->labelPlotPeak && event->type() == QEvent::MouseButtonDblClick) {
        if (!m_pixPeak.isNull()) {
            if (!imageViewer) {
                imageViewer = new ImageViewer(this);
                imageViewer->setAttribute(Qt::WA_DeleteOnClose);
                imageViewer = nullptr;
            }
            ImageViewer *viewer = new ImageViewer(this);
            viewer->setAttribute(Qt::WA_DeleteOnClose);
            viewer->setImage(m_pixPeak);
            viewer->show();
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void FormFittingSin::doCorrection(const QVector<double> &v14, const QVector<double> &v24)
{
    ui->textBrowserSinLog->append("===== start correction =====");

    if (v14.empty()) {
        ui->textBrowserSinLog->append("v14 is Empty!");
        return;
    }

    m_v14 = v14;

    double k = (m_sin.k1 * m_sin.T + m_sin.b1) / 8.5 / 1000.0;
    double b = (m_sin.k2 * m_sin.T + m_sin.b2) / 1000.0;

    m_k = k;
    m_b = b;

    QJsonArray x_arr;
    QJsonArray y_arr;
    for (int i = 0; i < v14.size(); ++i) {
        x_arr.append(i);
        y_arr.append(v14[i]);
    }

    QJsonObject objFitSin;
    objFitSin[KEY_X] = x_arr;
    objFitSin[KEY_Y] = y_arr;
    objFitSin["k"] = k;
    objFitSin["b"] = b;

    QUrl urlFitSin(m_urlCalculate);
    HttpClient *clientFitSin = new HttpClient(this);
    connect(clientFitSin, &HttpClient::success, this, [=](const QJsonDocument &resp) {
        QJsonObject result = resp.object();

        m_sin.A = result[KEY_A].toDouble();
        m_sin.w = result[KEY_w].toDouble();
        m_sin.xc = result[KEY_xc].toDouble();
        m_sin.y0 = result[KEY_y0].toDouble();
        double loss_rss = result["loss_rss"].toDouble();
        double loss_mse = result["loss_mse"].toDouble();
        double r2 = result["r2"].toDouble();
        ui->textBrowserSinLog->append(
            QString("Loss: rss: %1, mse: %2, r2: %3").arg(loss_rss).arg(loss_mse).arg(r2));
        QString imageUrl = result[KEY_IMG_URL].toString();

        ui->lineEdit_A->setText(QString::number(m_sin.A));
        ui->lineEdit_w->setText(QString::number(m_sin.w));
        ui->lineEdit_xc->setText(QString::number(m_sin.xc));
        ui->lineEdit_y0->setText(QString::number(m_sin.y0));
        QString msg = QString("response: %1")
                          .arg(QString(QJsonDocument(resp).toJson(QJsonDocument::Compact)));
        ui->textBrowserSinLog->append(msg);
        LOG_INFO(msg);
        fillFittingCurveData();
        clientFitSin->getImage(QUrl(imageUrl));
    });

    connect(clientFitSin, &HttpClient::imageLoaded, this, [=](const QPixmap &pixmap) {
        QString msg = QString("Fitting Sin recv img size: (%1, %2)")
                          .arg(pixmap.size().width())
                          .arg(pixmap.size().height());
        ui->textBrowserSinLog->append(msg);
        LOG_INFO(msg);

        if (pixmap.isNull()) {
            QString msg = "pic is empty!";
            ui->textBrowserSinLog->append(msg);
            LOG_WARN(msg);
            return;
        }
        m_pixSin = pixmap;
        ui->labelPlotSin->setPixmap(m_pixSin);
    });

    connect(clientFitSin, &HttpClient::imageFailed, this, [=](const QString &err) {
        QString msg = "pic load failed: " + err;
        ui->textBrowserSinLog->append(msg);
        LOG_WARN(msg);
    });

    connect(clientFitSin, &HttpClient::failure, this, [&](const QString &err) {
        QString msg = "fitting failed: " + err;
        ui->textBrowserSinLog->append(msg);
        LOG_WARN(msg);
    });

    clientFitSin->post(urlFitSin, objFitSin);

    QUrl urlFindPeak(m_urlFindPeak);
    HttpClient *clientFindPeak = new HttpClient(this);
    connect(clientFindPeak, &HttpClient::success, this, [=](const QJsonDocument &resp) {
        QJsonObject result = resp.object();
        QJsonArray arr = result["peaks"].toArray();
        for (auto x : arr) {
            auto obj = x.toObject();
            qDebug() << obj;
        }
        for (int i = 0; i < arr.size(); ++i) {
            auto obj = arr[i].toObject();
            double x = obj["x"].toDouble();
            double y = obj["y"].toDouble();
            if (i == 0) {
                ui->doubleSpinBoxX1->setValue(x);
            }
            if (i == 1) {
                ui->doubleSpinBoxX2->setValue(x);
            }
            ui->textBrowserSinLog->append(QString("peak: (%1, %2)").arg(x).arg(y));
        }

        QString url = result["image_url"].toString();
        clientFindPeak->getImage(QUrl(url));
    });
    connect(clientFindPeak, &HttpClient::imageLoaded, this, [=](const QPixmap &pixmap) {
        QString msg = QString("Find Peak recv img size: (%1, %2)")
                          .arg(pixmap.size().width())
                          .arg(pixmap.size().height());
        ui->textBrowserSinLog->append(msg);
        LOG_INFO(msg);

        if (pixmap.isNull()) {
            QString msg = "pic is empty!";
            ui->textBrowserSinLog->append(msg);
            LOG_WARN(msg);
            return;
        }
        m_pixPeak = pixmap;
        ui->labelPlotPeak->setPixmap(m_pixPeak);
    });
    connect(clientFindPeak, &HttpClient::imageFailed, this, [=](const QString &err) {
        QString msg = "pic load failed: " + err;
        ui->textBrowserSinLog->append(msg);
        LOG_WARN(msg);
    });
    connect(clientFindPeak, &HttpClient::failure, this, [&](const QString &err) {
        QString msg = "fitting failed: " + err;
        ui->textBrowserSinLog->append(msg);
        LOG_WARN(msg);
    });
    QJsonObject objFindPeak;
    QJsonArray peak_x_arr;
    QJsonArray peak_y_arr;
    for (int i = 0; i < v24.size(); ++i) {
        peak_x_arr.append(i);
        peak_y_arr.append(v24[i]);
    }
    objFindPeak[KEY_X] = peak_x_arr;
    objFindPeak[KEY_Y] = peak_y_arr;
    clientFindPeak->post(urlFindPeak, objFindPeak);
}

std::optional<QPair<double, double>> FormFittingSin::solveSinParams_hard(
    double x1, double y1, double x2, double y2, double A, double y0)
{
    y1 = (y1 - m_b) / m_k;
    y2 = (y2 - m_b) / m_k;

    double t = qAsin((y2 - y0) / A) / qAsin((y1 - y0) / A);
    double xc = (x2 - t * x1) / (1 - t);
    double w = (x1 - xc) * M_PI / qAsin((y1 - y0) / A);
    return QPair<double, double>(xc, w);
}

void FormFittingSin::on_btnAdjust_clicked()
{
    // 获取界面输入的两点
    double lambda1 = ui->spinBoxX1Real->value();
    double lambda2 = ui->spinBoxX2Real->value();
    double point1 = ui->doubleSpinBoxX1->value();
    double point2 = ui->doubleSpinBoxX2->value();
    double y1 = m_v14[point1];
    double y2 = m_v14[point2];

    // 获取拟合曲线当前的 A 和 y0
    double A = m_sin.A;
    double y0 = m_sin.y0;
    auto res = solveSinParams_hard(lambda1, y1, lambda2, y2, A, y0);

    auto xc = res->first;
    auto w = res->second;
    QString msg = QString("adjust finish: y = %5 * [%1 * sin(pi * (x - %2) / %3) + %4] + %6")
                      .arg(A)
                      .arg(xc)
                      .arg(w)
                      .arg(y0)
                      .arg(m_k)
                      .arg(m_b);
    m_sin_fixed.A = A;
    m_sin_fixed.w = w;
    m_sin_fixed.xc = xc;
    m_sin_fixed.y0 = y0;
    ui->textBrowserSinLog->append(msg);
    fillFixedFittingCurveData(lambda1);
    packageRawData();
}

void FormFittingSin::on_btnUpdate_clicked()
{
    m_sin.A = ui->lineEdit_A->text().toDouble();
    m_sin.w = ui->lineEdit_w->text().toDouble();
    m_sin.xc = ui->lineEdit_xc->text().toDouble();
    m_sin.y0 = ui->lineEdit_y0->text().toDouble();

    m_sin.k1 = ui->lineEdit_k1->text().toDouble();
    m_sin.k2 = ui->lineEdit_k2->text().toDouble();
    m_sin.b1 = ui->lineEdit_b1->text().toDouble();
    m_sin.b2 = ui->lineEdit_b2->text().toDouble();

    m_sin.T = ui->lineEdit_T->text().toDouble();
}

void FormFittingSin::on_btnGenerateThreshold_clicked()
{
    double start = ui->doubleSpinBoxStart->value();
    fillFixedFittingCurveData(start);
    packageRawData();
}

void FormFittingSin::on_btnCalculate_k1b1_k2b2_clicked()
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
        ui->textBrowserSinLog->append(QString("slope: y = %1 * T + %2").arg(k).arg(b));
        ui->textBrowserSinLog->append(
            QString("loss: mse: %1, mae: %2, r2: %3").arg(mse).arg(mae).arg(r2));
        m_sin.k1 = k;
        m_sin.b1 = b;
    }
    {
        double k = objIntercept["k"].toDouble();
        double b = objIntercept["b"].toDouble();
        double mse = objIntercept["mse"].toDouble();
        double mae = objIntercept["mae"].toDouble();
        double r2 = objIntercept["r2"].toDouble();
        ui->lineEdit_k2->setText(QString::number(k));
        ui->lineEdit_b2->setText(QString::number(b));
        ui->textBrowserSinLog->append(QString("intercept: y = %1 * T + %2").arg(k).arg(b));
        ui->textBrowserSinLog->append(
            QString("loss: mse: %1, mae: %2, r2: %3").arg(mse).arg(mae).arg(r2));
        m_sin.k2 = k;
        m_sin.b2 = b;
    }
}

void FormFittingSin::on_btnGetTemperature_clicked()
{
    QString val = QString::number(ui->doubleSpinBoxTemperature->value());
    ui->lineEdit_T->setText(val);
    ui->lineEdit_T_->setText(val);
}
