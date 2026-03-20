#include "mainwindow.h"

#include <QDesktopServices>
#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QTranslator>

#include "../mode/FormEasy/formeasy.h"
#include "../mode/FormExpert/formexpert.h"
#include "../mode/FormProduce/formproduce.h"
#include "../form/setting/formsetting.h"
#include "../form/AutoUpdate/autoupdate.h"
#include "./ui_mainwindow.h"
#include "funcdef.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::initMsgBar() {
    QLabel *linkLabel = new QLabel(this);
    linkLabel->setText(QString("version: %1").arg(APP_VERSION));
    linkLabel->setTextFormat(Qt::RichText);
    linkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    linkLabel->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(linkLabel);
}

void MainWindow::initTheme() {
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

void MainWindow::initLanguage() {
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

void MainWindow::initAlgorithm() {
    QString algorithm = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_ALGORITHM, "Freedom");
    connect(ui->menuAlgorithm, &QMenu::triggered, this, &MainWindow::menuAlgorithmSelect);
    const QList<QAction *> actions = ui->menuAlgorithm->actions();
    for (QAction *act : actions) {
        act->setCheckable(true);
        if (COMPARE_CaseInsensitive(act->text(), algorithm)) {
            menuAlgorithmSelect(act);
        }
    }
}

void MainWindow::initMode() {
    QString mode = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EASY);
    connect(ui->menuMode, &QMenu::triggered, this, &MainWindow::menuModeSelect);
    const QList<QAction *> actions = ui->menuMode->actions();
    for (QAction *act : actions) {
        act->setCheckable(true);
        if (COMPARE_CaseInsensitive(act->text(), mode)) {
            menuModeSelect(act);
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QMainWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillRect(rect(), Qt::transparent);

    if (!m_background.isNull()) {
        QPixmap scaled = m_background.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
        painter.fillRect(rect(), QColor(255, 255, 255, 180));
    }
}

void MainWindow::setLanguage(const QString &language) {
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
        if (m_formProduce) {
            m_formProduce->retranslateUI();
        }
        if(m_formSetting) {
            m_formSetting->retranslateUI();
        }
    }
}

void MainWindow::setTheme(const QString &theme) {
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

void MainWindow::setAlgorithm(const QString &algorithm) {
    qApp->setProperty("algorithm", algorithm);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_ALGORITHM, algorithm);
    const QList<QAction *> actions = ui->menuAlgorithm->actions();
    for (QAction *act : actions) {
        if (COMPARE_CaseInsensitive(act->text(), algorithm)) {
            act->setChecked(true);
        } else {
            act->setChecked(false);
        }
    }
    if (m_formEasy) {
        m_formEasy->setAlgorithm(algorithm);
    }
    if (m_formExpert) {
        m_formExpert->setAlgorithm(algorithm);
    }
    if (m_formProduce) {
        m_formProduce->setAlgorithm(algorithm);
    }
}

void MainWindow::setCli()
{
    ui->menuAlgorithm->menuAction()->setVisible(false);
    ui->menuMode->menuAction()->setVisible(false);
}

void MainWindow::safeDelete(QWidget *&w) {
    if (!w) return;

    ui->stackedWidgetMode->removeWidget(w);
    w->deleteLater();
    w = nullptr;
}

void MainWindow::setMode(const QString &mode) {
    qApp->setProperty("mode", mode);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, mode);

    const QList<QAction *> actions = ui->menuMode->actions();
    for (QAction *act : actions) {
        if (act->text().toLower() == mode) {
            act->setChecked(true);
        } else {
            act->setChecked(false);
        }
    }
    if (mode == CFG_MODE_EASY) {
        if (!m_formEasy) {
            m_formEasy = new FormEasy;
            ui->stackedWidgetMode->addWidget(m_formEasy);
            connect(m_formEasy, &FormEasy::initThreshold, m_formSetting, &FormSetting::initThreshold);
            connect(m_formSetting, &FormSetting::sendThreshold, m_formEasy, &FormEasy::recvThreshold);
            connect(m_formSetting, &FormSetting::sendThresholdOption, m_formEasy, &FormEasy::recvThresholdOption);
        }
        ui->stackedWidgetMode->setCurrentWidget(m_formEasy);

        safeDelete((QWidget *&)m_formExpert);
        safeDelete((QWidget *&)m_formProduce);
    } else if (mode == CFG_MODE_EXPERT) {
        if (!m_formExpert) {
            m_formExpert = new FormExpert;
            ui->stackedWidgetMode->addWidget(m_formExpert);
        }
        ui->stackedWidgetMode->setCurrentWidget(m_formExpert);

        safeDelete((QWidget *&)m_formEasy);
        safeDelete((QWidget *&)m_formProduce);
    } else if (mode == CFG_MODE_PRODUCE) {
        if (!m_formProduce) {
            m_formProduce = new FormProduce;
            ui->stackedWidgetMode->addWidget(m_formProduce);
        }
        ui->stackedWidgetMode->setCurrentWidget(m_formProduce);

        safeDelete((QWidget *&)m_formEasy);
        safeDelete((QWidget *&)m_formExpert);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_formEasy) {
        m_formEasy->close();
    }
    if (m_formExpert) {
        m_formExpert->close();
    }
    if (m_formProduce) {
        m_formProduce->close();
    }
    if(m_formSetting) {
        m_formSetting->close();
    }
}

