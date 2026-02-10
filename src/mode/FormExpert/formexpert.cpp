#include "formexpert.h"
#include "../ThreadWorker/threadworker.h"
#include "../form/AutoUpdate/autoupdate.h"
#include "../form/FormExternal/formexternal.h"
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
#include "ui_formexpert.h"

FormExpert::FormExpert(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormExpert)
{
    ui->setupUi(this);
    init();
}

FormExpert::~FormExpert()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
    delete ui;
}

void FormExpert::retranslateUI()
{
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
    if (formExternal) {
        formExternal->retranslateUI();
    }
}

void FormExpert::setAlgorithm(const QString &algorithm)
{
    formSerial->onChangeFrameType(algorithm);
    formPlot->setAlgorithm(algorithm);
    m_worker->setAlgorithm(algorithm);
}

void FormExpert::initToolbar()
{
    QList<QToolButton *> buttonList = {
        ui->btnSerial,
        ui->btnData,
        ui->btnLog,
        ui->btnPlot,
        ui->btnUpdate,
        ui->btnSetting,
        ui->btnExternal,
    };

    QMap<QToolButton *, QString> onIcons = {
        {ui->btnSerial, "usb-on"},
        {ui->btnData, "data-on"},
        {ui->btnLog, "log-on"},
        {ui->btnPlot, "plot-on"},
        {ui->btnUpdate, "update-on"},
        {ui->btnSetting, "setting-on"},
        {ui->btnExternal, "external-on"},
    };

    QMap<QToolButton *, QString> offIcons = {
        {ui->btnSerial, "usb-off"},
        {ui->btnData, "data-off"},
        {ui->btnLog, "log-off"},
        {ui->btnPlot, "plot-off"},
        {ui->btnUpdate, "update-off"},
        {ui->btnSetting, "setting-off"},
        {ui->btnExternal, "external-off"},
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

void FormExpert::initStackWidget()
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

    formExternal = new FormExternal(this);
    ui->stackedWidget->addWidget(formExternal);

    formAutoUpdate = new AutoUpdate(this);
    ui->stackedWidget->addWidget(formAutoUpdate);

    playMPU6050 = new FormPlayMPU6050(this);
    ui->stackedWidget->addWidget(playMPU6050);

    ui->stackedWidget->setCurrentWidget(formSerial);

    QShortcut *shortcut_Serial = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_1), this);
    QShortcut *shortcut_Data = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_2), this);
    QShortcut *shortcut_Plot = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_3), this);
    QShortcut *shortcut_Log = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_4), this);
    QShortcut *shortcut_Setting = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_5), this);
    QShortcut *shortcut_Play = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_6), this);
    QShortcut *shortcut_External = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_7), this);
    connect(shortcut_Serial, &QShortcut::activated, this, [this]() { ui->btnSerial->click(); });
    connect(shortcut_Data, &QShortcut::activated, this, [this]() { ui->btnData->click(); });
    connect(shortcut_Plot, &QShortcut::activated, this, [this]() { ui->btnPlot->click(); });
    connect(shortcut_Log, &QShortcut::activated, this, [this]() { ui->btnLog->click(); });
    connect(shortcut_Setting, &QShortcut::activated, this, [this]() { ui->btnSetting->click(); });
    connect(shortcut_Play, &QShortcut::activated, this, [this]() {
        ui->stackedWidget->setCurrentWidget(playMPU6050);
    });
    connect(shortcut_External, &QShortcut::activated, this, [this]() { ui->btnExternal->click(); });
}

void FormExpert::on_tBtnData_clicked()
{
    m_showData = !m_showData;
    m_plotData->setVisible(m_showData);
}

void FormExpert::plotDataClose()
{
    m_showData = false;
    ui->tBtnData->setChecked(false);
}

void FormExpert::plotHistoryClose()
{
    m_showHistory = false;
    ui->tBtnHistory->setChecked(false);
}

void FormExpert::plotSimulateClose()
{
    m_showSimulate = false;
    ui->tBtnSimulate->setChecked(false);
}

void FormExpert::plotCorrectionClose()
{
    m_showCorrection = false;
    ui->tBtnCorrection->setChecked(false);
    disconnect(m_worker,
               &ThreadWorker::dataReady4k,
               m_plotCorrection,
               &FormPlotCorrection::onEpochCorrection);
}

void FormExpert::on_tBtnHistory_clicked()
{
    m_showHistory = !m_showHistory;
    m_plotHistory->setVisible(m_showHistory);
}

void FormExpert::on_tBtnSimulate_clicked()
{
    m_showSimulate = !m_showSimulate;
    m_plotSimulate->setVisible(m_showSimulate);
}

void FormExpert::on_tBtnCorrection_clicked()
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

void FormExpert::closeEvent(QCloseEvent *event)
{
    m_plotCorrection->close();
    m_plotData->close();
    m_plotHistory->close();
    m_plotSimulate->close();
    formPlot->close();
    formData->close();
    formLog->close();
    formAutoUpdate->close();
    formExternal->close();
    formSerial->close();
    formSetting->close();
}

