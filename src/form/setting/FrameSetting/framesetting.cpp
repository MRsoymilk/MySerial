#include "framesetting.h"
#include "LengthCalc/lengthcalc.h"
#include "ui_framesetting.h"

FrameSetting::FrameSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FrameSetting)
{
    ui->setupUi(this);
}

FrameSetting::~FrameSetting()
{
    delete ui;
}

void FrameSetting::on_tBtnCalculate24bit_clicked()
{
    LengthCalc length;
    length.setFrame(ui->lineEditHead24bit->text(), ui->lineEditFoot24bit->text());
    length.exec();
}

void FrameSetting::on_tBtnCalculate14bit_clicked()
{
    LengthCalc length;
    length.setFrame(ui->lineEditHead14bit->text(), ui->lineEditFoot14bit->text());
    length.exec();
}

void FrameSetting::on_btnUpdate_clicked() {}
