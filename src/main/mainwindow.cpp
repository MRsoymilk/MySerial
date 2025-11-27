#include "mainwindow.h"
#include "../form/AutoUpdate/autoupdate.h"
#include "../form/FormPlotCorrection/formplotcorrection.h"
#include "../form/FormPlotData/formplotdata.h"
#include "../form/FormPlotHistory/formplothistory.h"
#include "../form/FormPlotSimulate/formplotsimulate.h"
#include "../form/data/formdata.h"
#include "../form/log/formlog.h"
#include "../form/play/formplaympu6050.h"
#include "../form/plot/formplot.h"
#include "../form/serial/formserial.h"
#include "../form/setting/formsetting.h"
#include "./ui_mainwindow.h"
#include "funcdef.h"
#include "threadworker.h"
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
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
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

    formAutoUpdate = new AutoUpdate(this);
    ui->stackedWidget->addWidget(formAutoUpdate);

    playMPU6050 = new FormPlayMPU6050(this);
    ui->stackedWidget->addWidget(playMPU6050);

    QObject::connect(formSerial, &FormSerial::recv2PlotLLC, formPlot, &FormPlot::onDataReceivedLLC);
    QObject::connect(formSerial, &FormSerial::recv2PlotF30, formPlot, &FormPlot::onDataReceivedF30);
    QObject::connect(formSerial, &FormSerial::recv2DataF30, formData, &FormData::onDataReceivedF30);
    QObject::connect(formSerial, &FormSerial::recv2PlotF15, formPlot, &FormPlot::onDataReceivedF15);
    QObject::connect(formSerial, &FormSerial::recv2DataF15, formData, &FormData::onDataReceivedF15);
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
        ui->btnUpdate,
        ui->btnSetting,
    };

    QMap<QToolButton *, QString> onIcons = {
        {ui->btnSerial, "usb-on"},
        {ui->btnData, "data-on"},
        {ui->btnLog, "log-on"},
        {ui->btnPlot, "plot-on"},
        {ui->btnUpdate, "update-on"},
        {ui->btnSetting, "setting-on"},
    };

    QMap<QToolButton *, QString> offIcons = {
        {ui->btnSerial, "usb-off"},
        {ui->btnData, "data-off"},
        {ui->btnLog, "log-off"},
        {ui->btnPlot, "plot-off"},
        {ui->btnUpdate, "update-off"},
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

    ui->tBtnData->setObjectName("data");
    ui->tBtnData->setIconSize(QSize(24, 24));
    ui->tBtnData->setCheckable(true);
    ui->tBtnData->setChecked(m_showData);
    ui->tBtnData->setToolTip(tr("Data"));
    ui->tBtnData->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnHistory->setObjectName("history");
    ui->tBtnHistory->setIconSize(QSize(24, 24));
    ui->tBtnHistory->setCheckable(true);
    ui->tBtnHistory->setChecked(m_showHistory);
    ui->tBtnHistory->setToolTip(tr("History"));
    ui->tBtnHistory->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnSimulate->setObjectName("simulate");
    ui->tBtnSimulate->setIconSize(QSize(24, 24));
    ui->tBtnSimulate->setCheckable(true);
    ui->tBtnSimulate->setChecked(m_showSimulate);
    ui->tBtnSimulate->setToolTip(tr("Simulate"));
    ui->tBtnSimulate->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->tBtnCorrection->setObjectName("correction");
    ui->tBtnCorrection->setIconSize(QSize(24, 24));
    ui->tBtnCorrection->setCheckable(true);
    ui->tBtnCorrection->setChecked(m_showCorrection);
    ui->tBtnCorrection->setToolTip(tr("Correction"));
    ui->tBtnCorrection->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

void MainWindow::initEasyMode()
{
    m_line = new QLineSeries();

    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    m_chart = new QChart();
    m_chart->addSeries(m_line);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_line->attachAxis(m_axisX);
    m_line->attachAxis(m_axisY);
    m_axisX->setTitleText(tr("index"));
    m_axisY->setTitleText(tr("intensity"));
    m_chart->setTitle(tr("wavelength"));
    m_chartView = new MyChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(m_chartView);

    m_modelValue = new QStandardItemModel(this);
    m_modelValue->setColumnCount(3);
    m_modelValue->setHeaderData(0, Qt::Horizontal, tr("index"));
    m_modelValue->setHeaderData(1, Qt::Horizontal, tr("intensity"));
    m_modelValue->setHeaderData(2, Qt::Horizontal, tr("raw"));

    ui->tableViewValue->setModel(m_modelValue);
    ui->tableViewValue->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewValue->verticalHeader()->setVisible(false);
    ui->tableViewValue->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewValue->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->spinBox->setValue(5);

    ui->tBtnSwitch->setToolTip("switch");
    ui->tBtnPause->setToolTip("pause");
    ui->tBtnZoom->setToolTip("zoom");
    ui->tBtnCrop->setToolTip("crop");
    ui->tBtnPeak->setToolTip("find peak");
    ui->tBtnFWHM->setToolTip("FWHM");
    ui->tBtnImg->setToolTip("save image");

    const QString mode = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EXPERT);
    if (mode == CFG_MODE_EXPERT) {
        on_actionExpert_triggered();
    } else {
        on_actionEasy_triggered();
    }

    ui->tBtnZoom->setChecked(true);

    m_peaks = new QScatterSeries();
    m_chart->addSeries(m_peaks);
    m_peaks->attachAxis(m_axisX);
    m_peaks->attachAxis(m_axisY);
    m_peaks->setColor(Qt::red);
    m_peaks->setName(tr("Peaks"));
    m_peaks->setMarkerSize(5.0);
    m_peaks->setPointLabelsVisible(true);
    m_peaks->setPointLabelsClipping(false);
    m_peaks->setPointLabelsColor(Qt::red);
    m_peaks->setPointLabelsFont(QFont("Arial", 10, QFont::Bold));
    m_peaks->setPointLabelsFormat("(@xPoint, @yPoint)");
}
#include "plot_algorithm.h"
bool MainWindow::connectEasyMode()
{
    if (formSerial->startEasyConnect()) {
        formSerial->setEasyFrame();
        m_worker->setAlgorithm(static_cast<int>(SHOW_ALGORITHM::F15_SINGLE));
        formSerial->writeEasyData(calcIntegrationTime(ui->spinBox->value()));
        formSerial->writeEasyData("DD3C000330CDFF");
        return true;
    } else {
        LOG_WARN("connect failed!");
        return false;
    }
}

