#include "darkspectrum.h"
#include "ui_darkspectrum.h"

DarkSpectrum::DarkSpectrum(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DarkSpectrum)
{
    ui->setupUi(this);
}

DarkSpectrum::~DarkSpectrum()
{
    delete ui;
}

void DarkSpectrum::closeEvent(QCloseEvent *event)
{
    emit windowClose();
}
