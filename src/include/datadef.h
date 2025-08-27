#ifndef DATADEF_H
#define DATADEF_H

#include <QString>

const QString SERVER_UPDATE = "http://192.168.123.233:8000";
const QString SERVER_CALCULATE = "http://192.168.123.233:5000";
const QString URL_FITTING_SIN = QString("%1/%2").arg(SERVER_CALCULATE).arg("fit_sin");
const QString URL_FIND_PEAK = QString("%1/%2").arg(SERVER_CALCULATE).arg("find_peak");
#endif
