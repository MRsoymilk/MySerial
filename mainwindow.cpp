#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "form/data/formdata.h"
#include "form/log/formlog.h"
#include "form/plot/formplot.h"
#include "form/serial/formserial.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initStackWidget()
{
    formSerial = new FormSerial(this);
    formPlot = new FormPlot(this);
    formData = new FormData(this);
    formLog = new FormLog(this);

    ui->stackedWidget->addWidget(formSerial);
    ui->stackedWidget->addWidget(formPlot);
    ui->stackedWidget->addWidget(formData);
    ui->stackedWidget->addWidget(formLog);
    QObject::connect(formSerial, &FormSerial::dataReceived, formPlot, &FormPlot::onDataReceived);

    ui->stackedWidget->setCurrentWidget(formSerial);
}

void MainWindow::initToolbar()
{
    QList<QToolButton *> buttonList = {ui->btnSerial, ui->btnData, ui->btnLog, ui->btnPlot};

    QMap<QToolButton *, QString> onIcons = {{ui->btnSerial, ":/res/icons/usb-on.png"},
                                            {ui->btnData, ":/res/icons/data-on.png"},
                                            {ui->btnLog, ":/res/icons/log-on.png"},
                                            {ui->btnPlot, ":/res/icons/plot-on.png"}};

    QMap<QToolButton *, QString> offIcons = {{ui->btnSerial, ":/res/icons/usb-off.png"},
                                             {ui->btnData, ":/res/icons/data-off.png"},
                                             {ui->btnLog, ":/res/icons/log-off.png"},
                                             {ui->btnPlot, ":/res/icons/plot-off.png"}};

    for (QToolButton *btn : buttonList) {
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setIconSize(QSize(32, 32));
        btn->setStyleSheet("QToolButton { border: none; background: transparent; }");

        connect(btn, &QToolButton::clicked, this, [=]() {
            for (QToolButton *b : buttonList) {
                b->setIcon(QIcon(offIcons[b]));
            }
            btn->setIcon(QIcon(onIcons[btn]));
        });
    }

    ui->btnSerial->click();
}

void MainWindow::init()
{
    // init stacked widget
    initStackWidget();

    // init toolbar
    initToolbar();
}

void MainWindow::on_btnSerial_clicked()
{
    ui->stackedWidget->setCurrentWidget(formSerial);
}

void MainWindow::on_btnPlot_clicked()
{
    ui->stackedWidget->setCurrentWidget(formPlot);
}

void MainWindow::on_btnData_clicked()
{
    ui->stackedWidget->setCurrentWidget(formData);
}

void MainWindow::on_btnLog_clicked()
{
    ui->stackedWidget->setCurrentWidget(formLog);
}
