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
    delete ui;
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
}

void FormData::onDataReceived(const QByteArray &data)
{
    QList<QStandardItem *> rowItems;

    rowItems << new QStandardItem(TIMESTAMP());
    QString to_show;
    for (int i = 0; i < data.length(); ++i) {
        to_show.append(QString("%1 ").arg((unsigned char) data[i], 2, 16, QChar('0')).toUpper());
    }
    rowItems << new QStandardItem(to_show);
    rowItems << new QStandardItem(QString::number(data.length()));

    m_model->appendRow(rowItems);
}

void FormData::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *clearAction = new QAction(tr("Clear"), this);
    connect(clearAction, &QAction::triggered, this, &FormData::clearData);
    contextMenu.addAction(clearAction);

    QAction *exportAction = new QAction(tr("Export to CSV"), this);
    connect(exportAction, &QAction::triggered, this, &FormData::exportToCSV);
    contextMenu.addAction(exportAction);

    contextMenu.exec(ui->table->viewport()->mapToGlobal(pos));
}

void FormData::clearData()
{
    m_model->clear();
}

void FormData::exportToCSV()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save CSV"), "", tr("CSV Files (*.csv)"));
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
        for (int row = 0; row < m_model->rowCount(); ++row) {
            QString timestamp = m_model->item(row, 0)->text();
            QString data = m_model->item(row, 1)->text();
            QString size = m_model->item(row, 2)->text();

            stream << timestamp << "," << data << "," << size << "\n";
        }
    } else {
        LOG_INFO("save selected data!");
        foreach (const QModelIndex &index, selectedRows) {
            int row = index.row();

            QString timestamp = m_model->item(row, 0)->text();
            QString data = m_model->item(row, 1)->text();
            QString size = m_model->item(row, 2)->text();

            stream << timestamp << "," << data << "," << size << "\n";
        }
    }

    file.close();
    LOG_INFO("Data exported to {}", path);
}
