#include "formplotdata.h"
#include "ui_formplotdata.h"

#include <QFileDialog>
#include <QMenu>
#include "funcdef.h"

FormPlotData::FormPlotData(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlotData)
{
    ui->setupUi(this);
    init();
}

FormPlotData::~FormPlotData()
{
    if (m_model) {
        delete m_model;
    }
    delete ui;
}

void FormPlotData::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormPlotData::displayData(const QVector<double> &v14,
                               const QVector<double> &v24,
                               const QVector<qint32> &raw14,
                               const QVector<qint32> &raw24)
{
    clearData();
    int count = qMax(qMax(v14.size(), v24.size()), qMax(raw14.size(), raw24.size()));
    for (int i = 0; i < count; ++i) {
        QString index = QString::number(i);
        QString yV14 = (i < v14.size()) ? QString::number(v14[i]) : "";
        QString yV24 = (i < v24.size()) ? QString::number(v24[i]) : "";
        QString yR14 = (i < raw14.size()) ? QString::number(raw14[i]) : "";
        QString yR24 = (i < raw24.size()) ? QString::number(raw24[i]) : "";

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(index);
        rowItems << new QStandardItem(yV14);
        rowItems << new QStandardItem(yV24);
        rowItems << new QStandardItem(yR14);
        rowItems << new QStandardItem(yR24);
        m_model->appendRow(rowItems);
    }
}

void FormPlotData::updateTable4k(const QVector<double> &v14,
                                 const QVector<double> &v24,
                                 const QVector<qint32> &raw14,
                                 const QVector<qint32> &raw24)
{
    if (this->isVisible()) {
        ++m_count;
        m_current = m_count;
        listV14.append(v14);
        listV24.append(v24);
        listRaw14.append(raw14);
        listRaw24.append(raw24);
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV14.back(), listV24.back(), listRaw14.back(), listRaw24.back());
    }
}

void FormPlotData::init()
{
    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(5);
    m_model->setHeaderData(0, Qt::Horizontal, "index");
    m_model->setHeaderData(1, Qt::Horizontal, "14bit(v)");
    m_model->setHeaderData(2, Qt::Horizontal, "24bit(v)");
    m_model->setHeaderData(3, Qt::Horizontal, "14bit(raw)");
    m_model->setHeaderData(4, Qt::Horizontal, "24bit(raw)");

    ui->table->setModel(m_model);

    ui->table->setContextMenuPolicy(Qt::CustomContextMenu);

    int columnCount = m_model->columnCount();
    for (int i = 0; i < columnCount; ++i) {
        ui->table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
    }
    connect(ui->table, &QWidget::customContextMenuRequested, this, &FormPlotData::showContextMenu);

    m_count = 0;
    ui->lineEditGo->setText("1");
}

void FormPlotData::closeEvent(QCloseEvent *event)
{
    m_current = m_count = 0;
    ui->labelStatus->setText("0/0");
    clearData();
    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotData::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *clearAction = new QAction(tr("Clear"), this);
    connect(clearAction, &QAction::triggered, this, &FormPlotData::clearData);
    contextMenu.addAction(clearAction);

    QAction *exportCurrentAction = new QAction(tr("Export Current to CSV"), this);
    connect(exportCurrentAction, &QAction::triggered, this, &FormPlotData::exportCurrentToCSV);
    contextMenu.addAction(exportCurrentAction);

    QAction *exportAllAction = new QAction(tr("Export All to CSV"), this);
    connect(exportAllAction, &QAction::triggered, this, &FormPlotData::exportAllToCSV);
    contextMenu.addAction(exportAllAction);

    contextMenu.exec(ui->table->viewport()->mapToGlobal(pos));
}

void FormPlotData::clearData()
{
    if (m_model->rowCount() > 0) {
        m_model->removeRows(0, m_model->rowCount());
    }
}

