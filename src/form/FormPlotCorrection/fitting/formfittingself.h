#ifndef FORMFITTINGSELF_H
#define FORMFITTINGSELF_H

#include <QWidget>

namespace Ui {
class FormFittingSelf;
}

class FormFittingSelf : public QWidget
{
    Q_OBJECT

public:
    explicit FormFittingSelf(QWidget *parent = nullptr);
    ~FormFittingSelf();

private slots:
    void on_btnCalculate_clicked();
    void on_tBtnToHex_clicked();
    void on_tBtnToIntArray_clicked();
    void on_tBtnToPlot_clicked();

private:
    void init();

private:
    Ui::FormFittingSelf *ui;
    QVector<double> m_values;
};

#endif // FORMFITTINGSELF_H
