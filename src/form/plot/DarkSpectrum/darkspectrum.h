#ifndef DARKSPECTRUM_H
#define DARKSPECTRUM_H

#include <QWidget>

namespace Ui {
class DarkSpectrum;
}

class DarkSpectrum : public QWidget
{
    Q_OBJECT

public:
    explicit DarkSpectrum(QWidget *parent = nullptr);
    ~DarkSpectrum();

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::DarkSpectrum *ui;
};

#endif // DARKSPECTRUM_H
