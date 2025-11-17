#include "formplotdata.h"
#include "ui_formplotdata.h"

#include <QFileDialog>
#include <QMenu>
#include <QShortcut>
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

void FormPlotData::displayData(const QVector<double> &v31,
                               const QVector<double> &v33,
                               const QVector<double> &raw31,
                               const QVector<double> &raw33)
{
    clearData();
    int count = qMax(qMax(v31.size(), v33.size()), qMax(raw31.size(), raw33.size()));
    for (int i = 0; i < count; ++i) {
        QString index = QString::number(i);
        QString yV14 = (i < v31.size()) ? QString::number(v31[i]) : "";
        QString yV24 = (i < v33.size()) ? QString::number(v33[i]) : "";
        QString yR14 = (i < raw31.size()) ? QString::number(raw31[i]) : "";
        QString yR24 = (i < raw33.size()) ? QString::number(raw33[i]) : "";

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(index);
        rowItems << new QStandardItem(yV14);
        rowItems << new QStandardItem(yV24);
        rowItems << new QStandardItem(yR14);
        rowItems << new QStandardItem(yR24);
        m_model->appendRow(rowItems);
    }

    if (ui->checkBoxHighlightMinMax->isChecked()) {
        int columnCount = m_model->columnCount();
        int rowCount = m_model->rowCount();
        QStringList infoList;

        for (int col = 1; col < columnCount; ++col) {
            double minVal = std::numeric_limits<double>::max();
            double maxVal = std::numeric_limits<double>::lowest();
            int minRow = -1, maxRow = -1;

            for (int row = 0; row < rowCount; ++row) {
                bool ok = false;
                double val = m_model->item(row, col)->text().toDouble(&ok);
                if (!ok)
                    continue; // 跳过空值或非数值

                if (val < minVal) {
                    minVal = val;
                    minRow = row;
                }
                if (val > maxVal) {
                    maxVal = val;
                    maxRow = row;
                }
            }

            if (minRow >= 0) {
                m_model->item(minRow, col)->setBackground(QBrush(Qt::green));
            }
            if (maxRow >= 0) {
                m_model->item(maxRow, col)->setBackground(QBrush(Qt::red));
            }
            QString header = m_model->headerData(col, Qt::Horizontal).toString();
            QString info = QString("%1:"
                                   "max= %2 (at %3), min= %4 (at=%5)")
                               .arg(header)
                               .arg(maxVal)
                               .arg(maxRow)
                               .arg(minVal)
                               .arg(minRow);
            infoList << info;
        }
        ui->labelMinMaxInfo->setText(infoList.join('\n'));
    }
}

void FormPlotData::updateTable4k(const QVector<double> &v31,
                                 const QVector<double> &v33,
                                 const QVector<double> &raw31,
                                 const QVector<double> &raw33)
{
    if (this->isVisible()) {
        ++m_count;
        m_current = m_count;
        listV31.append(v31);
        listV33.append(v33);
        listRaw31.append(raw31);
        listRaw33.append(raw33);
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV31.back(), listV33.back(), listRaw31.back(), listRaw33.back());
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

    ui->checkBoxChooseGroup->setCheckState(Qt::CheckState::Unchecked);
    ui->lineEditTarget->setEnabled(false);
    ui->cBoxRaw14->setChecked(true);
    ui->cBoxRaw24->setChecked(true);
    ui->cBoxV14->setChecked(true);
    ui->cBoxV24->setChecked(true);

    QShortcut *shortcut_prev = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcut_prev, &QShortcut::activated, this, &FormPlotData::on_tBtnPrev_clicked);
    QShortcut *shortcut_next = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(shortcut_next, &QShortcut::activated, this, &FormPlotData::on_tBtnNext_clicked);
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

