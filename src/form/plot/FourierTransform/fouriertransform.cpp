#include "fouriertransform.h"
#include "DraggableLine/draggableline.h"
#include "MyChartView/mychartview.h"
#include "funcdef.h"
#include "keydef.h"
#include "ui_fouriertransform.h"
#include <cmath>
#include <fftw3.h>
#include <limits>

FourierTransform::FourierTransform(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FourierTransform)
{
    ui->setupUi(this);
    init();
}

FourierTransform::~FourierTransform()
{
    delete ui;
}

void FourierTransform::setSampleRate(double rate)
{
    if (rate > 0) {
        m_sampleRate = rate;
    }
}
QList<QPointF> FourierTransform::ifftBandLimited(const QList<QPointF> &data,
                                                 double sampleRate,
                                                 double freqStart,
                                                 double freqEnd)
{
    int N = data.size();
    if (N == 0)
        return {};

    double *in = (double *) fftw_malloc(sizeof(double) * N);
    fftw_complex *freq = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
    double *out = (double *) fftw_malloc(sizeof(double) * N);

    for (int i = 0; i < N; ++i) {
        in[i] = data[i].y();
    }

    fftw_plan p_forward = fftw_plan_dft_r2c_1d(N, in, freq, FFTW_ESTIMATE);
    fftw_execute(p_forward);

    for (int k = 0; k <= N / 2; ++k) {
        double f = (double) k * sampleRate / N;

        bool isDC = (k == 0);

        if (f < freqStart || f > freqEnd) {
            if (isDC && freqStart <= 1e-6) {
            } else {
                freq[k][0] = 0.0;
                freq[k][1] = 0.0;
            }
        }
    }

    fftw_plan p_inverse = fftw_plan_dft_c2r_1d(N, freq, out, FFTW_ESTIMATE);
    fftw_execute(p_inverse);

    for (int i = 0; i < N; ++i) {
        out[i] /= N;
    }

    QList<QPointF> result;
    result.reserve(N);

    double y_min = std::numeric_limits<double>::max();
    double y_max = std::numeric_limits<double>::lowest();

    for (int i = 0; i < N; ++i) {
        double t = data[i].x();
        double y = out[i];
        result.append(QPointF(t, y));

        if (y < y_min)
            y_min = y;
        if (y > y_max)
            y_max = y;
    }

    fftw_destroy_plan(p_forward);
    fftw_destroy_plan(p_inverse);
    fftw_free(in);
    fftw_free(freq);
    fftw_free(out);

    if (m_lineIFFT)
        m_lineIFFT->replace(result);

    double margin = (y_max - y_min) * 0.1;
    if (qAbs(margin) < 1e-6)
        margin = 1.0;

    if (m_axisYIFFT) {
        m_axisYIFFT->setRange(y_min - margin, y_max + margin);
    }

    return result;
}

QList<QPointF> FourierTransform::fftAmplitude(const QList<QPointF> &data, double sampleRate)
{
    int N = data.size();
    if (N == 0)
        return {};

    double *in = (double *) fftw_malloc(sizeof(double) * N);
    fftw_complex *out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));

    for (int i = 0; i < N; i++)
        in[i] = data[i].y();

    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    QList<QPointF> spectrum;
    spectrum.reserve(N / 2 + 1);

    double x_max = 0;
    double y_max = std::numeric_limits<double>::min();

    for (int k = 0; k <= N / 2; k++) {
        double real = out[k][0];
        double imag = out[k][1];

        double amplitude = sqrt(real * real + imag * imag);

        amplitude = amplitude / N;
        if (k > 0)
            amplitude *= 2;

        double freq = (double) k * sampleRate / N;

        spectrum.append(QPointF(freq, amplitude));

        if (freq > x_max)
            x_max = freq;
        if (amplitude > y_max)
            y_max = amplitude;
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    m_lineFFT->replace(spectrum);
    m_axisXFFT->setRange(0, x_max);
    m_axisYFFT->setRange(0, y_max * 1.1);

    return spectrum;
}

void FourierTransform::transform(const QList<QPointF> &data)
{
    if (data.isEmpty())
        return;

    m_currentData = data;

    fftAmplitude(m_currentData, m_sampleRate);

    if (!m_currentData.isEmpty()) {
        double t_start = m_currentData.first().x();
        double t_end = m_currentData.last().x();
        m_axisXIFFT->setRange(t_start, t_end);
    }

    if (m_enableRange) {
        updateIFFT();
    } else {
        m_lineIFFT->replace(m_currentData);

        double y_min = std::numeric_limits<double>::max();
        double y_max = std::numeric_limits<double>::lowest();
        for (const auto &p : m_currentData) {
            if (p.y() < y_min)
                y_min = p.y();
            if (p.y() > y_max)
                y_max = p.y();
        }
        m_axisYIFFT->setRange(y_min, y_max);
    }
}

void FourierTransform::updateIFFT()
{
    if (m_currentData.isEmpty())
        return;
    ifftBandLimited(m_currentData, m_sampleRate, m_start, m_end);
}

