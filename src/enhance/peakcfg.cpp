#include "peakcfg.h"

#include "ui_peakcfg.h"

PeakCfg::PeakCfg(QWidget *parent) : QWidget(parent), ui(new Ui::PeakCfg) { ui->setupUi(this);
    init();
}

PeakCfg::~PeakCfg() { delete ui; }

void PeakCfg::init() {
    ui->spinBoxWindow->setValue(m_window);
    ui->doubleSpinBoxThreshold->setValue(m_threshold);
    ui->doubleSpinBoxMinDist->setValue(m_min_dist);
}

QVector<double> PeakCfg::getCfg()
{
    return {static_cast<double>(m_window), m_threshold, m_min_dist};
}

void PeakCfg::on_spinBoxWindow_valueChanged(int val)
{
    m_window = val;
}


void PeakCfg::on_doubleSpinBoxThreshold_valueChanged(double val)
{
    m_threshold = val;
}


void PeakCfg::on_doubleSpinBoxMinDist_valueChanged(double val)
{
    m_min_dist = val;
}

