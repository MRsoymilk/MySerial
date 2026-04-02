#include "pointstracker.h"

#include "MyChartView/mychartview.h"
#include "ui_pointstracker.h"

PointsTracker::PointsTracker(QWidget *parent) : QWidget(parent), ui(new Ui::PointsTracker) {
    ui->setupUi(this);
    init();
}

PointsTracker::~PointsTracker() { delete ui; }

void PointsTracker::addPoints(QMap<QString, double> values) {
    for (auto it = values.begin(); it != values.end(); ++it) {
        const QString &name = it.key();
        double value = it.value();

        if (!m_mapLines.contains(name)) {
            QLineSeries *series = new QLineSeries;
            series->setName(name);
            static int colorIndex = 0;
            QColor color = QColor::fromHsv((colorIndex * 40) % 360, 200, 220);
            series->setColor(color);
            colorIndex++;
            for (int i = 0; i < m_idx; ++i) {
                series->append(i, std::numeric_limits<double>::quiet_NaN());
            }
            m_chart->addSeries(series);
            series->attachAxis(m_axisX);
            series->attachAxis(m_axisY);
            m_mapLines.insert(name, series);
        }

        QLineSeries *series = m_mapLines[name];
        series->append(m_idx, value);
        m_axisY->setRange(std::min(m_axisY->min(), value), std::max(m_axisY->max(), value));
    }

    if (m_idx > m_axisX->max()) m_axisX->setMax(m_idx);

    m_idx++;
}

void PointsTracker::clearPoints() {
    m_chart->removeAllSeries();
    m_axisX->setRange(0, 10);
    m_axisY->setRange(0, 1);
    m_mapLines.clear();
    m_idx = 0;
}

void PointsTracker::init() {
    m_chart = new QChart();
    m_chart->setTitle(tr("Points Tracker"));

    m_axisX = new QValueAxis;
    m_axisX->setTitleText(tr("Index"));

    m_axisY = new QValueAxis;
    m_axisY->setTitleText(tr("Value"));

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->gLayChart->addWidget(m_chartView);

    m_chartView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_chartView, &QWidget::customContextMenuRequested,
            this, &PointsTracker::onContextMenu);
}

void PointsTracker::onContextMenu(const QPoint &pos) {
    if (m_mapLines.isEmpty()) return;

    QMenu menu(this);

    QAction *actAll = menu.addAction(tr("导出全部数据"));
    connect(actAll, &QAction::triggered, this, &PointsTracker::exportCSV);

    if (m_mapLines.size() > 1) {
        QMenu *subMenu = menu.addMenu(tr("导出单条数据"));
        for (auto it = m_mapLines.begin(); it != m_mapLines.end(); ++it) {
            const QString &name = it.key();
            QAction *act = subMenu->addAction(name);
            connect(act, &QAction::triggered, this, [this, name]() {
                exportSelectedCSV(name);
            });
        }
    }

    menu.exec(m_chartView->mapToGlobal(pos));
}

void PointsTracker::exportCSV() {
    if (m_mapLines.isEmpty()) return;

    QString path = QFileDialog::getSaveFileName(
        this, tr("export all"), "points_tracker.csv",
        "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);

    QStringList names = m_mapLines.keys();
    out << "Index";
    for (const auto &name : names) out << "," << name;
    out << "\n";

    int maxCount = 0;
    for (auto *s : m_mapLines) maxCount = std::max(maxCount, s->count());

    for (int i = 0; i < maxCount; ++i) {
        out << i;
        for (const auto &name : names) {
            QLineSeries *s = m_mapLines[name];
            if (i < s->count()) {
                double y = s->at(i).y();
                std::isnan(y) ? out << "," : out << "," << y;
            } else {
                out << ",";
            }
        }
        out << "\n";
    }
}

void PointsTracker::exportSelectedCSV(const QString &name) {
    if (!m_mapLines.contains(name)) return;

    QString path = QFileDialog::getSaveFileName(
        this, tr("export %1").arg(name),
        QString("%1.csv").arg(name),
        "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);

    out << "Index," << name << "\n";
    QLineSeries *s = m_mapLines[name];
    for (int i = 0; i < s->count(); ++i) {
        double y = s->at(i).y();
        std::isnan(y) ? out << i << "," << "\n"
                      : out << i << "," << y << "\n";
    }
}

void PointsTracker::closeEvent(QCloseEvent *event) {
    clearPoints();
    emit windowClose();
}
