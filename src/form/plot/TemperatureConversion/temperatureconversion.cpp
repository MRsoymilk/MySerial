#include "temperatureconversion.h"
#include "ui_temperatureconversion.h"

TemperatureConversion::TemperatureConversion(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TemperatureConversion)
{
    ui->setupUi(this);
}

TemperatureConversion::~TemperatureConversion()
{
    delete ui;
}

double TemperatureConversion::k()
{
    return ui->doubleSpinBoxK->value();
}

double TemperatureConversion::b()
{
    return ui->doubleSpinBoxB->value();
}
