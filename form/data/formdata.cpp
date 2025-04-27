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
    model = new QStandardItemModel(this);
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, "timestamp");
    model->setHeaderData(1, Qt::Horizontal, "data");
    model->setHeaderData(2, Qt::Horizontal, "size");

    ui->table->setModel(model);
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

    model->appendRow(rowItems);
}

void FormData::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *exportAction = new QAction(tr("Export to CSV"), this);
    connect(exportAction, &QAction::triggered, this, &FormData::exportToCSV);
    contextMenu.addAction(exportAction);

    contextMenu.exec(ui->table->viewport()->mapToGlobal(pos));
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
        for (int row = 0; row < model->rowCount(); ++row) {
            QString timestamp = model->item(row, 0)->text();
            QString data = model->item(row, 1)->text();
            QString size = model->item(row, 2)->text();

            stream << timestamp << "," << data << "," << size << "\n";
        }
    } else {
        LOG_INFO("save selected data!");
        foreach (const QModelIndex &index, selectedRows) {
            int row = index.row();

            QString timestamp = model->item(row, 0)->text();
            QString data = model->item(row, 1)->text();
            QString size = model->item(row, 2)->text();

            stream << timestamp << "," << data << "," << size << "\n";
        }
    }

    file.close();
    LOG_INFO("Data exported to {}", path);
}
