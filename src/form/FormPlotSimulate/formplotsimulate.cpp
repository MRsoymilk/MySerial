#include "formplotsimulate.h"
#include "ui_formplotsimulate.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>

#include "funcdef.h"

FormPlotSimulate::FormPlotSimulate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlotSimulate)
{
    ui->setupUi(this);

    m_timer = new QTimer(this);
    m_timer->setInterval(5);

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

// ================= INI =================

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

    m_frames.clear();
    m_frameIndex = 0;

    QFile file(m_ini.file);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString suffix = QFileInfo(file).suffix().toLower();

    QTextStream in(&file);

    // ---------- CSV ----------
    if (suffix == "csv") {
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
            if (fields.size() <= index)
                continue;
            QString value = fields[index];
            value.replace("0x", "");
            value.remove(QRegularExpression("\\s+"));
            if (value.isEmpty())
                continue;
            QByteArray frame = QByteArray::fromHex(value.toUtf8());
            if (!frame.isEmpty())
                m_frames.append(frame);
        }
    }

    // ---------- TXT ----------
    else {
        while (!in.atEnd()) {
            QString line = in.readLine();

            line.replace("0x", "");
            line.remove(QRegularExpression("\\s+"));

            if (line.isEmpty())
                continue;

            QByteArray frame = QByteArray::fromHex(line.toUtf8());

            if (!frame.isEmpty())
                m_frames.append(frame);
        }
    }

    file.close();

    ui->progressBar->setValue(0);

    if (m_frames.isEmpty()) {
        return;
    }

    m_timer->start();
}

void FormPlotSimulate::sendChunk()
{
    if (m_frameIndex >= m_frames.size()) {
        m_timer->stop();
        ui->progressBar->setValue(100);
        return;
    }

    emit simulateDataReady(m_frames[m_frameIndex]);

    m_frameIndex++;
    int percent = m_frameIndex * 100 / m_frames.size();
    ui->progressBar->setValue(percent);
}
