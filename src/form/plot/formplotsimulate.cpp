#include "formplotsimulate.h"
#include <QFileDialog>
#include "funcdef.h"
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

void FormPlotSimulate::getINI()
{
    m_ini.file = SETTING_GET(CFG_GROUP_SIMULATE, CFG_SIMULATE_FILE);
    m_ini.head = SETTING_GET(CFG_GROUP_SIMULATE, CFG_SIMULATE_HEAD).split(",");
    m_ini.tail = SETTING_GET(CFG_GROUP_SIMULATE, CFG_SIMULATE_TAIL).split(",");
    ui->lineEditHead->setText(m_ini.head.join(","));
    ui->lineEditTail->setText(m_ini.tail.join(","));
    ui->lineEditPath->setText(m_ini.file);
}

void FormPlotSimulate::setINI()
{
    SETTING_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_HEAD, m_ini.head.join(","));
    SETTING_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_TAIL, m_ini.tail.join(","));
    SETTING_SET(CFG_GROUP_SIMULATE, CFG_SIMULATE_FILE, m_ini.file);
}

void FormPlotSimulate::init()
{
    getINI();
}

void FormPlotSimulate::on_btnLoadFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "choose file", "", "(*.*)");
    if (filePath.isEmpty())
        return;

    ui->lineEditPath->setText(filePath);

    m_ini.head = ui->lineEditHead->text().split(",", Qt::SkipEmptyParts);
    m_ini.tail = ui->lineEditTail->text().split(",", Qt::SkipEmptyParts);
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
    m_ini.head = ui->lineEditHead->text().split(",", Qt::SkipEmptyParts);
    m_ini.tail = ui->lineEditTail->text().split(",", Qt::SkipEmptyParts);
    m_ini.file = ui->lineEditPath->text();
    simulate4k();
    setINI();
}

void FormPlotSimulate::simulate4k()
{
    QFile file(m_ini.file);
    QString data;
    if (file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        data.replace("0x", "");
        data.remove(QRegularExpression("[\\s\r\n\t]"));
        file.close();
    } else {
        return;
    }

    QByteArray dataBytes = QByteArray::fromHex(data.toUtf8());
    QByteArray buffer = dataBytes;
    struct FrameType
    {
        QByteArray header;
        QString name;
    };
    struct FRAME
    {
        QByteArray bit14;
        QByteArray bit24;
    };
    const QByteArray footer = QByteArray::fromHex("CEFF");
    FRAME frame;
    const QList<FrameType> m_frameTypes = {
        {QByteArray::fromHex("DE3A096631"), "curve_24bit"},
        {QByteArray::fromHex("DE3A096633"), "curve_14bit"},
    };
    while (true) {
        int firstHeaderIdx = -1;
        int matchedHeaderLen = 0;
        QString matchedType;

        for (const auto &type : m_frameTypes) {
            int idx = buffer.indexOf(type.header);
            if (idx != -1 && (firstHeaderIdx == -1 || idx < firstHeaderIdx)) {
                firstHeaderIdx = idx;
                matchedHeaderLen = type.header.size();
                matchedType = type.name;
            }
        }

        if (firstHeaderIdx == -1) {
            SHOW_AUTO_CLOSE_MSGBOX(this, "Simulate", "no header found!");
            LOG_WARN("no header found: {}", file.fileName());
            break;
        }

        int endIdx = buffer.indexOf(footer, firstHeaderIdx + matchedHeaderLen);
        if (endIdx == -1) {
            SHOW_AUTO_CLOSE_MSGBOX(this, "Simulate", "simulate finish");
            return;
        }

        int frameLen = endIdx + footer.size() - firstHeaderIdx;
        QByteArray tmp_frame = buffer.mid(firstHeaderIdx, frameLen);

        if (matchedType == "curve_24bit") {
            frame.bit24 = tmp_frame;
        } else if (matchedType == "curve_14bit") {
            frame.bit14 = tmp_frame;
            if (!frame.bit24.isEmpty()) {
                emit simulateDataReady4k(frame.bit14, frame.bit24);
            }
        }

        buffer.remove(0, endIdx + footer.size());
    }
}
