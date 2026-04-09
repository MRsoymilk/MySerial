#include "formfittingpoints.h"

#include <qclipboard.h>
#include <qmenu.h>

#include <QFileDialog>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QThread>

#include "funcdef.h"
#include "ui_formfittingpoints.h"
#include "myprocess.h"

FormFittingPoints::FormFittingPoints(QWidget *parent) : QWidget(parent), ui(new Ui::FormFittingPoints) {
    ui->setupUi(this);
    init();
}

FormFittingPoints::~FormFittingPoints() { delete ui; }

void FormFittingPoints::init() {
    QString dir = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_DIR, "");
    ui->lineEditDir->setText(dir);
    int data_count = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_DATA_COUNT, "10").toInt();
    ui->spinBoxDataCount->setValue(data_count);
    int wave_start = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_WAVE_START, "900").toInt();
    ui->spinBoxWavelengthStart->setValue(wave_start);
    int wave_end = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_WAVE_END, "1700").toInt();
    ui->spinBoxWavelengthEnd->setValue(wave_end);
    double step = SETTING_CONFIG_GET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_WAVE_STEP, "1").toDouble();
    ui->doubleSpinBoxStep->setValue(step);
    // QTableView
    m_collectModel = new QStandardItemModel(this);
    m_collectModel->setColumnCount(5);
    m_collectModel->setHorizontalHeaderLabels(
        {tr("wavelength"), tr("file_name"), tr("file_path"), tr("status"), tr("intensity")});
    ui->tableViewCollectStatus->setModel(m_collectModel);

    // 自适应列宽
    ui->tableViewCollectStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewCollectStatus->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewCollectStatus->setEditTriggers(QAbstractItemView::NoEditTriggers);
    refreshCollectTable();

    ui->tableViewCollectStatus->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableViewCollectStatus, &QTableView::customContextMenuRequested, this,
            &FormFittingPoints::onTableContextMenu);
    connect(&MY_PROCESS, &MyProcess::outputReceived, this, [](const QString &output){
        LOG_INFO("progress success: {}", output);
    });
    connect(&MY_PROCESS, &MyProcess::errorReceived, this, [](const QString &output){
        LOG_CRITICAL("progress fail: {}", output);
    });
}

void FormFittingPoints::onTableContextMenu(const QPoint &pos) {
    QModelIndex index = ui->tableViewCollectStatus->indexAt(pos);

    if (!index.isValid()) return;

    QMenu menu(this);

    QAction *exportAction = menu.addAction("Export Wavelength and Intensity to CSV");

    QAction *ret = menu.exec(ui->tableViewCollectStatus->viewport()->mapToGlobal(pos));

    if (ret == exportAction) {
        exportWavelengthIntensityToCsv();
    }
}

void FormFittingPoints::exportWavelengthIntensityToCsv() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export CSV", "collection.csv", "CSV Files (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "Wavelength,Intensity\n";
    int rows = m_collectModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        QString col_wavelength = m_collectModel->item(i, WAVELENGTH) ? m_collectModel->item(i, WAVELENGTH)->text() : "";
        QString col_intensity = m_collectModel->item(i, INTENSITY) ? m_collectModel->item(i, INTENSITY)->text() : "";
        out << col_wavelength << "," << col_intensity << "\n";
    }

    file.close();

    QMessageBox::information(this, "Export", "CSV exported successfully.");
}

void FormFittingPoints::refreshCollectTable() {
    m_collectModel->removeRows(0, m_collectModel->rowCount());

    int wave_start = ui->spinBoxWavelengthStart->value();
    int wave_end = ui->spinBoxWavelengthEnd->value();
    double step = ui->doubleSpinBoxStep->value();

    QString dir = ui->lineEditDir->text();

    QDir saveDir(dir);

    int row = 0;
    int n = static_cast<int>((wave_end - wave_start) / step + 0.5);

    for (int i = 0; i <= n; ++i) {
        double w = wave_start + i * step;
        QString waveStr = QString::number(w, 'f', 1);  // 900.0

        // 文件名：900.txt / 901.txt ...
        QString fileName = waveStr + ".txt";

        QString fullPath = saveDir.filePath(fileName);

        bool exist = QFileInfo::exists(fullPath);

        QStandardItem *itemWave = new QStandardItem(waveStr);
        QStandardItem *itemFile = new QStandardItem(fileName);
        QStandardItem *itemPath = new QStandardItem(fullPath);
        QStandardItem *itemStatus = new QStandardItem();
        QStandardItem *itemIntensity = new QStandardItem();

        if (exist) {
            itemStatus->setText(item_status[COLLECTED]);

            QBrush green(QColor(200, 255, 200));

            itemWave->setBackground(green);
            itemFile->setBackground(green);
            itemPath->setBackground(green);
            itemStatus->setBackground(green);
            itemIntensity->setBackground(green);
        } else {
            itemStatus->setText(item_status[NOT_COLLECTED]);

            QBrush red(QColor(255, 180, 180));

            itemWave->setBackground(red);
            itemFile->setBackground(red);
            itemPath->setBackground(red);
            itemStatus->setBackground(red);
            itemIntensity->setBackground(red);
        }

        // ========= 加入模型 =========
        m_collectModel->setItem(row, WAVELENGTH, itemWave);
        m_collectModel->setItem(row, FILE_NAME, itemFile);
        m_collectModel->setItem(row, FILE_PATH, itemPath);
        m_collectModel->setItem(row, STATUS, itemStatus);
        m_collectModel->setItem(row, INTENSITY, itemIntensity);

        row++;
    }
}

