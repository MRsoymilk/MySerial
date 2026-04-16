#ifndef FINDPEAK_H
#define FINDPEAK_H

#include <QLineSeries>
#include <QList>
#include <QPointF>

class FindPeak {
public:
    FindPeak();

    static QVector<QPointF> find(QLineSeries *line, int window = 3, double thresholdFactor = 1.0, double minDist = 5.0);
};

#endif  // FINDPEAK_H
