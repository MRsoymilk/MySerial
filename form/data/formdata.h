#ifndef FORMDATA_H
#define FORMDATA_H

#include <QWidget>

namespace Ui {
class FormData;
}

class FormData : public QWidget
{
    Q_OBJECT

public:
    explicit FormData(QWidget *parent = nullptr);
    ~FormData();

private:
    Ui::FormData *ui;
};

#endif // FORMDATA_H
