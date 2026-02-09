#include "formfittingpoints.h"
#include "ui_formfittingpoints.h"

#include <QFileDialog>
#include "funcdef.h"

FormFittingPoints::FormFittingPoints(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormFittingPoints)
{
    ui->setupUi(this);
    init();
}

FormFittingPoints::~FormFittingPoints()
{
    delete ui;
}

void FormFittingPoints::init()
{
    QString dir = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_DIR, "");
    ui->lineEditDir->setText(dir);
    int data_count
        = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_DATA_COUNT, "10").toInt();
    ui->spinBoxDataCount->setValue(data_count);
    int wave_start
        = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_WAVE_START, "900").toInt();
    ui->spinBoxWavelengthStart->setValue(wave_start);
    int wave_end = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_WAVE_END, "1700")
                       .toInt();
    ui->spinBoxWavelengthEnd->setValue(wave_end);
    double step = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_WAVE_STEP, "1")
                      .toDouble();
    ui->doubleSpinBoxStep->setValue(step);
    // QTableView
    m_collectModel = new QStandardItemModel(this);

    m_collectModel->setColumnCount(4);

    m_collectModel->setHorizontalHeaderLabels({"波长", "文件名", "保存路径", "状态"});

    ui->tableViewCollectStatus->setModel(m_collectModel);

    // 自适应列宽
    ui->tableViewCollectStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableViewCollectStatus->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableViewCollectStatus->setEditTriggers(QAbstractItemView::NoEditTriggers);
    refreshCollectTable();
}

void FormFittingPoints::refreshCollectTable()
{
    m_collectModel->removeRows(0, m_collectModel->rowCount());

    int wave_start = ui->spinBoxWavelengthStart->value();
    int wave_end = ui->spinBoxWavelengthEnd->value();
    double step = ui->doubleSpinBoxStep->value();

    QString dir = ui->lineEditDir->text();

    QDir saveDir(dir);

    int row = 0;

    for (double w = wave_start; w <= wave_end; w += step) {
        QString waveStr = QString::number(w, 'f', 1); // 900.0

        // 文件名：900.txt / 901.txt ...
        QString fileName = waveStr + ".txt";

        QString fullPath = saveDir.filePath(fileName);

        bool exist = QFileInfo::exists(fullPath);

        // ========= 创建 Item =========
        QStandardItem *itemWave = new QStandardItem(waveStr);

        QStandardItem *itemFile = new QStandardItem(fileName);

        QStandardItem *itemPath = new QStandardItem(fullPath);

        QStandardItem *itemStatus = new QStandardItem();

        // ========= 状态 & 颜色 =========
        if (exist) {
            itemStatus->setText("已采集");

            QBrush green(Qt::green);

            itemWave->setBackground(green);
            itemFile->setBackground(green);
            itemPath->setBackground(green);
            itemStatus->setBackground(green);

        } else {
            itemStatus->setText("未采集");

            QBrush red(QColor(255, 180, 180));

            itemWave->setBackground(red);
            itemFile->setBackground(red);
            itemPath->setBackground(red);
            itemStatus->setBackground(red);
        }

        // ========= 加入模型 =========
        m_collectModel->setItem(row, 0, itemWave);
        m_collectModel->setItem(row, 1, itemFile);
        m_collectModel->setItem(row, 2, itemPath);
        m_collectModel->setItem(row, 3, itemStatus);

        row++;
    }
}

void FormFittingPoints::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormFittingPoints::updateCollectionStatus(bool status)
{
    ui->btnCollect->setEnabled(true);
    refreshCollectTable();
    int rows = m_collectModel->rowCount();

    for (int i = 0; i < rows; ++i) {
        QStandardItem *statusItem = m_collectModel->item(i, 3); // 第3列：状态

        if (!statusItem)
            continue;

        if (statusItem->text() == "未采集") {
            // 第1列：文件名
            QStandardItem *fileItem = m_collectModel->item(i, 1);

            if (!fileItem)
                continue;

            QString fileName = fileItem->text();

            // 设置到输入框
            ui->lineEditWavelength->setText(fileName);
            ui->tableViewCollectStatus->selectRow(i);
            ui->tableViewCollectStatus->scrollTo(m_collectModel->index(i, 0));

            return; // 找到第一个就结束
        }
    }
}

void FormFittingPoints::on_tBtnSelectDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("choose dir"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    ui->lineEditDir->setText(dir);
    SETTING_CONFIG_SET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_DIR, dir);
}

void FormFittingPoints::on_btnCollect_clicked()
{
    ui->btnCollect->setEnabled(false);
    QString dir = ui->lineEditDir->text();
    QString file = ui->lineEditWavelength->text();
    int count = ui->spinBoxDataCount->value();
    emit toCollectionFittingPoints(dir, file, count);
}

void FormFittingPoints::on_tBtnRefresh_clicked()
{
    refreshCollectTable();
}

void FormFittingPoints::on_tableViewCollectStatus_clicked(const QModelIndex &index)
{
    int row = index.row();

    // 第 1 列：文件名
    QStandardItem *fileItem = m_collectModel->item(row, 1);
    if (!fileItem)
        return;

    ui->lineEditWavelength->setText(fileItem->text());
}
