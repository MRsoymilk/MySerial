#include "formplotcorrection.h"
#include "funcdef.h"
#include "ui_formplotcorrection.h"

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
            }
        } else if (ui->stackedWidget->currentWidget() == m_formSin) {
            m_formSin->doCorrection(v14);
            m_start = false;
        }
    } else {
        ui->btnStart->setStyleSheet("");
    }
    QCoreApplication::processEvents();
}

void FormPlotCorrection::init()
{
    m_start = false;
    m_formKB = new FormFittingKB;
    m_formSin = new FormFittingSin;
    m_formSelf = new FormFittingSelf;

    ui->stackedWidget->addWidget(m_formKB);
    ui->stackedWidget->addWidget(m_formSin);
    ui->stackedWidget->addWidget(m_formSelf);

    QStringList algorithms;
    algorithms << "fitting_self" << "fitting_sin" << "fitting_kb";
    ui->comboBoxAlgorithm->addItems(algorithms);

    QString txt = ui->comboBoxAlgorithm->currentText();
    if (txt == "fitting_sin") {
        ui->stackedWidget->setCurrentWidget(m_formSin);
    } else if (txt == "fitting_kb") {
        ui->stackedWidget->setCurrentWidget(m_formKB);
    } else if (txt == "fitting_self") {
        ui->stackedWidget->setCurrentWidget(m_formSelf);
    }
    connect(m_formSin, &FormFittingSin::sendSin, this, &FormPlotCorrection::sendSin);
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
    }
}
