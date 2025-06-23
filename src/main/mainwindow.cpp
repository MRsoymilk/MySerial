#include "mainwindow.h"
#include "../form/data/formdata.h"
#include "../form/log/formlog.h"
#include "../form/plot/formplot.h"
#include "../form/serial/formserial.h"
#include "../form/setting/formsetting.h"
#include "./ui_mainwindow.h"
#include "funcdef.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->showMessage(QString("version: %1 on %2").arg(APP_VERSION).arg(APP_REPO));
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initLanguage()
{
    QString language = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_LANGUAGE, "en");
    connect(ui->menuLanguage, &QMenu::triggered, this, &MainWindow::menuLanguageSelect);
    const QList<QAction *> actions = ui->menuLanguage->actions();
    for (QAction *act : actions) {
        if (act->text() == language) {
            menuLanguageSelect(act);
            return;
        }
    }
}

void MainWindow::initStackWidget()
{
    formSerial = new FormSerial(this);
    ui->stackedWidget->addWidget(formSerial);
    ui->stackedWidget->setCurrentWidget(formSerial);

    formPlot = new FormPlot(this);
    ui->stackedWidget->addWidget(formPlot);

    formData = new FormData(this);
    ui->stackedWidget->addWidget(formData);

    formLog = new FormLog(this);
    ui->stackedWidget->addWidget(formLog);

    formSetting = new FormSetting(this);
    ui->stackedWidget->addWidget(formSetting);

    QObject::connect(formSerial, &FormSerial::recv2Plot4k, formPlot, &FormPlot::onDataReceived4k);
    QObject::connect(formSerial, &FormSerial::recv2Data4k, formData, &FormData::onDataReceived4k);
    QObject::connect(formPlot, &FormPlot::sendKB, formSerial, &FormSerial::sendRaw);
}

void MainWindow::initToolbar()
{
    QList<QToolButton *> buttonList = {
        ui->btnSerial,
        ui->btnData,
        ui->btnLog,
        ui->btnPlot,
        ui->btnSetting,
    };

    QMap<QToolButton *, QString> onIcons = {
        {ui->btnSerial, ":/res/icons/usb-on.png"},
        {ui->btnData, ":/res/icons/data-on.png"},
        {ui->btnLog, ":/res/icons/log-on.png"},
        {ui->btnPlot, ":/res/icons/plot-on.png"},
        {ui->btnSetting, ":/res/icons/setting-on.png"},
    };

    QMap<QToolButton *, QString> offIcons = {
        {ui->btnSerial, ":/res/icons/usb-off.png"},
        {ui->btnData, ":/res/icons/data-off.png"},
        {ui->btnLog, ":/res/icons/log-off.png"},
        {ui->btnPlot, ":/res/icons/plot-off.png"},
        {ui->btnSetting, ":/res/icons/setting-off.png"},
    };

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

    // init language
    initLanguage();
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Tab) {
        int count = ui->stackedWidget->count();
        m_currentPageIndex = (m_currentPageIndex + 1) % count;
        ui->stackedWidget->setCurrentIndex(m_currentPageIndex);

        QList<QToolButton *> buttonList = {ui->btnSerial, ui->btnData, ui->btnPlot, ui->btnLog};
        buttonList[m_currentPageIndex]->click();

        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::on_btnSetting_clicked()
{
    ui->stackedWidget->setCurrentWidget(formSetting);
}

void MainWindow::setLanguage(const QString &language)
{
    QTranslator *translator = new QTranslator(this);
    if (translator->load(QString(":/res/i18n/%1.qm").arg(language))) {
        qApp->installTranslator(translator);
        ui->retranslateUi(this);
        if (formSerial) {
            formSerial->retranslateUI();
        }
        if (formData) {
            formData->retranslateUI();
        }
        if (formPlot) {
            formPlot->retranslateUI();
        }
        if (formLog) {
            formLog->retranslateUI();
        }
        if (formSetting) {
            formSetting->retranslateUI();
        }
    }
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_LANGUAGE, language);
}

void MainWindow::menuLanguageSelect(QAction *selectedAction)
{
    QString language = selectedAction->text();
    setLanguage(language);
    const QList<QAction *> actions = ui->menuLanguage->actions();
    for (QAction *act : actions) {
        if (act == selectedAction) {
            act->setChecked(true);
            act->setIcon(QIcon(":res/icons/yes.png"));
        } else {
            act->setChecked(false);
            act->setIcon(QIcon());
        }
    }
}
