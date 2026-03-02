#include "showdata.h"
#include "ui_showdata.h"

ShowData::ShowData(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowData)
{
    ui->setupUi(this);
    init();
}

ShowData::~ShowData()
{
    delete ui;
}

void ShowData::showData(const MY_DATA &data)
{
    m_model->removeRows(0, m_model->rowCount());

    const auto &raw31  = data.curve31.raw.data;
    const auto &volt31 = data.curve31.data;

    const auto &raw33  = data.curve33.raw.data;
    const auto &volt33 = data.curve33.data;

    int size = qMax(qMax(raw31.size(), volt31.size()),
                    qMax(raw33.size(), volt33.size()));

    m_model->setRowCount(size);

    for (int i = 0; i < size; ++i)
    {
        m_model->setItem(i, 0, new QStandardItem(QString::number(i)));

        // ===== 31 =====
        if (i < raw31.size())
            m_model->setItem(i, 1,
                             new QStandardItem(QString::number(raw31[i].y(), 'f', 6)));

        if (i < volt31.size())
            m_model->setItem(i, 2,
                             new QStandardItem(QString::number(volt31[i].y(), 'f', 6)));

        // ===== 33 =====
        if (i < raw33.size())
            m_model->setItem(i, 3,
                             new QStandardItem(QString::number(raw33[i].y(), 'f', 6)));

        if (i < volt33.size())
            m_model->setItem(i, 4,
                             new QStandardItem(QString::number(volt33[i].y(), 'f', 6)));
    }
}

void ShowData::init() {
    m_model = new QStandardItemModel(this);

    m_model->setColumnCount(5);
    m_model->setHorizontalHeaderLabels({
        "Index",
        "31_Y_Raw", "31_Y_Voltage",
        "33_Y_Raw", "33_Y_Voltage"
    });
    ui->tableView->setModel(m_model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
