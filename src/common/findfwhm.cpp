#include "findfwhm.h"

QList<FWHMResult> FindFWHM::find(QLineSeries *line, const QList<QPointF> &peaks) {
    QList<FWHMResult> results;
    if (!line || peaks.isEmpty()) return results;

    for (const auto &peak : peaks) {
        double yHalf = peak.y() / 2.0;
        auto xLeft  = findHalfLeft(line, peak.x(), yHalf);
        auto xRight = findHalfRight(line, peak.x(), yHalf);
        if (!xLeft || !xRight) continue;

        FWHMResult r;
        r.xPeak  = peak.x();
        r.yPeak  = peak.y();
        r.yHalf  = yHalf;
        r.xLeft  = *xLeft;
        r.xRight = *xRight;
        r.fwhm   = *xRight - *xLeft;
        results.append(r);
    }
    return results;
}

double FindFWHM::interpolateX(double x1, double y1,
                              double x2, double y2, double yTarget) {
    return x1 + (yTarget - y1) * (x2 - x1) / (y2 - y1);
}

std::optional<double> FindFWHM::findHalfLeft(QLineSeries *line,
                                             double xPeak, double yHalf) {
    for (int i = line->count() - 1; i >= 1; --i) {
        if (line->at(i).x() >= xPeak) continue;
        double y1 = line->at(i).y();
        double y2 = line->at(i - 1).y();
        if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
            return interpolateX(line->at(i).x(), y1,
                                line->at(i - 1).x(), y2, yHalf);
        }
    }
    return std::nullopt;
}

std::optional<double> FindFWHM::findHalfRight(QLineSeries *line,
                                              double xPeak, double yHalf) {
    for (int i = 0; i < line->count() - 1; ++i) {
        if (line->at(i).x() <= xPeak) continue;
        double y1 = line->at(i).y();
        double y2 = line->at(i + 1).y();
        if ((y1 >= yHalf && y2 <= yHalf) || (y1 <= yHalf && y2 >= yHalf)) {
            return interpolateX(line->at(i).x(), y1,
                                line->at(i + 1).x(), y2, yHalf);
        }
    }
    return std::nullopt;
}
