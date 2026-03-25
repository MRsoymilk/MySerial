#include "loadingoverlay.h"

#include "ui_loadingoverlay.h"

LoadingOverLay::LoadingOverLay(QWidget *parent) : QWidget(parent), ui(new Ui::LoadingOverLay) {
    ui->setupUi(this);
    init();
}

LoadingOverLay::~LoadingOverLay() { delete ui; }

void LoadingOverLay::updateTry(int count) {
    m_count = count;
    ui->labelTry->setText(QString("try: [%1]:").arg(m_count));
    ui->labelTry->update();
}

void LoadingOverLay::reTry() {
    ++m_count;
    ui->labelTry->setText(QString("try: [%1]:").arg(m_count));
    ui->labelTry->update();
}

void LoadingOverLay::updateInfo(int progress, const QString &msg) {
    ui->progressBar->setValue(progress);
    ui->labelInfo->setText(msg);

    ui->progressBar->update();
    ui->labelInfo->update();
}

void LoadingOverLay::init() {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAutoFillBackground(true);
    setStyleSheet("background-color: rgba(255, 255, 255, 120);");
}

void LoadingOverLay::on_btnStop_clicked() { emit stopConnect(); }
