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

struct PARAMS_ARCSIN
{
    double t_k1, t_b1;
    double t_k2, t_b2;
    double l_k, l_b, l_d, l_alpha;
    double r_k, r_b, r_d, r_alpha;
};

#endif  // GLOBAL_H
