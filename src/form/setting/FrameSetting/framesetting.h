#ifndef FRAMESETTING_H
#define FRAMESETTING_H

#include <QWidget>

namespace Ui {
class FrameSetting;
}

class FrameSetting : public QWidget
{
    Q_OBJECT

public:
    explicit FrameSetting(QWidget *parent = nullptr);
    ~FrameSetting();
    void retranslateUI();

private slots:
    void on_tBtnCalculate24bit_clicked();
    void on_tBtnCalculate14bit_clicked();
    void on_btnUpdate_clicked();

private:
    void init();

private:
    Ui::FrameSetting *ui;
};

#endif // FRAMESETTING_H
