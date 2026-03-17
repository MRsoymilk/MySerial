#include "darkspectrum.h"

#include "ui_darkspectrum.h"

DarkSpectrum::DarkSpectrum(QWidget *parent) : QWidget(parent), ui(new Ui::DarkSpectrum) { ui->setupUi(this); }

DarkSpectrum::~DarkSpectrum() { delete ui; }

void DarkSpectrum::calculate(const QVector<double> &v) {
    if (v.isEmpty()) return;

    double avg = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    ui->lineEditBaseLine->setText(QString::number(avg));

    if (m_lastDark.isEmpty()) {
        m_lastDark = v;
        return;
    }

    int N = std::min(v.size(), m_lastDark.size());

    QVector<double> delta(N);

    for (int i = 0; i < N; ++i) delta[i] = v[i] - m_lastDark[i];

    double meanDelta = std::accumulate(delta.begin(), delta.end(), 0.0) / N;

    double sum = 0.0;
    for (double d : delta) sum += (d - meanDelta) * (d - meanDelta);

    double sigma = std::sqrt(sum / (N - 1));

    ui->lineEditRMS->setText(QString::number(sigma));

    const double maxADC = std::pow(2.0, 23) - 1.0;

    double dr = (maxADC - avg) / sigma;

    ui->lineEditDR->setText(QString::number(dr));

    m_lastDark = v;
    emit doCalculate(false);
}

void DarkSpectrum::closeEvent(QCloseEvent *event) { emit windowClose(); }

void DarkSpectrum::on_btnRefresh_clicked() {
    m_lastDark.clear();
    ui->lineEditBaseLine->setText("");
    ui->lineEditRMS->setText("");
    ui->lineEditDR->setText("");
    emit doCalculate(true);
}
