#include "mainwindow.h"
#include "../form/data/formdata.h"
#include "../form/log/formlog.h"
#include "../form/play/formplaympu6050.h"
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
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initMsgBar()
{
    QLabel *linkLabel = new QLabel(this);
    linkLabel->setText(
        QString(
            "version: %1 on "
            "<a href='%2'><span style='text-decoration: underline; color: #2980b9;'>%2</span></a>")
            .arg(APP_VERSION)
            .arg(APP_REPO));
    linkLabel->setTextFormat(Qt::RichText);
    linkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    linkLabel->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(linkLabel);
}

void MainWindow::initTheme()
{
    QString theme = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_THEME, "Lite");
    connect(ui->menuTheme, &QMenu::triggered, this, &MainWindow::menuThemeSelect);
    const QList<QAction *> actions = ui->menuTheme->actions();
    for (QAction *act : actions) {
        if (act->text() == theme) {
            menuThemeSelect(act);
            return;
        }
    }
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
    formPlot = new FormPlot(this);
    ui->stackedWidget->addWidget(formPlot);

    formSerial = new FormSerial(this);
    ui->stackedWidget->addWidget(formSerial);

    formData = new FormData(this);
    ui->stackedWidget->addWidget(formData);

    formLog = new FormLog(this);
    ui->stackedWidget->addWidget(formLog);

    formSetting = new FormSetting(this);
    ui->stackedWidget->addWidget(formSetting);

    playMPU6050 = new FormPlayMPU6050(this);
    ui->stackedWidget->addWidget(playMPU6050);

    QObject::connect(formSerial, &FormSerial::recv2Plot4k, formPlot, &FormPlot::onDataReceived4k);
    QObject::connect(formSerial, &FormSerial::recv2Data4k, formData, &FormData::onDataReceived4k);
    QObject::connect(formPlot, &FormPlot::sendKB, formSerial, &FormSerial::sendRaw);
    QObject::connect(formPlot,
                     &FormPlot::changeFrameType,
                     formSerial,
                     &FormSerial::onChangeFrameType);
    QObject::connect(formSerial, &FormSerial::recv2MPU, playMPU6050, &FormPlayMPU6050::onRecvMPU);

    ui->stackedWidget->setCurrentWidget(formSerial);

    QShortcut *shortcut_Serial = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_1), this);
    QShortcut *shortcut_Data = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_2), this);
    QShortcut *shortcut_Plot = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_3), this);
    QShortcut *shortcut_Log = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_4), this);
    QShortcut *shortcut_Setting = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_5), this);
    QShortcut *shortcut_Play = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_6), this);
    connect(shortcut_Serial, &QShortcut::activated, this, [this]() {
        ui->stackedWidget->setCurrentWidget(formSerial);
    });
    connect(shortcut_Data, &QShortcut::activated, this, [this]() {
        ui->stackedWidget->setCurrentWidget(formData);
    });
    connect(shortcut_Plot, &QShortcut::activated, this, [this]() {
        ui->stackedWidget->setCurrentWidget(formPlot);
    });
    connect(shortcut_Log, &QShortcut::activated, this, [this]() {
        ui->stackedWidget->setCurrentWidget(formLog);
    });
    connect(shortcut_Setting, &QShortcut::activated, this, [this]() {
        ui->stackedWidget->setCurrentWidget(formSetting);
    });
    connect(shortcut_Play, &QShortcut::activated, this, [this]() {
        ui->stackedWidget->setCurrentWidget(playMPU6050);
    });
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
        {ui->btnSerial, "usb-on"},
        {ui->btnData, "data-on"},
        {ui->btnLog, "log-on"},
        {ui->btnPlot, "plot-on"},
        {ui->btnSetting, "setting-on"},
    };

    QMap<QToolButton *, QString> offIcons = {
        {ui->btnSerial, "usb-off"},
        {ui->btnData, "data-off"},
        {ui->btnLog, "log-off"},
        {ui->btnPlot, "plot-off"},
        {ui->btnSetting, "setting-off"},
    };

    for (QToolButton *btn : buttonList) {
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setIconSize(QSize(32, 32));
        btn->setStyleSheet("QToolButton { border: none; background: transparent; }");

        connect(btn, &QToolButton::clicked, this, [=]() {
            for (QToolButton *b_off : buttonList) {
                b_off->setObjectName(offIcons[b_off]);
                b_off->style()->unpolish(b_off);
                b_off->style()->polish(b_off);
                b_off->update();
            }
            btn->setObjectName(onIcons[btn]);
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            btn->update();
        });
    }

    ui->btnSerial->click();
}

void MainWindow::init()
{
    initMsgBar();
    initStackWidget();
    initToolbar();
    initLanguage();
    initTheme();
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

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillRect(rect(), Qt::transparent);

    if (!m_background.isNull()) {
        QPixmap scaled = m_background.scaled(this->size(),
                                             Qt::KeepAspectRatioByExpanding,
                                             Qt::SmoothTransformation);

        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
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

void MainWindow::setTheme(const QString &theme)
{
    QFile file(QString(":/res/themes/%1.qss").arg(theme));
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString style = file.readAll();
        qApp->setStyleSheet(style);
    }
    if (theme == "HelloKitty") {
        m_background = QPixmap(":/res/themes/background/HelloKitty.jpg");
    }
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_THEME, theme);
}

void MainWindow::menuLanguageSelect(QAction *selectedAction)
{
    QString language = selectedAction->text();
    setLanguage(language);
    const QList<QAction *> actions = ui->menuLanguage->actions();
    for (QAction *act : actions) {
        if (act == selectedAction) {
            act->setChecked(true);
            if (m_theme.endsWith("Lite")) {
                act->setIcon(QIcon(":res/icons/yes.png"));
            } else {
                act->setIcon(QIcon(":res/icons/yes_white.png"));
            }

        } else {
            act->setChecked(false);
            act->setIcon(QIcon());
        }
    }
}

void MainWindow::menuThemeSelect(QAction *selectedTheme)
{
    QString theme = selectedTheme->text();
    setTheme(theme);
    m_theme = theme;
    const QList<QAction *> actions = ui->menuTheme->actions();
    for (QAction *act : actions) {
        if (act == selectedTheme) {
            act->setChecked(true);
            if (theme.endsWith("Lite")) {
                act->setIcon(QIcon(":res/icons/yes.png"));
            } else {
                act->setIcon(QIcon(":res/icons/yes_white.png"));
            }

        } else {
            act->setChecked(false);
            act->setIcon(QIcon());
        }
    }
}
