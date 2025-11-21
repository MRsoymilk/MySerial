#ifndef GLOBAL_H
#define GLOBAL_H

#include <QList>
#include <QPointF>
#include <limits>

struct CURVE
{
    QList<QPointF> data;
    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::min();
    double y_min = std::numeric_limits<double>::max();
    double y_max = std::numeric_limits<double>::min();
};

#endif  // GLOBAL_H
