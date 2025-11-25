#include "signalnoiseratio.h"
#include "ui_signalnoiseratio.h"

SignalNoiseRatio::SignalNoiseRatio(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SignalNoiseRatio)
{
    ui->setupUi(this);
}

SignalNoiseRatio::~SignalNoiseRatio()
{
    delete ui;
}