void FormExpert::init()
{
    initStackWidget();
    initToolbar();

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
    connect(m_plotSimulate, &FormPlotSimulate::simulateReset, formSerial, &FormSerial::clearData);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(formPlot,
            &FormPlot::newDataReceivedF15,
            m_worker,
            &ThreadWorker::processDataF15,
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
            &ThreadWorker::collectionFitingPointsFinish,
            m_plotCorrection,
            &FormPlotCorrection::onCollectionFitingPointsFinish,
            Qt::QueuedConnection);
    connect(m_plotCorrection,
            &FormPlotCorrection::toCollectionFittingPoints,
            m_worker,
            &ThreadWorker::onCollectionFittingPoints,
            Qt::QueuedConnection);
    connect(m_plotCorrection,
            &FormPlotCorrection::doFile,
            m_plotSimulate,
            &FormPlotSimulate::onDoFile,
            Qt::QueuedConnection);

    m_workerThread->start();

    connect(m_plotData,
            &FormPlotData::windowClose,
            this,
            &FormExpert::plotDataClose,
            Qt::QueuedConnection);
    connect(m_plotHistory,
            &FormPlotHistory::windowClose,
            this,
            &FormExpert::plotHistoryClose,
            Qt::QueuedConnection);
    connect(formPlot,
            &FormPlot::toHistory,
            m_plotHistory,
            &FormPlotHistory::onHistoryRecv,
            Qt::QueuedConnection);
    connect(m_plotSimulate,
            &FormPlotSimulate::windowClose,
            this,
            &FormExpert::plotSimulateClose,
            Qt::QueuedConnection);
    connect(m_plotCorrection,
            &FormPlotCorrection::windowClose,
            this,
            &FormExpert::plotCorrectionClose,
            Qt::QueuedConnection);
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
    connect(m_plotCorrection,
            &FormPlotCorrection::sendParamsArcSin,
            m_worker,
            &ThreadWorker::onParamsArcSin,
            Qt::QueuedConnection);
    connect(formSerial,
            &FormSerial::recvTemperature,
            m_plotCorrection,
            &FormPlotCorrection::onTemperature,
            Qt::QueuedConnection);
    connect(m_worker,
            &ThreadWorker::showCorrectionCurve,
            m_plotCorrection,
            &FormPlotCorrection::onShowCorrectionCurve,
            Qt::QueuedConnection);
    connect(formSerial,
            &FormSerial::recvTemperature,
            m_plotHistory,
            &FormPlotHistory::onTemperature,
            Qt::QueuedConnection);
    connect(formPlot, &FormPlot::sendOffset31, this, [&](int val) { m_worker->setOffset31(val); });
    connect(formPlot, &FormPlot::sendOffset33, this, [&](int val) { m_worker->setOffset33(val); });
    connect(formPlot,
            &FormPlot::toExternalSpectral,
            formExternal,
            &FormExternal::onExternalSpectral,
            Qt::QueuedConnection);
    connect(formPlot,
            &FormPlot::broadcast,
            m_plotCorrection,
            &FormPlotCorrection::onBroadcast,
            Qt::QueuedConnection);
    connect(m_plotCorrection,
            &FormPlotCorrection::toExternalSpectral,
            formExternal,
            &FormExternal::onExternalSpectral,
            Qt::QueuedConnection);
    connect(m_worker,
            &ThreadWorker::changeThresholdStatus,
            m_plotCorrection,
            &FormPlotCorrection::onThresholdStatus,
            Qt::QueuedConnection);

    QObject::connect(formSerial,
                     &FormSerial::recv2PlotLLC,
                     m_worker,
                     &ThreadWorker::processDataLLC,
                     Qt::QueuedConnection);
    QObject::connect(formSerial,
                     &FormSerial::recv2PlotF30,
                     m_worker,
                     &ThreadWorker::processDataF30,
                     Qt::QueuedConnection);
    QObject::connect(formSerial,
                     &FormSerial::recv2DataF30,
                     formData,
                     &FormData::onDataReceivedF30,
                     Qt::QueuedConnection);
    QObject::connect(formSerial,
                     &FormSerial::recv2PlotF15,
                     m_worker,
                     &ThreadWorker::processDataF15,
                     Qt::QueuedConnection);
    QObject::connect(formSerial,
                     &FormSerial::recv2DataF15,
                     formData,
                     &FormData::onDataReceivedF15,
                     Qt::QueuedConnection);
    QObject::connect(formSerial,
                     &FormSerial::recv2MPU,
                     playMPU6050,
                     &FormPlayMPU6050::onRecvMPU,
                     Qt::QueuedConnection);
}

void FormExpert::on_btnSerial_clicked()
{
    ui->stackedWidget->setCurrentWidget(formSerial);
}

void FormExpert::on_btnPlot_clicked()
{
    ui->stackedWidget->setCurrentWidget(formPlot);
}

void FormExpert::on_btnData_clicked()
{
    ui->stackedWidget->setCurrentWidget(formData);
}

void FormExpert::on_btnLog_clicked()
{
    ui->stackedWidget->setCurrentWidget(formLog);
}

void FormExpert::on_btnUpdate_clicked()
{
    ui->stackedWidget->setCurrentWidget(formAutoUpdate);
}

void FormExpert::on_btnExternal_clicked()
{
    ui->stackedWidget->setCurrentWidget(formExternal);
}

void FormExpert::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Tab) {
        int count = ui->stackedWidget->count();
        m_currentPageIndex = (m_currentPageIndex + 1) % count;
        ui->stackedWidget->setCurrentIndex(m_currentPageIndex);

        QList<QToolButton *> buttonList = {ui->btnSerial, ui->btnData, ui->btnPlot, ui->btnLog};
        buttonList[m_currentPageIndex]->click();

        event->accept();
    }
}

void FormExpert::on_btnSetting_clicked()
{
    ui->stackedWidget->setCurrentWidget(formSetting);
}
