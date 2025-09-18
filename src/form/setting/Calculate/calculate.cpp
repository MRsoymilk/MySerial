#include "calculate.h"
#include "datadef.h"
#include "funcdef.h"
#include "ui_calculate.h"

Calculate::Calculate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calculate)
{
    ui->setupUi(this);
    init();
}

Calculate::~Calculate()
{
    delete ui;
}

void Calculate::init()
{
    m_iniCalculate.url = SETTING_CONFIG_GET(CFG_GROUP_SETTING,
                                            CFG_SETTING_FIT_SIN_URL,
                                            URL_FITTING_SIN);
    ui->lineEditCalculateURL->setText(m_iniCalculate.url);
}

void Calculate::retranslateUI()
{
    ui->retranslateUi(this);
}

void Calculate::on_lineEditCalculateURL_editingFinished()
{
    m_iniCalculate.url = ui->lineEditCalculateURL->text();
    SETTING_CONFIG_SET(CFG_GROUP_SETTING, CFG_SETTING_FIT_SIN_URL, m_iniCalculate.url);
}