void FormPlotData::exportAllToCSV()
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

    QTextStream stream(&file);
    stream.setRealNumberPrecision(10);

    int groupCount = listV14.size();
    if (groupCount == 0) {
        LOG_WARN("No data to export!");
        return;
    }

    int rowCount = listV14[0].size();

    QStringList headers;
    headers << m_model->headerData(0, Qt::Horizontal).toString();
    QString nameV14 = m_model->headerData(1, Qt::Horizontal).toString();
    QString nameV24 = m_model->headerData(2, Qt::Horizontal).toString();
    QString nameRaw14 = m_model->headerData(3, Qt::Horizontal).toString();
    QString nameRaw24 = m_model->headerData(4, Qt::Horizontal).toString();
    for (int i = 0; i < groupCount; ++i) {
        headers << QString("%1_%2").arg(nameV14).arg(i + 1);
        headers << QString("%1_%2").arg(nameV24).arg(i + 1);
        headers << QString("%1_%2").arg(nameRaw14).arg(i + 1);
        headers << QString("%1_%2").arg(nameRaw24).arg(i + 1);
    }
    stream << headers.join(",") << "\n";

    for (int row = 0; row < rowCount; ++row) {
        QStringList rowData;
        rowData << QString::number(row);
        for (int g = 0; g < groupCount; ++g) {
            const auto &v14 = listV14[g];
            const auto &v24 = listV24[g];
            const auto &raw14 = listRaw14[g];
            const auto &raw24 = listRaw24[g];

            if (row < v14.size() && row < v24.size() && row < raw14.size() && row < raw24.size()) {
                rowData << QString::number(v14[row], 'f', 6);
                rowData << QString::number(v24[row], 'f', 6);
                rowData << QString::number(raw14[row]);
                rowData << QString::number(raw24[row]);
            } else {
                rowData << "" << "" << "" << "";
            }
        }
        stream << rowData.join(",") << "\n";
    }

    file.close();
    QString msg = QString("All group data exported to %1").arg(path);
    SHOW_AUTO_CLOSE_MSGBOX(this, "Exprot", msg);
    LOG_INFO(msg);
}

void FormPlotData::exportCurrentToCSV()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save CSV"),
                                                "data.csv",
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

    QTextStream stream(&file);

    int columnCount = m_model->columnCount();
    int rowCount = m_model->rowCount();

    QStringList headers;
    for (int col = 0; col < columnCount; ++col) {
        QString header = m_model->headerData(col, Qt::Horizontal).toString();
        headers << header;
    }
    stream << headers.join(",") << "\n";

    LOG_INFO("Exporting all data...");
    for (int row = 0; row < rowCount; ++row) {
        QStringList rowData;
        for (int col = 0; col < columnCount; ++col) {
            rowData << m_model->item(row, col)->text();
        }
        stream << rowData.join(",") << "\n";
    }

    file.close();
    QString msg = QString("Data exported to %1").arg(path);
    SHOW_AUTO_CLOSE_MSGBOX(this, "Exprot", msg);
    LOG_INFO(msg);
}

void FormPlotData::on_tBtnPrev_clicked()
{
    if (m_current - 1 > 0) {
        --m_current;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV14[m_current - 1],
                    listV24[m_current - 1],
                    listRaw14[m_current - 1],
                    listRaw24[m_current - 1]);
    }
}

void FormPlotData::on_tBtnNext_clicked()
{
    if (m_current + 1 <= m_count) {
        ++m_current;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV14[m_current - 1],
                    listV24[m_current - 1],
                    listRaw14[m_current - 1],
                    listRaw24[m_current - 1]);
    }
}

void FormPlotData::on_lineEditGo_editingFinished()
{
    int target = ui->lineEditGo->text().toInt();
    if (target > 0 && target <= m_count) {
        m_current = target;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV14[m_current - 1],
                    listV24[m_current - 1],
                    listRaw14[m_current - 1],
                    listRaw24[m_current - 1]);
    }
}
