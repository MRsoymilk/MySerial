#include "formplotsimulate.h"
#include "ui_formplotsimulate.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

#include "funcdef.h"

FormPlotSimulate::FormPlotSimulate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlotSimulate)
{
    ui->setupUi(this);

    // ===== 初始化 Timer =====
    m_timer = new QTimer(this);
    m_timer->setInterval(5); // 5ms 一包，可调

    connect(m_timer, &QTimer::timeout, this, &FormPlotSimulate::sendChunk);

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

void FormPlotSimulate::init()
{
    getINI();
}

void FormPlotSimulate::getINI()
{
    m_ini.file = SETTING_CONFIG_GET(CFG_GROUP_SIMULATE, CFG_SIMULATE_FILE);

    ui->lineEditPath->setText(m_ini.file);
}

void FormPlotSimulate::setINI()
{
    SETTING_CONFIG_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_FILE, m_ini.file);
}

void FormPlotSimulate::on_btnLoadFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("choose file"), "", "(*.*)");

    if (path.isEmpty())
        return;

    ui->lineEditPath->setText(path);

    m_ini.file = path;

    simulate4k();
    setINI();
}

void FormPlotSimulate::on_toolButtonRe_clicked()
{
    m_ini.file = ui->lineEditPath->text();

    simulate4k();
    setINI();
}

void FormPlotSimulate::closeEvent(QCloseEvent *event)
{
    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotSimulate::simulate4k()
{
    if (m_timer->isActive())
        m_timer->stop();

    emit simulateReset();

    QFile file(m_ini.file);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString data;

    QString suffix = QFileInfo(file).suffix().toLower();

    if (suffix == "csv") {
        QTextStream in(&file);

        QString header = in.readLine();
        QStringList headers = header.split(',');

        int index = headers.indexOf("data");

        if (index < 0) {
            file.close();

            SHOW_AUTO_CLOSE_MSGBOX(this, tr("Error Read CSV"), tr("no data column found!"));
            return;
        }

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');

            if (fields.size() > index) {
                QString value = fields[index];

                value.replace("0x", "");
                value.remove(QRegularExpression("\\s+"));

                data += value;
            }
        }

    }
    else {
        data = file.readAll();

        data.replace("0x", "");
        data.remove(QRegularExpression("\\s+"));
    }

    file.close();

    m_sendData = QByteArray::fromHex(data.toUtf8());

    m_offset = 0;

    ui->progressBar->setValue(0);

    if (m_sendData.isEmpty())
        return;

    m_timer->start();
}

void FormPlotSimulate::sendChunk()
{
    const int chunkSize = 1024 * 10; // 10KB

    if (m_offset >= m_sendData.size()) {
        m_timer->stop();

        ui->progressBar->setValue(100);

        return;
    }

    int len = qMin(chunkSize, m_sendData.size() - m_offset);

    QByteArray chunk = m_sendData.mid(m_offset, len);

    emit simulateDataReady(chunk);

    m_offset += len;

    int percent = (int) (m_offset * 100 / m_sendData.size());

    ui->progressBar->setValue(percent);
}