void MainWindow::closeEasyMode()
{
    formSerial->writeEasyData("DD3C000360CDFF");
    formSerial->stopEasyConnect();
}

void MainWindow::init()
{
    initMsgBar();
    initStackWidget();
    initToolbar();
    initLanguage();
    initTheme();
    initEasyMode();

    m_plotData = new FormPlotData;
    m_plotData->hide();

    m_plotHistory = new FormPlotHistory;
    m_plotHistory->hide();

    m_plotSimulate = new FormPlotSimulate;
    m_plotSimulate->hide();

    m_plotCorrection = new FormPlotCorrection;
    m_plotCorrection->hide();

    // thread worker
    m_workerThread = new QThread(this);
    m_worker = new ThreadWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_plotSimulate,
            &FormPlotSimulate::simulateDataReady,
            formSerial,
            &FormSerial::onSimulateRecv,
            Qt::QueuedConnection);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(formPlot,
            &FormPlot::newDataReceivedF15,
            m_worker,
            &ThreadWorker::processDataF15,
            Qt::QueuedConnection);
    connect(formPlot,
            &FormPlot::newDataReceivedF30,
            m_worker,
            &ThreadWorker::processDataF30,
            Qt::QueuedConnection);
    connect(formPlot,
            &FormPlot::newDataReceivedLLC,
            m_worker,
            &ThreadWorker::processDataLLC,
            Qt::QueuedConnection);

    connect(m_plotHistory, &FormPlotHistory::sendToPlot, formPlot, &FormPlot::updatePlot4k);
    connect(m_worker,
            &ThreadWorker::plotReady4k,
            formPlot,
            &FormPlot::updatePlot4k,
            Qt::QueuedConnection);
    connect(m_worker,
            &ThreadWorker::dataReady4k,
            m_plotData,
            &FormPlotData::updateTable4k,
            Qt::QueuedConnection);
    connect(m_worker,
            &ThreadWorker::plotReady4k,
            this,
            &MainWindow::updatePlot,
            Qt::QueuedConnection);
    connect(m_worker,
            &ThreadWorker::dataReady4k,
            this,
            &MainWindow::updateTable,
            Qt::QueuedConnection);

    m_workerThread->start();

    connect(m_plotData, &FormPlotData::windowClose, this, &MainWindow::plotDataClose);
    connect(m_plotHistory, &FormPlotHistory::windowClose, this, &MainWindow::plotHistoryClose);
    connect(formPlot, &FormPlot::toHistory, m_plotHistory, &FormPlotHistory::onHistoryRecv);
    connect(m_plotSimulate, &FormPlotSimulate::windowClose, this, &MainWindow::plotSimulateClose);
    connect(m_plotCorrection,
            &FormPlotCorrection::windowClose,
            this,
            &MainWindow::plotCorrectionClose);
    connect(m_plotCorrection,
            &FormPlotCorrection::useLoadedThreshold,
            m_worker,
            &ThreadWorker::onUseLoadedThreshold,
            Qt::QueuedConnection);
    connect(m_plotCorrection,
            &FormPlotCorrection::useLoadedThreadsholdOption,
            m_worker,
            &ThreadWorker::onUseLoadedThreadsholdOption,
            Qt::QueuedConnection);
    connect(formSerial,
            &FormSerial::recvTemperature,
            m_plotCorrection,
            &FormPlotCorrection::onTemperature);
    connect(m_worker,
            &ThreadWorker::showCorrectionCurve,
            m_plotCorrection,
            &FormPlotCorrection::onShowCorrectionCurve);
    connect(formSerial,
            &FormSerial::recvTemperature,
            m_plotHistory,
            &FormPlotHistory::onTemperature);
    connect(formPlot,
            &FormPlot::changeFrameType,
            m_plotSimulate,
            &FormPlotSimulate::onChangeFrameType);
    connect(formPlot, &FormPlot::changeFrameType, this, [&](int index) {
        m_worker->setAlgorithm(index);
    });
    connect(formPlot, &FormPlot::sendOffset31, this, [&](int val) { m_worker->setOffset31(val); });
    connect(formPlot, &FormPlot::sendOffset33, this, [&](int val) { m_worker->setOffset33(val); });
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

