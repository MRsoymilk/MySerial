#include "formeasy.h"
#include "ui_formeasy.h"

FormEasy::FormEasy(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormEasy)
{
    ui->setupUi(this);
}

FormEasy::~FormEasy()
{
    delete ui;
}
