#include "mainwindow.h"

#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QTranslator>
#include "../mode/FormEasy/formeasy.h"
#include "../mode/FormExpert/formexpert.h"
#include "./ui_mainwindow.h"
#include "funcdef.h"
#include "global.h"
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
    linkLabel->setText(QString("version: %1").arg(APP_VERSION));
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
        act->setCheckable(true);
        if (act->text() == theme) {
            menuThemeSelect(act);
        }
    }
}

void MainWindow::initLanguage()
{
    QString language = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_LANGUAGE, "en");
    connect(ui->menuLanguage, &QMenu::triggered, this, &MainWindow::menuLanguageSelect);
    const QList<QAction *> actions = ui->menuLanguage->actions();
    for (QAction *act : actions) {
        act->setCheckable(true);
        if (act->text() == language) {
            menuLanguageSelect(act);
        }
    }
}

void MainWindow::initAlgorithm()
{
    QString algorithm = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_ALGORITHM, "Freedom");
    connect(ui->menuAlgorithm, &QMenu::triggered, this, &MainWindow::menuAlgorithmSelect);
    const QList<QAction *> actions = ui->menuAlgorithm->actions();
    for (QAction *act : actions) {
        act->setCheckable(true);
        if (act->text() == algorithm) {
            menuAlgorithmSelect(act);
        }
    }
}

void MainWindow::initMode()
{
    QString mode = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EASY);
    connect(ui->menuMode, &QMenu::triggered, this, &MainWindow::menuModeSelect);
    const QList<QAction *> actions = ui->menuMode->actions();
    for (QAction *act : actions) {
        act->setCheckable(true);
        if (act->text() == mode) {
            menuModeSelect(act);
        }
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
        painter.fillRect(rect(), QColor(255, 255, 255, 180));
    }
}

void MainWindow::setLanguage(const QString &language)
{
    qApp->setProperty("language", language);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_LANGUAGE, language);

    QTranslator *translator = new QTranslator(this);
    if (translator->load(QString(":/res/i18n/%1.qm").arg(language))) {
        qApp->installTranslator(translator);
        ui->retranslateUi(this);
        if (m_formEasy) {
            m_formEasy->retranslateUI();
        }
        if (m_formExpert) {
            m_formExpert->retranslateUI();
        }
    }
}

void MainWindow::setTheme(const QString &theme)
{
    qApp->setProperty("theme", theme);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_THEME, theme);

    QFile file(QString(":/res/themes/%1.qss").arg(theme));
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString style = file.readAll();
        qApp->setStyleSheet(style);
    }
    if (theme == "HelloKitty") {
        m_background = QPixmap(":/res/themes/background/HelloKitty.jpg");
    }
}

void MainWindow::setAlgorithm(const QString &algorithm)
{
    qApp->setProperty("algorithm", algorithm);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_ALGORITHM, algorithm);

    m_formEasy->setAlgorithm(algorithm);
    m_formExpert->setAlgorithm(algorithm);
}

void MainWindow::setMode(const QString &mode)
{
    qApp->setProperty("mode", mode);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, mode);

    if (mode == CFG_MODE_EXPERT) {
        ui->stackedWidgetMode->setCurrentWidget(m_formExpert);
    } else if (mode == CFG_MODE_EASY) {
        ui->stackedWidgetMode->setCurrentWidget(m_formEasy);
    }
}

void MainWindow::initStackWidget()
{
    m_formEasy = new FormEasy;
    m_formExpert = new FormExpert;
    ui->stackedWidgetMode->addWidget(m_formEasy);
    ui->stackedWidgetMode->addWidget(m_formExpert);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_formEasy->close();
    m_formExpert->close();
}

void MainWindow::menuLanguageSelect(QAction *selectedAction)
{
    QString language = selectedAction->text();
    setLanguage(language);
    const QList<QAction *> actions = ui->menuLanguage->actions();
    for (QAction *act : actions) {
        if (act == selectedAction) {
            act->setChecked(true);
        } else {
            act->setChecked(false);
        }
    }
}

void MainWindow::menuThemeSelect(QAction *selectedTheme)
{
    QString theme = selectedTheme->text();
    setTheme(theme);
    const QList<QAction *> actions = ui->menuTheme->actions();
    for (QAction *act : actions) {
        if (act == selectedTheme) {
            act->setChecked(true);
        } else {
            act->setChecked(false);
        }
    }
}

void MainWindow::menuAlgorithmSelect(QAction *selectedAlgorithm)
{
    QString algorithm = selectedAlgorithm->text();
    setAlgorithm(algorithm);
    const QList<QAction *> actions = ui->menuAlgorithm->actions();
    for (QAction *act : actions) {
        if (act == selectedAlgorithm) {
            act->setChecked(true);
        } else {
            act->setChecked(false);
        }
    }
}

void MainWindow::menuModeSelect(QAction *selectedMode)
{
    QString mode = selectedMode->text();
    setMode(mode);
    const QList<QAction *> actions = ui->menuMode->actions();
    for (QAction *act : actions) {
        if (act == selectedMode) {
            act->setChecked(true);
        } else {
            act->setChecked(false);
        }
    }
}

void MainWindow::on_actionEasy_triggered()
{
    LOG_INFO("software mode: Easy");
    ui->stackedWidgetMode->setCurrentWidget(m_formEasy);
    ui->actionEasy->setChecked(true);
    ui->actionExpert->setChecked(false);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EASY);
}

void MainWindow::on_actionExpert_triggered()
{
    LOG_INFO("software mode: Expert");
    ui->stackedWidgetMode->setCurrentWidget(m_formExpert);
    ui->actionEasy->setChecked(false);
    ui->actionExpert->setChecked(true);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EXPERT);
}

void MainWindow::init()
{
    initMsgBar();
    initStackWidget();
    initTheme();
    initLanguage();
    initMode();
    initAlgorithm();
}
