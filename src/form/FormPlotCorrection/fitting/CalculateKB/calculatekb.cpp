#include "calculatekb.h"
#include "ui_calculatekb.h"

#include <QChart>
#include <QChartView>
#include <QJsonArray>
#include <QLineSeries>
#include <QMenu>
#include <QValueAxis>
#include "datadef.h"
#include "funcdef.h"
#include "httpclient.h"

QString m_urlFitKB;

CalculateKB::CalculateKB(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CalculateKB)
{
    ui->setupUi(this);
    init();
}

CalculateKB::~CalculateKB()
{
    delete ui;
}

QJsonObject CalculateKB::getResult()
{
    return m_result;
}

void CalculateKB::init()
{
    m_model = new QStandardItemModel(0, 3, this);
    m_model->setHeaderData(0, Qt::Horizontal, tr("temperature (℃)"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("slope (mV)"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("intercept (mV)"));
    ui->tableView->setModel(m_model);
    connect(ui->tableView,
            &QWidget::customContextMenuRequested,
            this,
            &CalculateKB::showContextMenu);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_urlFitKB = SETTING_CONFIG_GET(CFG_GROUP_SETTING, CFG_SETTING_FIT_KB_URL, URL_FITTING_KB);
}

void CalculateKB::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *displayAction = new QAction(tr("Display"), this);
    connect(displayAction, &QAction::triggered, this, &CalculateKB::displayData);
    contextMenu.addAction(displayAction);

    QAction *deleteAction = new QAction(tr("Delete"), this);
    connect(deleteAction, &QAction::triggered, this, &CalculateKB::deleteData);
    contextMenu.addAction(deleteAction);

    contextMenu.exec(ui->tableView->viewport()->mapToGlobal(pos));
}
void CalculateKB::displayData()
{
    // 1. 提取数据
    QVector<double> temperature, slopes, intercepts;
    int rowCount = m_model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        bool ok1, ok2, ok3;
        double t = m_model->item(row, 0)->text().toDouble(&ok1);
        double s = m_model->item(row, 1)->text().toDouble(&ok2);
        double i = m_model->item(row, 2)->text().toDouble(&ok3);
        if (ok1 && ok2 && ok3) {
            temperature.append(t);
            slopes.append(s);
            intercepts.append(i);
        }
    }

    // 2. 创建曲线
    QLineSeries *seriesSlope = new QLineSeries();
    seriesSlope->setName(tr("Slope (mV)"));
    QLineSeries *seriesIntercept = new QLineSeries();
    seriesIntercept->setName(tr("Intercept (mV)"));

    for (int i = 0; i < temperature.size(); ++i) {
        seriesSlope->append(temperature[i], slopes[i]);
        seriesIntercept->append(temperature[i], intercepts[i]);
    }

    // 3. 创建图表
    QChart *chart = new QChart();
    chart->addSeries(seriesSlope);
    chart->addSeries(seriesIntercept);
    chart->setTitle(tr("Temperature vs Slope & Intercept"));

    // === 双 Y 轴设置 ===
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText(tr("Temperature (℃)"));
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisYLeft = new QValueAxis;
    axisYLeft->setTitleText(tr("Slope (mV)"));
    chart->addAxis(axisYLeft, Qt::AlignLeft);

    QValueAxis *axisYRight = new QValueAxis;
    axisYRight->setTitleText(tr("Intercept (mV)"));
    chart->addAxis(axisYRight, Qt::AlignRight);

    seriesSlope->attachAxis(axisX);
    seriesSlope->attachAxis(axisYLeft);
    seriesIntercept->attachAxis(axisX);
    seriesIntercept->attachAxis(axisYRight);

    // 4. 创建 QChartView
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 5. 清空旧图表
    QLayoutItem *item;
    while ((item = ui->gLayPlot->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    // 6. 添加到布局
    ui->gLayPlot->addWidget(chartView, 0, 0);
}

void CalculateKB::deleteData()
{
    QItemSelectionModel *selectionModel = ui->tableView->selectionModel();
    if (!selectionModel->hasSelection())
        return;

    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (!selectedRows.isEmpty()) {
        int row = selectedRows.first().row();
        m_model->removeRow(row);
    }
}

void CalculateKB::on_btnCalculate_clicked()
{
    HttpClient *client = new HttpClient;
    connect(client, &HttpClient::success, this, [&](const QJsonDocument &resp) {
        m_result = resp.object();
        ui->textEditFitOutput->append(QString("[%1]:").arg(TIMESTAMP_0()));
        QString jsonStr = resp.toJson(QJsonDocument::Indented);
        ui->textEditFitOutput->append(jsonStr);
    });
    QJsonObject objRequest;
    QJsonArray arrSlopes;
    QJsonArray arrIntercepts;
    QJsonArray arrTemperature;
    int rowCount = m_model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        bool ok1, ok2, ok3;
        double temp = m_model->item(row, 0)->text().toDouble(&ok1);
        double slope = m_model->item(row, 1)->text().toDouble(&ok2);
        double intercept = m_model->item(row, 2)->text().toDouble(&ok3);

        if (ok1) {
            arrTemperature.append(temp);
        }
        if (ok2) {
            arrSlopes.append(slope);
        }
        if (ok3) {
            arrIntercepts.append(intercept);
        }
    }

    objRequest.insert("temperature", arrTemperature);
    objRequest.insert("slopes", arrSlopes);
    objRequest.insert("intercepts", arrIntercepts);

    client->post(m_urlFitKB, objRequest);
}

void CalculateKB::on_textEditInput_textChanged()
{
    QString text = ui->textEditInput->toPlainText();

    if (text.isEmpty()) {
        return;
    }

    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    QVector<double> slopes;      // 存放斜率
    QVector<double> intercepts;  // 存放截距
    QVector<double> temperature; // 存放温度

    // 第一行是表头，从第二行开始解析
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i].simplified(); // 去掉多余空格
        QStringList cols = line.split(' ', Qt::SkipEmptyParts);
        if (cols.size() >= 5) {
            bool ok1 = false, ok2 = false, ok3 = false;
            double temp = cols[1].toDouble(&ok1);      // 第2列
            double slope = cols[3].toDouble(&ok2);     // 第4列
            double intercept = cols[4].toDouble(&ok3); // 第5列
            if (ok1 && ok2 && ok3) {
                temperature.append(temp);
                slopes.append(slope);
                intercepts.append(intercept);
            }
        }
    }

    // 打印测试
    for (int i = 0; i < slopes.size(); ++i) {
        qDebug() << "temperature = " << temperature[i] << ", slope =" << slopes[i]
                 << ", intercept =" << intercepts[i];
    }

    for (int row = 0; row < temperature.size(); ++row) {
        m_model->setItem(row, 0, new QStandardItem(QString::number(temperature[row])));
        m_model->setItem(row, 1, new QStandardItem(QString::number(slopes[row])));
        m_model->setItem(row, 2, new QStandardItem(QString::number(intercepts[row])));
    }
    ui->tableView->resizeColumnsToContents();

    ui->tabWidget->setCurrentWidget(ui->tabProcessing);
}