void FourierTransform::init()
{
    m_lineFFT = new QLineSeries();
    m_chartFFT = new QChart();
    m_chartFFT->addSeries(m_lineFFT);
    m_chartFFT->setTitle(tr("Frequency Spectrum (FFT)"));

    m_axisXFFT = new QValueAxis();
    m_axisXFFT->setTitleText("Frequency (Hz)");
    m_chartFFT->addAxis(m_axisXFFT, Qt::AlignBottom);
    m_lineFFT->attachAxis(m_axisXFFT);

    m_axisYFFT = new QValueAxis();
    m_axisYFFT->setTitleText("Amplitude");
    m_chartFFT->addAxis(m_axisYFFT, Qt::AlignLeft);
    m_lineFFT->attachAxis(m_axisYFFT);

    m_chartViewFFT = new MyChartView(m_chartFFT);
    m_chartViewFFT->setRenderHint(QPainter::Antialiasing);
    ui->gLayFFT->addWidget(m_chartViewFFT);

    m_lineIFFT = new QLineSeries();
    m_chartIFFT = new QChart();
    m_chartIFFT->addSeries(m_lineIFFT);
    m_chartIFFT->setTitle(tr("Filtered Signal (IFFT)"));

    m_axisXIFFT = new QValueAxis();
    m_axisXIFFT->setTitleText("Time (s)");
    m_chartIFFT->addAxis(m_axisXIFFT, Qt::AlignBottom);
    m_lineIFFT->attachAxis(m_axisXIFFT);

    m_axisYIFFT = new QValueAxis();
    m_axisYIFFT->setTitleText("Value");
    m_chartIFFT->addAxis(m_axisYIFFT, Qt::AlignLeft);
    m_lineIFFT->attachAxis(m_axisYIFFT);

    m_chartViewIFFT = new MyChartView(m_chartIFFT);
    m_chartViewIFFT->setRenderHint(QPainter::Antialiasing);
    ui->gLayIFFT->addWidget(m_chartViewIFFT);

    ui->tBtnRange->setCheckable(true);
    int rate = SETTING_CONFIG_GET(CFG_GROUP_FOURIER, CFG_FOURIER_SAMPLE_RATE, "1000").toInt();
    ui->spinBoxSampleRate->setValue(rate);
}

void FourierTransform::on_tBtnRange_clicked()
{
    m_enableRange = ui->tBtnRange->isChecked();

    if (m_enableRange) {
        QRectF plotArea = m_chartFFT->plotArea();

        double defaultStartPixels = plotArea.left();
        double defaultEndPixels = plotArea.right();

        m_lineLeft = new DraggableLine(m_chartFFT, defaultStartPixels, Qt::green);
        m_lineRight = new DraggableLine(m_chartFFT, defaultEndPixels, Qt::darkGreen);

        connect(m_lineLeft, &DraggableLine::xValueChanged, this, [this](qreal left) {
            left = std::max(left, m_axisXFFT->min());
            m_start = left;

            ui->doubleSpinBoxStart->blockSignals(true);
            ui->doubleSpinBoxStart->setValue(m_start);
            ui->doubleSpinBoxStart->blockSignals(false);

            updateIFFT();
        });

        connect(m_lineRight, &DraggableLine::xValueChanged, this, [this](qreal right) {
            right = std::min(right, m_axisXFFT->max());
            m_end = right;

            ui->doubleSpinBoxEnd->blockSignals(true);
            ui->doubleSpinBoxEnd->setValue(m_end);
            ui->doubleSpinBoxEnd->blockSignals(false);

            updateIFFT();
        });

        m_chartFFT->scene()->addItem(m_lineLeft);
        m_chartFFT->scene()->addItem(m_lineRight);

        m_start = m_axisXFFT->min();
        m_end = m_axisXFFT->max();
        ui->doubleSpinBoxStart->setValue(m_start);
        ui->doubleSpinBoxEnd->setValue(m_end);
        updateIFFT();

    } else {
        if (m_lineLeft) {
            m_chartFFT->scene()->removeItem(m_lineLeft);
            delete m_lineLeft;
            m_lineLeft = nullptr;
        }
        if (m_lineRight) {
            m_chartFFT->scene()->removeItem(m_lineRight);
            delete m_lineRight;
            m_lineRight = nullptr;
        }

        m_start = 0;
        m_end = m_sampleRate / 2;

        if (m_lineIFFT && !m_currentData.isEmpty()) {
            m_lineIFFT->replace(m_currentData);
            double y_min = std::numeric_limits<double>::max();
            double y_max = std::numeric_limits<double>::lowest();
            for (auto p : m_currentData) {
                if (p.y() < y_min)
                    y_min = p.y();
                if (p.y() > y_max)
                    y_max = p.y();
            }
            m_axisYIFFT->setRange(y_min, y_max);
        }
    }
}

void FourierTransform::on_spinBoxSampleRate_valueChanged(int rate)
{
    m_sampleRate = rate;
    SETTING_CONFIG_SET(CFG_GROUP_FOURIER, CFG_FOURIER_SAMPLE_RATE, QString::number(rate));
}
