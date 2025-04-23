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
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, "timestamp");
    model->setHeaderData(1, Qt::Horizontal, "data");
    model->setHeaderData(2, Qt::Horizontal, "size");

    ui->table->setModel(model);
    ui->table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
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
