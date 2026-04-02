#ifndef FINDFWHM_H
#define FINDFWHM_H

#include <QList>
#include <QPointF>
#include <QtCharts/QLineSeries>

struct FWHMResult {
    double xPeak;
    double yPeak;
    double yHalf;
    double xLeft;
    double xRight;
    double fwhm;
};

class FindFWHM {
public:
    static QList<FWHMResult> find(QLineSeries *line, const QList<QPointF> &peaks);

private:
    static double interpolateX(double x1, double y1,
                               double x2, double y2, double yTarget);
    static std::optional<double> findHalfLeft(QLineSeries *line,
                                              double xPeak, double yHalf);
    static std::optional<double> findHalfRight(QLineSeries *line,
                                               double xPeak, double yHalf);
};
#endif  // FINDFWHM_H
