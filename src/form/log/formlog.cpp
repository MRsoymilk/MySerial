#include "formlog.h"
#include "UILogSink.h"
#include "mylog.h"
#include "ui_formlog.h"

FormLog::FormLog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormLog)
{
    ui->setupUi(this);
    auto ui_sink = std::make_shared<UILogSink_mt>(ui->txtLog);
    ui_sink->set_pattern("[%H:%M:%S] [%l] %v");

    MY_LOG.getLogger()->sinks().push_back(ui_sink);
    ui->txtLog->setMaximumBlockCount(1000);
}

FormLog::~FormLog()
{
    delete ui;
}

void FormLog::retranslateUI()
{
    ui->retranslateUi(this);
}
