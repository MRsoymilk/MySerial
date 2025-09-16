#ifndef METHODTHRESHOLD_H
#define METHODTHRESHOLD_H

#include <QJsonObject>

class MethodThreshold
{
public:
    MethodThreshold();
    void setParams(const QJsonObject &params);
    void generateThreshold();

private:
};

#endif // METHODTHRESHOLD_H
