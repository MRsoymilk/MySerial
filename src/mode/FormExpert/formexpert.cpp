#include "formexpert.h"
#include "ui_formexpert.h"

FormExpert::FormExpert(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormExpert)
{
    ui->setupUi(this);
}

FormExpert::~FormExpert()
{
    delete ui;
}
