#ifndef FORMEXPERT_H
#define FORMEXPERT_H

#include <QWidget>

namespace Ui {
class FormExpert;
}

class FormExpert : public QWidget
{
    Q_OBJECT

public:
    explicit FormExpert(QWidget *parent = nullptr);
    ~FormExpert();

private:
    Ui::FormExpert *ui;
};

#endif // FORMEXPERT_H
