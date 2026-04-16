#include "findpeak.h"

FindPeak::FindPeak() {}

QVector<QPointF> FindPeak::find(QLineSeries *line, int window, double thresholdFactor, double minDist) {
    QVector<QPointF> peaks;
    int n = line->count();

    QVector<double> values;
    values.reserve(n);
    for (int i = 0; i < n; i++) values.append(line->at(i).y());

    double mean = std::accumulate(values.begin(), values.end(), 0.0) / n;
    double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / n - mean * mean);
    double threshold = mean + thresholdFactor * stdev;

    double lastPeakX = -1e9;
    for (int i = window; i < n - window; i++) {
        double yCurr = line->at(i).y();
        if (yCurr < threshold) continue;

        bool isPeak = true;
        for (int j = i - window; j <= i + window; j++) {
            if (line->at(j).y() > yCurr) {
                isPeak = false;
                break;
            }
        }

        if (isPeak) {
            double xCurr = line->at(i).x();
            if (xCurr - lastPeakX >= minDist) {
                peaks.append(line->at(i));
                lastPeakX = xCurr;
            }
        }
    }
    return peaks;
}
