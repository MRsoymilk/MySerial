#include "formdata.h"
#include <QDateTime>
#include <QStandardItem>
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
    model->setColumnCount(2);
    model->setHeaderData(1, Qt::Horizontal, "timestamp");
    model->setHeaderData(2, Qt::Horizontal, "data");

    ui->table->setModel(model);
    ui->table->horizontalHeader()->setStretchLastSection(true);
}

void FormData::onDataReceived(const QByteArray &data)
{
    QList<QStandardItem *> rowItems;

    rowItems << new QStandardItem(TIMESTAMP());
    rowItems << new QStandardItem(QString::fromUtf8(data));

    model->appendRow(rowItems);
}