void MainWindow::on_btnUpdate_clicked()
{
    ui->stackedWidget->setCurrentWidget(formAutoUpdate);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_plotCorrection->close();
    m_plotData->close();
    m_plotHistory->close();
    m_plotSimulate->close();
    formPlot->close();
    formData->close();
    formLog->close();
    formAutoUpdate->close();
    formSerial->close();
    formSetting->close();
    event->accept();
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
        if (m_plotCorrection) {
            m_plotCorrection->retranslateUI();
        }
        if (m_plotSimulate) {
            m_plotSimulate->retranslateUI();
        }
        if (m_plotHistory) {
            m_plotHistory->retranslateUI();
        }
        if (m_plotData) {
            m_plotData->retranslateUI();
        }
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
        if (formAutoUpdate) {
            formAutoUpdate->retranslateUI();
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
        } else {
            act->setChecked(false);
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
        } else {
            act->setChecked(false);
        }
    }
}

void MainWindow::on_tBtnData_clicked()
{
    m_showData = !m_showData;
    m_plotData->setVisible(m_showData);
}

void MainWindow::plotDataClose()
{
    m_showData = false;
    ui->tBtnData->setChecked(false);
}

void MainWindow::plotHistoryClose()
{
    m_showHistory = false;
    ui->tBtnHistory->setChecked(false);
}

void MainWindow::plotSimulateClose()
{
    m_showSimulate = false;
    ui->tBtnSimulate->setChecked(false);
}

void MainWindow::plotCorrectionClose()
{
    m_showCorrection = false;
    ui->tBtnCorrection->setChecked(false);
    disconnect(m_worker,
               &ThreadWorker::dataReady4k,
               m_plotCorrection,
               &FormPlotCorrection::onEpochCorrection);
}

void MainWindow::on_tBtnHistory_clicked()
{
    m_showHistory = !m_showHistory;
    m_plotHistory->setVisible(m_showHistory);
}

void MainWindow::on_tBtnSimulate_clicked()
{
    m_showSimulate = !m_showSimulate;
    m_plotSimulate->setVisible(m_showSimulate);
}

void MainWindow::on_tBtnCorrection_clicked()
{
    m_showCorrection = !m_showCorrection;
    m_plotCorrection->setVisible(m_showCorrection);
    if (m_showCorrection) {
        connect(m_worker,
                &ThreadWorker::dataReady4k,
                m_plotCorrection,
                &FormPlotCorrection::onEpochCorrection,
                Qt::QueuedConnection);
    }
}

