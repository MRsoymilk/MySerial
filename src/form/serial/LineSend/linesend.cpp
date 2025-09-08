#include "linesend.h"
#include "ui_linesend.h"

LineSend::LineSend(int index, QWidget *parent)
    : m_index(index)
    , QWidget(parent)
    , ui(new Ui::LineSend)
{
    ui->setupUi(this);
}

LineSend::~LineSend()
{
    delete ui;
}

void LineSend::setLabel(const QString &label)
{
    ui->lineEdit_label->setText(label);
}

void LineSend::setCmd(const QString &cmd)
{
    ui->lineEdit_cmd->setText(cmd);
}

void LineSend::setBtn(const QString &name)
{
    ui->tBtn->setText(name);
}

QString LineSend::getLabel() const
{
    return ui->lineEdit_label->text();
}

QString LineSend::getCmd() const
{
    return ui->lineEdit_cmd->text();
}

void LineSend::on_lineEdit_label_editingFinished()
{
    emit labelEdited(m_index, ui->lineEdit_label->text());
}

void LineSend::on_lineEdit_cmd_editingFinished()
{
    emit cmdEdited(m_index, ui->lineEdit_cmd->text());
}

void LineSend::on_tBtn_clicked()
{
    emit cmdSendClicked(m_index, ui->lineEdit_cmd->text());
}