void MainWindow::menuLanguageSelect(QAction *selectedAction) {
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

void MainWindow::menuThemeSelect(QAction *selectedTheme) {
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

void MainWindow::menuAlgorithmSelect(QAction *selectedAlgorithm) {
    QString algorithm = selectedAlgorithm->text();
    setAlgorithm(algorithm);
}

void MainWindow::menuModeSelect(QAction *selectedMode) {
    QString mode = selectedMode->text().toLower();
    setMode(mode);
}

void MainWindow::on_actionEasy_triggered() {
    LOG_INFO("software mode: Easy");
    ui->stackedWidgetMode->setCurrentWidget(m_formEasy);
    ui->actionEasy->setChecked(true);
    ui->actionExpert->setChecked(false);
    ui->actionProduce->setChecked(false);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EASY);
}

void MainWindow::on_actionExpert_triggered() {
    LOG_INFO("software mode: Expert");
    ui->stackedWidgetMode->setCurrentWidget(m_formExpert);
    ui->actionExpert->setChecked(true);
    ui->actionEasy->setChecked(false);
    ui->actionProduce->setChecked(false);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EXPERT);
}

void MainWindow::on_actionProduce_triggered() {
    LOG_INFO("software mode: Produce");
    ui->stackedWidgetMode->setCurrentWidget(m_formProduce);
    ui->actionProduce->setChecked(true);
    ui->actionEasy->setChecked(false);
    ui->actionExpert->setChecked(false);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_PRODUCE);
}

void MainWindow::init() {
    m_formSetting = new FormSetting;
    connect(m_formSetting, &FormSetting::fullyControl, this, [=](bool isFully){
        ui->menuAlgorithm->menuAction()->setVisible(isFully);
        ui->menuMode->menuAction()->setVisible(isFully);
    });
    connect(m_formSetting, &FormSetting::sendDouble, this, [=](bool isDouble){
        if(isDouble) {
            setAlgorithm(CFG_ALGORITHM_F30_CURVES);
        }
        else {
            setAlgorithm(CFG_ALGORITHM_F30_SINGLE);
        }
    });
    connect(m_formSetting, &FormSetting::windowClose, this, [=]() {
        m_enableSetting = false;
        m_formSetting->setVisible(false);
    });

    initMsgBar();
    initMode();
    initTheme();
    initLanguage();
    initAlgorithm();
}

void MainWindow::on_actionSetting_triggered()
{
    m_enableSetting = !m_enableSetting;
    m_formSetting->setVisible(m_enableSetting);
}

void MainWindow::on_actionUpdate_triggered()
{
    AutoUpdate *upt = new AutoUpdate;
    upt->show();
}

void MainWindow::on_actionUser_Guide_triggered()
{
    QString pdfPath = QCoreApplication::applicationDirPath() + QString("/document/document_%1.pdf").arg(qApp->property("mode").toString().toLower());

    if (!QFile::exists(pdfPath)) {
        QMessageBox::critical(this, TITLE_ERROR, tr("Unable to find document: %1").arg(pdfPath));
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath));
}