void MainWindow::on_actionEasy_triggered()
{
    LOG_INFO("software mode: Easy");
    ui->stackedWidgetMode->setCurrentWidget(ui->pageEasy);
    ui->actionEasy->setChecked(true);
    ui->actionExpert->setChecked(false);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EASY);
}

void MainWindow::on_actionExpert_triggered()
{
    LOG_INFO("software mode: Expert");
    ui->stackedWidgetMode->setCurrentWidget(ui->pageExpert);
    ui->actionEasy->setChecked(false);
    ui->actionExpert->setChecked(true);
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_MODE, CFG_MODE_EXPERT);
}

void MainWindow::on_tBtnSwitch_clicked()
{
    if (!m_movie) {
        m_movie = new QMovie(":/res/icons/start.gif", QByteArray(), this);

        connect(m_movie, &QMovie::frameChanged, this, [this]() {
            ui->tBtnSwitch->setIcon(QIcon(m_movie->currentPixmap()));
        });
    }

    if (!m_isPlaying) {
        if (connectEasyMode()) {
            m_movie->start();
            m_isPlaying = true;
        }
    } else {
        m_movie->stop();
        m_isPlaying = false;
        ui->tBtnSwitch->setIcon(QIcon(":/res/icons/start.png"));
        closeEasyMode();
    }
}

void MainWindow::on_tBtnZoom_clicked()
{
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);
}

void MainWindow::on_tBtnCrop_clicked()
{
    m_enableCrop = !m_enableCrop;
    ui->tBtnCrop->setChecked(m_enableCrop);
    if (m_enableCrop) {
        m_chartView->setCropEnabled(true);
        m_chartView->recordInitialAxisRange();
    } else {
        m_chartView->setCropEnabled(false);
        m_chartView->backInitialRange();
    }
}

void MainWindow::on_tBtnPeak_clicked()
{
    m_findPeak = !m_findPeak;
    ui->tBtnPeak->setChecked(m_findPeak);
    callFindPeak();
}

void MainWindow::on_tBtnFWHM_clicked()
{
    m_calcFWHM = !m_calcFWHM;
    ui->tBtnFWHM->setChecked(m_calcFWHM);
    callCalcFWHM();
}

void MainWindow::saveChartAsImage(const QString &filePath)
{
    if (!m_chartView)
        return;

    QSize size = m_chartView->size();

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    m_chartView->render(&painter);
    painter.end();

    image.save(filePath);
}

void MainWindow::updatePlot(const CURVE &curve31,
                            const CURVE &curve33,
                            const double &temperature,
                            bool record)
{
    if (m_pause) {
        return;
    }
    m_line->replace(curve31.data);
    if (m_autoZoom) {
        m_axisX->setRange(curve31.x_min, curve31.x_max);
        m_axisY->setRange(curve31.y_min, curve31.y_max);
    }
    callFindPeak();
    callCalcFWHM();
}

void MainWindow::updateTable(const QVector<double> &v14,
                             const QVector<double> &v24,
                             const QVector<double> &raw14,
                             const QVector<double> &raw24)
{
    if (m_pause) {
        return;
    }
    if (m_modelValue->rowCount() > 0) {
        m_modelValue->removeRows(0, m_modelValue->rowCount());
    }
    int count = qMax(qMax(v14.size(), v24.size()), qMax(raw14.size(), raw24.size()));
    for (int i = 0; i < count; ++i) {
        QString index = QString::number(i);
        QString yV24 = (i < v24.size()) ? QString::number(v24[i]) : "";
        QString yR24 = (i < raw24.size()) ? QString::number(raw24[i]) : "";

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(index);
        rowItems << new QStandardItem(yV24);
        rowItems << new QStandardItem(yR24);
        m_modelValue->appendRow(rowItems);
    }
}

QVector<QPointF> MainWindow::findPeak(int window, double thresholdFactor, double minDist)
{
    QVector<QPointF> peaks;
    int n = m_line->count();

    QVector<double> values;
    values.reserve(n);
    for (int i = 0; i < n; i++)
        values.append(m_line->at(i).y());

    double mean = std::accumulate(values.begin(), values.end(), 0.0) / n;
    double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / n - mean * mean);
    double threshold = mean + thresholdFactor * stdev;

    double lastPeakX = -1e9;
    for (int i = window; i < n - window; i++) {
        double yCurr = m_line->at(i).y();
        if (yCurr < threshold)
            continue;

        bool isPeak = true;
        for (int j = i - window; j <= i + window; j++) {
            if (m_line->at(j).y() > yCurr) {
                isPeak = false;
                break;
            }
        }

        if (isPeak) {
            double xCurr = m_line->at(i).x();
            if (xCurr - lastPeakX >= minDist) {
                peaks.append(m_line->at(i));
                lastPeakX = xCurr;
            }
        }
    }
    return peaks;
}

