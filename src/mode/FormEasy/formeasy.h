#ifndef FORMEASY_H
#define FORMEASY_H

#include <QWidget>

namespace Ui {
class FormEasy;
}

class FormEasy : public QWidget
{
    Q_OBJECT

public:
    explicit FormEasy(QWidget *parent = nullptr);
    ~FormEasy();

private:
    Ui::FormEasy *ui;
};

#endif // FORMEASY_H
