#include "formplotsimulate.h"
#include <QFileDialog>
#include "funcdef.h"
#include "plot_algorithm.h"
#include "ui_formplotsimulate.h"

FormPlotSimulate::FormPlotSimulate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlotSimulate)
{
    ui->setupUi(this);
    init();
}

FormPlotSimulate::~FormPlotSimulate()
{
    delete ui;
}

void FormPlotSimulate::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormPlotSimulate::onChangeFrameType(int index)
{
    m_algorithm = index;
}

void FormPlotSimulate::getINI()
{
    m_ini.file = SETTING_CONFIG_GET(CFG_GROUP_SIMULATE, CFG_SIMULATE_FILE);
    m_ini.head = SETTING_CONFIG_GET(CFG_GROUP_SIMULATE, CFG_SIMULATE_HEAD).split(",");
    m_ini.tail = SETTING_CONFIG_GET(CFG_GROUP_SIMULATE, CFG_SIMULATE_TAIL).split(",");
    m_ini.option_correction = SETTING_CONFIG_GET(CFG_GROUP_SIMULATE,
                                                 CFG_SIMULATE_OPTION_CORRECTION,
                                                 VAL_DISABLE);
    ui->lineEditHead->setText(m_ini.head.join(","));
    ui->lineEditTail->setText(m_ini.tail.join(","));
    ui->lineEditPath->setText(m_ini.file);
    if (m_ini.option_correction == VAL_ENABLE) {
        ui->checkBoxCorrection->setChecked(true);
    }
    m_algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
}

void FormPlotSimulate::setINI()
{
    SETTING_CONFIG_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_HEAD, m_ini.head.join(","));
    SETTING_CONFIG_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_TAIL, m_ini.tail.join(","));
    SETTING_CONFIG_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_FILE, m_ini.file);
    SETTING_CONFIG_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_OPTION_CORRECTION, m_ini.option_correction);
}

void FormPlotSimulate::init()
{
    getINI();
}

void FormPlotSimulate::on_btnLoadFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("choose file"), "", "(*.*)");
    if (filePath.isEmpty())
        return;

    ui->lineEditPath->setText(filePath);

    m_ini.head = ui->lineEditHead->text().split(",", Qt::SkipEmptyParts);
    m_ini.tail = ui->lineEditTail->text().split(",", Qt::SkipEmptyParts);
    m_ini.file = filePath;
    m_ini.option_correction = ui->checkBoxCorrection->isChecked() ? VAL_ENABLE : VAL_DISABLE;
    simulate4k();
    setINI();
}

void FormPlotSimulate::closeEvent(QCloseEvent *event)
{
    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotSimulate::on_toolButtonRe_clicked()
{
    m_ini.head = ui->lineEditHead->text().split(",", Qt::SkipEmptyParts);
    m_ini.tail = ui->lineEditTail->text().split(",", Qt::SkipEmptyParts);
    m_ini.file = ui->lineEditPath->text();
    m_ini.option_correction = ui->checkBoxCorrection->isChecked() ? VAL_ENABLE : VAL_DISABLE;
    simulate4k();
    setINI();
}

void FormPlotSimulate::simulate4k()
{
    bool option = false;
    bool wait_delete = false;
    if (m_ini.option_correction == VAL_ENABLE) {
        option = true;
    } else {
        option = false;
    }
    emit simulateOption(option);

    QFile file(m_ini.file);
    QString data;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QString suffix = QFileInfo(file).suffix().toLower();

    if (suffix == "csv") {
        QTextStream in(&file);
        QString headerLine = in.readLine();
        QStringList headers = headerLine.split(',');

        int dataIndex = headers.indexOf("data");
        if (dataIndex == -1) {
            file.close();
            SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error Read CSV"), tr("no data column found!"));
            return;
        }

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            if (fields.size() > dataIndex) {
                QString value = fields[dataIndex];
                value.replace("0x", "");
                value.remove(QRegularExpression("[\\s\r\n\t]"));
                data += value;
            }
        }
    } else {
        data = file.readAll();
        data.replace("0x", "");
        data.remove(QRegularExpression("[\\s\r\n\t]"));
    }

    file.close();

    QByteArray dataBytes = QByteArray::fromHex(data.toUtf8());

    const int chunkSize = 1024;
    for (int i = 0; i < dataBytes.size(); i += chunkSize) {
        QByteArray chunk = dataBytes.mid(i, chunkSize);
        emit simulateDataReady(chunk);
    }
}

void FormPlotSimulate::on_checkBoxCorrection_clicked()
{
    m_ini.option_correction = ui->checkBoxCorrection->isChecked() ? VAL_ENABLE : VAL_DISABLE;
}
