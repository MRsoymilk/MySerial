#ifndef SHOWDATA_H
#define SHOWDATA_H

#include <QWidget>
#include <QStandardItemModel>
#include "global.h"

namespace Ui {
class ShowData;
}

class ShowData : public QWidget
{
    Q_OBJECT

public:
    explicit ShowData(QWidget *parent = nullptr);
    ~ShowData();
    void showData(const MY_DATA &data);

private:
    void init();

private:
    Ui::ShowData *ui;
    QStandardItemModel *m_model = nullptr;
};

#endif // SHOWDATA_H
