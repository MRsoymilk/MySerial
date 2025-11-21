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
    ui->lineEditPath->setText(m_ini.file);
    m_algorithm = SETTING_CONFIG_GET(CFG_GROUP_PLOT, CFG_PLOT_ALGORITHM, "0").toInt();
}

void FormPlotSimulate::setINI()
{
    SETTING_CONFIG_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_FILE, m_ini.file);
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

    m_ini.file = filePath;
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
    m_ini.file = ui->lineEditPath->text();
    simulate4k();
    setINI();
}

void FormPlotSimulate::simulate4k()
{
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

    const int chunkSize = 1024 * 10;
    long long total = dataBytes.size();
    long long sent = 0;

    for (; sent < total; sent += chunkSize) {
        long long len = qMin(chunkSize, total - sent);
        QByteArray chunk = dataBytes.mid(sent, len);

        emit simulateDataReady(chunk);

        ui->progressBar->setValue((sent + len) * 100 / total);
        QCoreApplication::processEvents();
    }
}
