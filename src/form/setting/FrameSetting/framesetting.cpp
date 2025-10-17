#include "framesetting.h"
#include "LengthCalc/lengthcalc.h"
#include "funcdef.h"
#include "ui_framesetting.h"

FrameSetting::FrameSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FrameSetting)
{
    ui->setupUi(this);
    init();
}

FrameSetting::~FrameSetting()
{
    delete ui;
}

void FrameSetting::retranslateUI()
{
    ui->retranslateUi(this);
}

void FrameSetting::on_tBtnCalculate24bit_clicked()
{
    LengthCalc length;
    length.setFrame(ui->lineEditHead24bit->text(), ui->lineEditFoot24bit->text());
    length.exec();
    ui->lineEditLength24bit->setText(length.getLength());
}

void FrameSetting::on_tBtnCalculate14bit_clicked()
{
    LengthCalc length;
    length.setFrame(ui->lineEditHead14bit->text(), ui->lineEditFoot14bit->text());
    length.exec();
    ui->lineEditLength14bit->setText(length.getLength());
}

void FrameSetting::on_btnUpdate_clicked()
{
    SETTING_FRAME_SET(FRAME_CURVE_14, FRAME_HEADER, ui->lineEditHead14bit->text());
    SETTING_FRAME_SET(FRAME_CURVE_14, FRAME_FOOTER, ui->lineEditFoot14bit->text());
    SETTING_FRAME_SET(FRAME_CURVE_14, FRAME_LENGTH, ui->lineEditLength14bit->text());

    SETTING_FRAME_SET(FRAME_CURVE_24, FRAME_HEADER, ui->lineEditHead24bit->text());
    SETTING_FRAME_SET(FRAME_CURVE_24, FRAME_FOOTER, ui->lineEditFoot24bit->text());
    SETTING_FRAME_SET(FRAME_CURVE_24, FRAME_LENGTH, ui->lineEditLength24bit->text());

    SETTING_FRAME_SYNC();
}

void FrameSetting::init()
{
    ui->lineEditHead14bit->setText(SETTING_FRAME_GET(FRAME_CURVE_14, FRAME_HEADER));
    ui->lineEditFoot14bit->setText(SETTING_FRAME_GET(FRAME_CURVE_14, FRAME_FOOTER));
    ui->lineEditLength14bit->setText(SETTING_FRAME_GET(FRAME_CURVE_14, FRAME_LENGTH));
    ui->lineEditHead24bit->setText(SETTING_FRAME_GET(FRAME_CURVE_24, FRAME_HEADER));
    ui->lineEditFoot24bit->setText(SETTING_FRAME_GET(FRAME_CURVE_24, FRAME_FOOTER));
    ui->lineEditLength24bit->setText(SETTING_FRAME_GET(FRAME_CURVE_24, FRAME_LENGTH));
}
