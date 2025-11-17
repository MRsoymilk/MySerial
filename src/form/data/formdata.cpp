#include "formdata.h"
#include <QDateTime>
#include <QFileDialog>
#include <QMenu>
#include <QStandardItem>
#include <QTextStream>
#include "funcdef.h"
#include "ui_formdata.h"

FormData::FormData(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormData)
{
    ui->setupUi(this);
    init();
}

FormData::~FormData()
{
    if (m_model) {
        delete m_model;
    }
    delete ui;
}

void FormData::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormData::init()
{
    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(3);
    m_model->setHeaderData(0, Qt::Horizontal, "timestamp");
    m_model->setHeaderData(1, Qt::Horizontal, "data");
    m_model->setHeaderData(2, Qt::Horizontal, "size");

    ui->table->setModel(m_model);
    ui->table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->table->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->table->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->table, &QWidget::customContextMenuRequested, this, &FormData::showContextMenu);
    getINI();
}

void FormData::onDataReceivedF15(const QByteArray &data31,
                                 const QByteArray &data33,
                                 const QByteArray &temperature)
{
    QList<QStandardItem *> rowItems;
    auto data = data33 + data31 + temperature;
    rowItems << new QStandardItem(TIMESTAMP());
    QString to_show;
    for (int i = 0; i < data.length(); ++i) {
        to_show.append(QString("%1 ").arg((unsigned char) data[i], 2, 16, QChar('0')).toUpper());
    }
    rowItems << new QStandardItem(to_show);
    rowItems << new QStandardItem(QString::number(data.length()));

    m_model->appendRow(rowItems);

    while (m_model->rowCount() > m_limit) {
        m_model->removeRow(0);
    }
}

void FormData::onDataReceivedF30(const QByteArray &data14,
                                 const QByteArray &data24,
                                 const QByteArray &temperature)
{
    QList<QStandardItem *> rowItems;
    auto data = data14 + data24 + temperature;
    rowItems << new QStandardItem(TIMESTAMP());
    QString to_show;
    for (int i = 0; i < data.length(); ++i) {
        to_show.append(QString("%1 ").arg((unsigned char) data[i], 2, 16, QChar('0')).toUpper());
    }
    rowItems << new QStandardItem(to_show);
    rowItems << new QStandardItem(QString::number(data.length()));

    m_model->appendRow(rowItems);

    while (m_model->rowCount() > m_limit) {
        m_model->removeRow(0);
    }
}

void FormData::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *clearAction = new QAction(tr("Clear"), this);
    connect(clearAction, &QAction::triggered, this, &FormData::clearData);
    contextMenu.addAction(clearAction);

    QAction *exportAction = new QAction(tr("Export Selected to CSV"), this);
    connect(exportAction, &QAction::triggered, this, &FormData::exportToCSV);
    contextMenu.addAction(exportAction);

    QAction *exportAllAction = new QAction(tr("Export All to CSV"), this);
    connect(exportAllAction, &QAction::triggered, this, &FormData::exportAllToCSV);
    contextMenu.addAction(exportAllAction);

    contextMenu.exec(ui->table->viewport()->mapToGlobal(pos));
}

void FormData::getINI()
{
    m_limit = SETTING_CONFIG_GET(CFG_GROUP_DATA, CFG_DATA_LIMIT, "10").toInt();
    ui->lineEditDataLimit->setText(QString::number(m_limit));
}

void FormData::setINI()
{
    SETTING_CONFIG_SET(CFG_GROUP_DATA, CFG_DATA_LIMIT, QString::number(m_limit));
}

void FormData::clearData()
{
    if (m_model->rowCount() > 0) {
        m_model->removeRows(0, m_model->rowCount());
    }
}

void FormData::exportAllToCSV()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save All to CSV"),
                                                "data_all.csv",
                                                tr("CSV Files (*.csv)"));
    if (path.isEmpty()) {
        LOG_WARN("csv path is empty!", path);
        return;
    }
    if (!path.endsWith(".csv", Qt::CaseInsensitive)) {
        path.append(".csv");
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_WARN("Could not open file {} for writing!", path);
        return;
    }

    QTextStream stream(&file);
    stream << "timestamp,data,size\n";
    exportAll(stream);

    file.close();
    LOG_INFO("Data exported to {}", path);
}

void FormData::exportSeleced(const QModelIndexList &selectedRows, QTextStream &stream)
{
    foreach (const QModelIndex &index, selectedRows) {
        int row = index.row();

        QString timestamp = m_model->item(row, 0)->text();
        QString data = m_model->item(row, 1)->text();
        QString size = m_model->item(row, 2)->text();

        stream << timestamp << "," << data << "," << size << "\n";
    }
}

void FormData::exportAll(QTextStream &stream)
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QString timestamp = m_model->item(row, 0)->text();
        QString data = m_model->item(row, 1)->text();
        QString size = m_model->item(row, 2)->text();

        stream << timestamp << "," << data << "," << size << "\n";
    }
}

void FormData::exportToCSV()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save CSV"),
                                                "data.csv",
                                                tr("CSV Files (*.csv)"));
    if (path.isEmpty()) {
        LOG_WARN("csv path is empty!", path);
        return;
    }
    if (!path.endsWith(".csv", Qt::CaseInsensitive)) {
        path.append(".csv");
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_WARN("Could not open file {} for writing!", path);
        return;
    }

    QTextStream stream(&file);
    stream << "timestamp,data,size\n";

    QModelIndexList selectedRows = ui->table->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        LOG_INFO("save all data!");
        exportAll(stream);
    } else {
        LOG_INFO("save selected data!");
        exportSeleced(selectedRows, stream);
    }

    file.close();
    LOG_INFO("Data exported to {}", path);
}

void FormData::on_lineEditDataLimit_editingFinished()
{
    m_limit = ui->lineEditDataLimit->text().toInt();
    setINI();
}
