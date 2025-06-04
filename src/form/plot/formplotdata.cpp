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

void FormPlotData::updateTable4k(const QVector<double> &v14, const QVector<double> &v24)
{
    m_model->removeRows(0, m_model->rowCount());
    for (int i = 0; i < v14.size(); ++i) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(i);
        rowItems << new QStandardItem(v14[i]);
        m_model->appendRow(rowItems);
    }
    for (int i = 0; i < v24.size(); ++i) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(i);
        rowItems << new QStandardItem(v24[i]);
        m_model->appendRow(rowItems);
    }
}

void FormPlotData::init()
{
    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, "x");
    m_model->setHeaderData(1, Qt::Horizontal, "y");

    ui->table->setModel(m_model);

    ui->table->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->table->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->table->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    connect(ui->table, &QWidget::customContextMenuRequested, this, &FormPlotData::showContextMenu);
}

void FormPlotData::closeEvent(QCloseEvent *event)
{
    m_model->removeRows(0, m_model->rowCount());
    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotData::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context Menu"), this);

    QAction *clearAction = new QAction(tr("Clear"), this);
    connect(clearAction, &QAction::triggered, this, &FormPlotData::clearData);
    contextMenu.addAction(clearAction);

    QAction *exportAction = new QAction(tr("Export to CSV"), this);
    connect(exportAction, &QAction::triggered, this, &FormPlotData::exportToCSV);
    contextMenu.addAction(exportAction);

    contextMenu.exec(ui->table->viewport()->mapToGlobal(pos));
}

void FormPlotData::clearData()
{
    if (m_model->rowCount() > 0) {
        m_model->removeRows(0, m_model->rowCount());
    }
}

void FormPlotData::exportToCSV()
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
    stream << "x,y\n";

    QModelIndexList selectedRows = ui->table->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        LOG_INFO("save all data!");
        for (int row = 0; row < m_model->rowCount(); ++row) {
            QString x = m_model->item(row, 0)->text();
            QString y = m_model->item(row, 1)->text();
            stream << x << "," << y << "\n";
        }
    } else {
        LOG_INFO("save selected data!");
        foreach (const QModelIndex &index, selectedRows) {
            int row = index.row();
            QString x = m_model->item(row, 0)->text();
            QString y = m_model->item(row, 1)->text();
            stream << x << "," << y << "\n";
        }
    }

    file.close();
    LOG_INFO("Data exported to {}", path);
}
