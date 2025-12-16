#ifndef TEMPERATURECONVERSION_H
#define TEMPERATURECONVERSION_H

#include <QDialog>

namespace Ui {
class TemperatureConversion;
}

class TemperatureConversion : public QDialog
{
    Q_OBJECT

public:
    explicit TemperatureConversion(QWidget *parent = nullptr);
    ~TemperatureConversion();
    double k();
    double b();

private:
    Ui::TemperatureConversion *ui;
};

#endif // TEMPERATURECONVERSION_H
