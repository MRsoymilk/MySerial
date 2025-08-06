#ifndef CALCULATE_H
#define CALCULATE_H

#include <QWidget>

namespace Ui {
class Calculate;
}

class Calculate : public QWidget
{
    Q_OBJECT
public:
    struct INI_CALCULATE
    {
        QString url;
    };

public:
    explicit Calculate(QWidget *parent = nullptr);
    ~Calculate();
    void init();

private slots:
    void on_lineEditCalculateURL_editingFinished();

private:
    Ui::Calculate *ui;
    INI_CALCULATE m_iniCalculate;
};

#endif // CALCULATE_H
