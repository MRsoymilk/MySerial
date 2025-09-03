#include "lengthcalc.h"
#include "ui_lengthcalc.h"

LengthCalc::LengthCalc(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LengthCalc)
{
    ui->setupUi(this);
}

LengthCalc::~LengthCalc()
{
    delete ui;
}

void LengthCalc::setFrame(const QString &head, const QString &foot)
{
    m_head = head;
    m_foot = foot;
}

void LengthCalc::on_btnCalculate_clicked()
{
    int data_size = ui->lineEditDataSize->text().toInt();
    m_length = data_size * 3 + m_head.length() / 2 + m_foot.length() / 2;
    ui->lineEditLength->setText(QString::number(m_length));
}

QString LengthCalc::getLength()
{
    return ui->lineEditLength->text();
}
