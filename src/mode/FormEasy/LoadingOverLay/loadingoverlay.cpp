#include "loadingoverlay.h"
#include "ui_loadingoverlay.h"

LoadingOverLay::LoadingOverLay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoadingOverLay)
{
    ui->setupUi(this);
    init();
}

LoadingOverLay::~LoadingOverLay()
{
    delete ui;
}

void LoadingOverLay::updateTry(int count)
{
    ui->labelTry->setText(QString("try: [%1]:").arg(count));
    QApplication::processEvents();
}

void LoadingOverLay::updateInfo(int progress, const QString &msg)
{
    ui->progressBar->setValue(progress);
    ui->labelInfo->setText(msg);
    QApplication::processEvents();
}

void LoadingOverLay::init()
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAutoFillBackground(true);
    setStyleSheet("background-color: rgba(255, 255, 255, 120);");
}
