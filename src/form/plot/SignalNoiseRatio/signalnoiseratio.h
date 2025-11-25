#ifndef SIGNALNOISERATIO_H
#define SIGNALNOISERATIO_H

#include <QWidget>

namespace Ui {
class SignalNoiseRatio;
}

class SignalNoiseRatio : public QWidget
{
    Q_OBJECT

public:
    explicit SignalNoiseRatio(QWidget *parent = nullptr);
    ~SignalNoiseRatio();

private:
    Ui::SignalNoiseRatio *ui;
};

#endif // SIGNALNOISERATIO_H
