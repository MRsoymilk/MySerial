#include "formfittingself.h"
#include "ui_formfittingself.h"

#include <QChart>
#include <QChartView>
#include <QClipboard>
#include <QFile>
#include <QLineSeries>

FormFittingSelf::FormFittingSelf(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormFittingSelf)
{
    ui->setupUi(this);
    init();
}

FormFittingSelf::~FormFittingSelf()
{
    delete ui;
}

void FormFittingSelf::retranslateUI()
{
    ui->retranslateUi(this);
}

void FormFittingSelf::init()
{
    ui->spinBoxCountStep1->setValue(267);
    ui->spinBoxCountStep2->setValue(167);
    ui->spinBoxTotal->setValue(535);
    ui->lineEditHead->setText("DD3C043142");
    ui->lineEditFoot->setText("CDFF");
    ui->btnCalculate->setToolTip(tr("generate file fitting_self.csv"));
}

void FormFittingSelf::on_btnCalculate_clicked()
{
    m_values.clear();

    int start = ui->spinBoxStart->value();
    int middle = ui->spinBoxMiddle->value();
    int end = ui->spinBoxEnd->value();
    int count_step1 = ui->spinBoxCountStep1->value();
    int count_step2 = ui->spinBoxCountStep2->value();
    int total = ui->spinBoxTotal->value();

    // ---------- 第1段 ----------
    if (count_step1 > 1) {
        double step = double(start - middle) / (count_step1 - 1);
        double value = start;
        for (int i = 0; i < count_step1; ++i) {
            m_values.append(static_cast<int>(std::lround(value)));
            value -= step;
        }
    } else {
        m_values.append(start);
    }

    // ---------- 第2段 ----------
    if (count_step2 > 1) {
        double step = double(middle - end) / (count_step2 - 1);
        double value = middle;
        for (int i = 1; i < count_step2; ++i) { // i=1 避免重复 middle
            value -= step;
            m_values.append(static_cast<int>(std::lround(value)));
        }
    } else {
        if (m_values.isEmpty() || m_values.last() != end)
            m_values.append(end);
    }

    // ---------- 第3段 ----------
    while (m_values.size() < total) {
        int last = m_values.last();
        m_values.append(last - 10);
    }

    QFile file("fitting_self.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);

    out << "Index,Value\n";

    for (int i = 0; i < m_values.size(); ++i) {
        out << i << "," << QString::number(m_values[i]) << "\n";
    }

    file.close();
}

void FormFittingSelf::on_tBtnToHex_clicked()
{
    on_btnCalculate_clicked();

    QStringList hexList;
    for (int v : m_values) {
        QString hex = QString("%1").arg(v & 0xFFFF, 4, 16, QChar('0')).toUpper();
        QString hi = hex.mid(0, 2);
        QString lo = hex.mid(2, 2);
        hexList << hi << lo;
    }
    QString result = ui->lineEditHead->text().trimmed() + hexList.join("")
                     + ui->lineEditFoot->text().trimmed();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(result);
}

void FormFittingSelf::on_tBtnToIntArray_clicked()
{
    on_btnCalculate_clicked();

    QStringList intList;
    for (int v : m_values) {
        intList << QString::number(v);
    }

    QString cArray = QString("int data[%1] = { ").arg(intList.size()) + intList.join(", ") + " };";

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(cArray);
}

void FormFittingSelf::on_tBtnToPlot_clicked()
{
    on_btnCalculate_clicked();

    QLineSeries *series = new QLineSeries();
    for (int i = 0; i < m_values.size(); ++i) {
        series->append(i, m_values[i]);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle(tr("line"));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QWidget *plotWindow = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(plotWindow);
    layout->addWidget(chartView);
    plotWindow->setLayout(layout);
    plotWindow->resize(600, 400);
    plotWindow->setWindowTitle(tr("fitting line display"));
    plotWindow->show();
}