void FormFittingPoints::closeEvent(QCloseEvent *event) { emit windowClose(); }

void FormFittingPoints::retranslateUI() { ui->retranslateUi(this); }

void FormFittingPoints::updateCollectionStatus(bool status) {
    ui->btnCollect->setEnabled(true);
    refreshCollectTable();
    int rows = m_collectModel->rowCount();

    for (int i = 0; i < rows; ++i) {
        QStandardItem *statusItem = m_collectModel->item(i, STATUS);

        if (!statusItem) continue;

        if (statusItem->text() == item_status[NOT_COLLECTED]) {
            // 第1列：文件名
            QStandardItem *fileItem = m_collectModel->item(i, FILE_NAME);

            if (!fileItem) continue;

            QString fileName = fileItem->text();

            // 设置到输入框
            ui->lineEditWavelength->setText(fileName);
            ui->tableViewCollectStatus->selectRow(i);
            ui->tableViewCollectStatus->scrollTo(m_collectModel->index(i, 0));

            return;  // 找到第一个就结束
        }
    }
}

void FormFittingPoints::setTargetIntensity(const double &avg) {
    auto sel = ui->tableViewCollectStatus->selectionModel()->selectedRows();

    if (sel.isEmpty()) return;

    int row = sel.first().row();
    QStandardItem *item = m_collectModel->item(row, INTENSITY);
    item->setText(QString::number(avg, 'f', 3));
}

void FormFittingPoints::on_tBtnSelectDir_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("choose dir"), "",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;

    ui->lineEditDir->setText(dir);
    SETTING_CONFIG_SET(CFG_GROUP_FITTING_POINTS, CFG_FITTING_POINTS_DIR, dir);
}

void FormFittingPoints::on_btnCollect_clicked() {
    ui->btnCollect->setEnabled(false);
    QString dir = ui->lineEditDir->text();
    QString file = ui->lineEditWavelength->text();
    int count = ui->spinBoxDataCount->value();
    emit toCollectionFittingPoints(dir, file, count);
}

void FormFittingPoints::on_tBtnRefresh_clicked() { refreshCollectTable(); }

void FormFittingPoints::on_tableViewCollectStatus_clicked(const QModelIndex &index) {
    int row = index.row();

    QStandardItem *fileItem = m_collectModel->item(row, FILE_NAME);
    if (!fileItem) return;

    ui->lineEditWavelength->setText(fileItem->text());
}

void FormFittingPoints::on_tableViewCollectStatus_doubleClicked(const QModelIndex &index) {
    int row = index.row();

    QStandardItem *pathItem = m_collectModel->item(row, FILE_PATH);
    if (!pathItem) return;
    QString path = pathItem->text();
    emit doFile(path);
}

void FormFittingPoints::on_tBtnFitting_clicked()
{
    QString fileName =  QDir::currentPath() + "/collection.csv";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR("Failed to open temp file");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "Wavelength,Intensity\n";

    int rows = m_collectModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        QString col_wavelength =
            m_collectModel->item(i, WAVELENGTH) ? m_collectModel->item(i, WAVELENGTH)->text() : "";
        QString col_intensity =
            m_collectModel->item(i, INTENSITY) ? m_collectModel->item(i, INTENSITY)->text() : "";

        out << col_wavelength << "," << col_intensity << "\n";
    }

    file.close();

    QString outFile = QDir::currentPath() + "/result.csv";

    QStringList cmd = {
        "script/fitting_points.py",
        "--input", fileName,
        "--output", outFile
    };

    MY_PROCESS.startAttach("python", cmd);
}

void FormFittingPoints::on_tBtnToHex_clicked()
{
    QString resultFile = QDir::currentPath() + "/result.csv";

    if (!QFile::exists(resultFile)) {
        QMessageBox::warning(this,
                             TITLE_WARNING,
                             "result.csv not exist! do fitting first!");
        return;
    }

    QFile file(resultFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "error", "open result.csv failed");
        return;
    }

    QTextStream in(&file);
    in.readLine();

    QStringList hexList;

    while (!in.atEnd()) {
        QStringList parts = in.readLine().split(",");

        if (parts.size() < 2) continue;

        bool ok = false;
        double val = parts[1].toDouble(&ok);

        if (ok) {
            int rounded = static_cast<int>(qRound(val)) & 0xFFFF;
            hexList << QString("%1").arg(rounded, 4, 16, QChar('0')).toUpper();
        }
    }

    file.close();

    QString hexLine = "DD3C064542" + hexList.join("") + "CDFF";

    QApplication::clipboard()->setText(hexLine);

    QMessageBox::information(this, tr("export success"), tr("hex add to clip board."));
}
