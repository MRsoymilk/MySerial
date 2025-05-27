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
    simulate();
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
    simulate();
    setINI();
}

void FormPlotSimulate::simulate()
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

    QList<QByteArray> headList, tailList;
    for (const QString &headStr : m_ini.head) {
        headList.append(QByteArray::fromHex(headStr.trimmed().toUtf8()));
    }
    for (const QString &tailStr : m_ini.tail) {
        tailList.append(QByteArray::fromHex(tailStr.trimmed().toUtf8()));
    }
    QMap<QByteArray, QString> m_name{
        {QByteArray::fromHex("DE3A096631"), "curve_24bit"},
        {QByteArray::fromHex("DE3A096633"), "curve_14bit"},
    };
    if (!headList.isEmpty() && !tailList.isEmpty()) {
        for (const QByteArray &headBytes : headList) {
            for (const QByteArray &tailBytes : tailList) {
                int pos = 0;
                while ((pos = dataBytes.indexOf(headBytes, pos)) != -1) {
                    int start = pos + headBytes.size();
                    int end = dataBytes.indexOf(tailBytes, start);
                    if (end != -1) {
                        QByteArray frame = headBytes + dataBytes.mid(start, end - start)
                                           + tailBytes;
                        emit simulateDataReady(frame, m_name[headBytes]);
                        pos = end + tailBytes.size();
                    } else {
                        break;
                    }
                }
            }
        }
    } else {
        SHOW_AUTO_CLOSE_MSGBOX(this, "Simulate", "Unsupport head/tail");
    }
}
