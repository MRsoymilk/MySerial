#include "formplotcorrection.h"
#include "funcdef.h"
#include "ui_formplotcorrection.h"

#include "ShowCorrectionCurve/showcorrectioncurve.h"
#include "fitting/formfittingarcsin.h"
#include "fitting/formfittingkb.h"
#include "fitting/formfittingself.h"
#include "fitting/formfittingsin.h"

FormPlotCorrection::FormPlotCorrection(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlotCorrection)
{
    ui->setupUi(this);
    init();
}

FormPlotCorrection::~FormPlotCorrection()
{
    delete ui;
}

void FormPlotCorrection::closeEvent(QCloseEvent *event)
{
    emit windowClose();
    QWidget::closeEvent(event);
}

void FormPlotCorrection::onEpochCorrection(const QVector<double> &v14, const QVector<double> &v24)
{
    if (m_start) {
        if (ui->stackedWidget->currentWidget() == m_formKB) {
            m_formKB->doCorrection(v14, v24);
            if (m_formKB->getRound() <= 0) {
                m_start = false;
                ui->btnStart->setStyleSheet("");
            }
        } else if (ui->stackedWidget->currentWidget() == m_formSin) {
            m_formSin->doCorrection(v14, v24);
            m_start = false;
            ui->btnStart->setStyleSheet("");
        }
    } else {
        ui->btnStart->setStyleSheet("");
    }
    QCoreApplication::processEvents();
}

void FormPlotCorrection::onTemperature(double temperature)
{
    m_formSin->setTemperature(temperature);
}

void FormPlotCorrection::init()
{
    m_start = false;
    m_formKB = new FormFittingKB;
    m_formSin = new FormFittingSin;
    m_formSelf = new FormFittingSelf;
    m_formArcSin = new FormFittingArcSin;

    ui->stackedWidget->addWidget(m_formKB);
    ui->stackedWidget->addWidget(m_formSin);
    ui->stackedWidget->addWidget(m_formSelf);
    ui->stackedWidget->addWidget(m_formArcSin);

    QStringList algorithms;
    algorithms << "fitting_arcsin" << "fitting_sin" << "fitting_self" << "fitting_kb";
    ui->comboBoxAlgorithm->addItems(algorithms);

    QString txt = ui->comboBoxAlgorithm->currentText();
    if (txt == "fitting_arcsin") {
        ui->stackedWidget->setCurrentWidget(m_formArcSin);
    } else if (txt == "fitting_sin") {
        ui->stackedWidget->setCurrentWidget(m_formSin);
    } else if (txt == "fitting_kb") {
        ui->stackedWidget->setCurrentWidget(m_formKB);
    } else if (txt == "fitting_self") {
        ui->stackedWidget->setCurrentWidget(m_formSelf);
    }
    connect(m_formSin, &FormFittingSin::sendSin, this, &FormPlotCorrection::sendSin);
    connect(m_formArcSin, &FormFittingArcSin::sendSin, this, &FormPlotCorrection::sendSin);
    ui->tBtnShowCorrectionCurve->setCheckable(true);
}

void FormPlotCorrection::on_btnStart_clicked()
{
    m_start = true;
    ui->btnStart->setStyleSheet("background-color: green; color: white;");
}

void FormPlotCorrection::on_comboBoxAlgorithm_currentTextChanged(const QString &algorithm)
{
    if (algorithm == "fitting_kb") {
        ui->stackedWidget->setCurrentWidget(m_formKB);
    } else if (algorithm == "fitting_sin") {
        ui->stackedWidget->setCurrentWidget(m_formSin);
    } else if (algorithm == "fitting_self") {
        ui->stackedWidget->setCurrentWidget(m_formSelf);
    } else if (algorithm == "fitting_arcsin") {
        ui->stackedWidget->setCurrentWidget(m_formArcSin);
    }
}

void FormPlotCorrection::on_tBtnShowCorrectionCurve_clicked()
{
    m_show = !m_show;
    ui->tBtnShowCorrectionCurve->setChecked(m_show);
    if (m_show) {
        QString txt = ui->comboBoxAlgorithm->currentText();
        if (txt == "fitting_sin") {
            m_formSin->updateParams();
            QJsonObject params = m_formSin->getParams();

            emit enableCorrectionCurve(true, params);
            ShowCorrectionCurve *sinShow = new ShowCorrectionCurve;
            connect(this,
                    &FormPlotCorrection::onShowCorrectionCurve,
                    sinShow,
                    &ShowCorrectionCurve::updatePlot);
            connect(sinShow, &ShowCorrectionCurve::windowClose, this, [&]() {
                m_show = false;
                ui->tBtnShowCorrectionCurve->setChecked(false);
            });
            sinShow->show();
        } else {
            SHOW_AUTO_CLOSE_MSGBOX(this, tr("Warning"), "only support fitting_sin");
        }
    } else {
        emit enableCorrectionCurve(false, {});
    }
}