void MainWindow::callFindPeak()
{
    if (m_findPeak) {
        if (!m_line || m_line->count() < 5) {
            return;
        }

        QVector<QPointF> peaks24 = findPeak(3, 1.0, 5.0);

        m_peaks->clear();
        for (const auto &pt : peaks24) {
            m_peaks->append(pt);
        }

        m_chart->update();
    } else {
        m_peaks->clear();
    }
}

void MainWindow::callCalcFWHM()
{
    if (m_calcFWHM) {
        for (auto *line : m_fwhmLines) {
            m_chart->removeSeries(line);
            delete line;
        }
        m_fwhmLines.clear();
        for (auto *label : m_fwhmLabels) {
            delete label;
        }
        m_fwhmLabels.clear();

        auto peaks = findPeak(3, 1.0, 5.0);
        if (peaks.isEmpty())
            return;

        for (const auto &peak : peaks) {
            double yPeak = peak.y();
            double xPeak = peak.x();
            double yHalf = yPeak / 2.0;

            double xLeft = xPeak, xRight = xPeak;
            for (int i = m_line->count() - 1; i >= 1; --i) {
                if (m_line->at(i).x() >= xPeak)
                    continue;
                double y1 = m_line->at(i).y();
                double y2 = m_line->at(i - 1).y();
                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_line->at(i).x();
                    double x2 = m_line->at(i - 1).x();
                    // 线性插值
                    xLeft = x1 + (yHalf - y1) * (x2 - x1) / (y2 - y1);
                    break;
                }
            }
            for (int i = 0; i < m_line->count() - 1; ++i) {
                if (m_line->at(i).x() <= xPeak)
                    continue;
                double y1 = m_line->at(i).y();
                double y2 = m_line->at(i + 1).y();
                if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
                    double x1 = m_line->at(i).x();
                    double x2 = m_line->at(i + 1).x();
                    // 线性插值
                    xRight = x1 + (yHalf - y1) * (x2 - x1) / (y2 - y1);
                    break;
                }
            }
            double fwhm = xRight - xLeft;

            QLineSeries *fwhmLine = new QLineSeries();
            fwhmLine->setColor(Qt::red);
            fwhmLine->setName(QString("FWHM %1").arg(xPeak, 0, 'f', 1));
            fwhmLine->append(xLeft, yHalf);
            fwhmLine->append(xRight, yHalf);
            m_chart->addSeries(fwhmLine);
            fwhmLine->attachAxis(m_axisX);
            fwhmLine->attachAxis(m_axisY);
            m_fwhmLines.append(fwhmLine);

            QPointF mid((xLeft + xRight) / 2.0, yHalf);
            QPointF scenePos = m_chart->mapToPosition(mid, fwhmLine);

            auto *label = new QGraphicsSimpleTextItem(QString("FWHM=%1").arg(fwhm, 0, 'f', 2),
                                                      m_chart);
            label->setBrush(Qt::red);
            label->setPos(scenePos + QPointF(5, -15));
            m_fwhmLabels.append(label);
        }
    } else {
        for (auto *line : m_fwhmLines) {
            m_chart->removeSeries(line);
            delete line;
        }
        m_fwhmLines.clear();
        for (auto *label : m_fwhmLabels) {
            delete label;
        }
        m_fwhmLabels.clear();
    }
}

QString MainWindow::calcIntegrationTime(int value)
{
    int rawValue = value * 5;
    QString hex = QString("%1").arg(rawValue, 6, 16, QLatin1Char('0')).toUpper();
    QString prefix = "DD3C000622";
    QString suffix = "CDFF";
    return prefix + hex + suffix;
}

void MainWindow::on_tBtnImg_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save Chart"),
                                                    "",
                                                    "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (!filePath.isEmpty()) {
        saveChartAsImage(filePath);
    }
}

void MainWindow::on_spinBox_valueChanged(int val)
{
    if (m_isPlaying) {
        formSerial->writeEasyData(calcIntegrationTime(ui->spinBox->value()));
    }
}

void MainWindow::on_tBtnPause_clicked()
{
    m_pause = !m_pause;
    ui->tBtnPause->setChecked(m_pause);
}