auto parseGroupRange = [](const QString &text, int maxGroup) -> QList<int> {
    QSet<int> tempSet;
    const QStringList parts = text.split(",", Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        QString trimmed = part.trimmed();
        if (trimmed.contains("-")) {
            const QStringList range = trimmed.split("-");
            if (range.size() == 2) {
                bool ok1 = false, ok2 = false;
                int start = range[0].toInt(&ok1);
                int end = range[1].toInt(&ok2);
                if (ok1 && ok2 && start <= end) {
                    for (int i = start; i <= end; ++i) {
                        if (i > 0 && i <= maxGroup)
                            tempSet.insert(i - 1); // 1-based to 0-based
                    }
                }
            }
        } else {
            bool ok = false;
            int val = trimmed.toInt(&ok);
            if (ok && val > 0 && val <= maxGroup) {
                tempSet.insert(val - 1); // 1-based to 0-based
            }
        }
    }

    QList<int> result = tempSet.values();
    std::sort(result.begin(), result.end());
    return result;
};

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

    int groupCount = listV31.size();
    if (groupCount == 0) {
        LOG_WARN("No data to export!");
        return;
    }

    QString targetStr;
    if (ui->checkBoxChooseGroup->isChecked()) {
        targetStr = ui->lineEditTarget->text().trimmed();
    } else {
        targetStr = QString("1-%2").arg(groupCount);
    }
    if (targetStr.isEmpty()) {
        LOG_WARN("No group specified in target input!");
        return;
    }

    QList<int> selectedGroups = parseGroupRange(targetStr, groupCount);
    if (selectedGroups.isEmpty()) {
        LOG_WARN("No valid groups parsed from target input!");
        return;
    }

    int rowCount = m_model->rowCount();

    bool exportV14 = ui->cBoxV14->isChecked();
    bool exportV24 = ui->cBoxV24->isChecked();
    bool exportRaw14 = ui->cBoxRaw14->isChecked();
    bool exportRaw24 = ui->cBoxRaw24->isChecked();

    QString nameV14 = m_model->headerData(1, Qt::Horizontal).toString();
    QString nameV24 = m_model->headerData(2, Qt::Horizontal).toString();
    QString nameRaw14 = m_model->headerData(3, Qt::Horizontal).toString();
    QString nameRaw24 = m_model->headerData(4, Qt::Horizontal).toString();

    QStringList headers;
    headers << m_model->headerData(0, Qt::Horizontal).toString();
    for (int groupIndex : selectedGroups) {
        if (exportV14)
            headers << QString("%1_%2").arg(nameV14).arg(groupIndex + 1);
        if (exportV24)
            headers << QString("%1_%2").arg(nameV24).arg(groupIndex + 1);
        if (exportRaw14)
            headers << QString("%1_%2").arg(nameRaw14).arg(groupIndex + 1);
        if (exportRaw24)
            headers << QString("%1_%2").arg(nameRaw24).arg(groupIndex + 1);
    }
    stream << headers.join(",") << "\n";

    for (int row = 0; row < rowCount; ++row) {
        QStringList rowData;
        rowData << QString::number(row);

        for (int groupIndex : selectedGroups) {
            bool hasV14 = (groupIndex < listV31.size());
            bool hasV24 = (groupIndex < listV33.size());
            bool hasRaw14 = (groupIndex < listRaw31.size());
            bool hasRaw24 = (groupIndex < listRaw33.size());

            if (exportV14)
                rowData << (hasV14 && row < listV31[groupIndex].size()
                                ? QString::number(listV31[groupIndex][row], 'f', 6)
                                : "");
            if (exportV24)
                rowData << (hasV24 && row < listV33[groupIndex].size()
                                ? QString::number(listV33[groupIndex][row], 'f', 6)
                                : "");
            if (exportRaw14)
                rowData << (hasRaw14 && row < listRaw31[groupIndex].size()
                                ? QString::number(listRaw31[groupIndex][row])
                                : "");
            if (exportRaw24)
                rowData << (hasRaw24 && row < listRaw33[groupIndex].size()
                                ? QString::number(listRaw33[groupIndex][row])
                                : "");
        }

        stream << rowData.join(",") << "\n";
    }

    file.close();
    QString msg = QString("Selected group data exported to %1").arg(path);
    SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export"), msg);
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

    QVector<int> selectedCols;
    selectedCols << 0;

    if (ui->cBoxV14->isChecked())
        selectedCols << 1;
    if (ui->cBoxV24->isChecked())
        selectedCols << 2;
    if (ui->cBoxRaw14->isChecked())
        selectedCols << 3;
    if (ui->cBoxRaw24->isChecked())
        selectedCols << 4;

    if (selectedCols.size() == 1) {
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export"), tr("No data columns selected for export."));
        return;
    }

    QStringList headers;
    for (int col : selectedCols) {
        headers << m_model->headerData(col, Qt::Horizontal).toString();
    }
    stream << headers.join(",") << "\n";

    int rowCount = m_model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QStringList rowData;
        for (int col : selectedCols) {
            QStandardItem *item = m_model->item(row, col);
            rowData << (item ? item->text() : "");
        }
        stream << rowData.join(",") << "\n";
    }

    file.close();
    QString msg = QString("Selected data exported to %1").arg(path);
    SHOW_AUTO_CLOSE_MSGBOX(this, tr("Export"), msg);
    LOG_INFO(msg);
}

void FormPlotData::on_tBtnPrev_clicked()
{
    if (m_current - 1 > 0) {
        --m_current;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV31[m_current - 1],
                    listV33[m_current - 1],
                    listRaw31[m_current - 1],
                    listRaw33[m_current - 1]);
    }
}

void FormPlotData::on_tBtnNext_clicked()
{
    if (m_current + 1 <= m_count) {
        ++m_current;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV31[m_current - 1],
                    listV33[m_current - 1],
                    listRaw31[m_current - 1],
                    listRaw33[m_current - 1]);
    }
}

void FormPlotData::on_lineEditGo_editingFinished()
{
    int target = ui->lineEditGo->text().toInt();
    if (target > 0 && target <= m_count) {
        m_current = target;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_count));
        displayData(listV31[m_current - 1],
                    listV33[m_current - 1],
                    listRaw31[m_current - 1],
                    listRaw33[m_current - 1]);
    }
}

void FormPlotData::on_checkBoxChooseGroup_checkStateChanged(const Qt::CheckState &state)
{
    if (state == Qt::CheckState::Checked) {
        ui->lineEditTarget->setEnabled(true);
    } else if (state == Qt::CheckState::Unchecked) {
        ui->lineEditTarget->setEnabled(false);
    }
}
