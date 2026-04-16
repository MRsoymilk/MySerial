#ifndef POINTSTRACKER_H
#define POINTSTRACKER_H

#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QWidget>

class MyChartView;

namespace Ui {
class PointsTracker;
}

class PointsTracker : public QWidget {
    Q_OBJECT

public:
    explicit PointsTracker(QWidget *parent = nullptr);
    ~PointsTracker();
    void addPoints(QMap<QString, double> values);
    void clearPoints();

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void init();
    void exportSelectedCSV(const QString &name);
    void exportCSV();
    void onContextMenu(const QPoint &pos);

private:
    Ui::PointsTracker *ui;
    QChart *m_chart;
    MyChartView *m_chartView;

    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY = nullptr;

    QMap<QString, QLineSeries *> m_mapLines;
    int m_idx = 0;
};

#endif  // POINTSTRACKER_H
