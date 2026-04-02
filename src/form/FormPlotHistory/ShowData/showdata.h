#ifndef SHOWDATA_H
#define SHOWDATA_H

#include <QStandardItemModel>
#include <QWidget>

#include "global.h"

namespace Ui {
class ShowData;
}

class ShowData : public QWidget {
    Q_OBJECT

public:
    explicit ShowData(QWidget *parent = nullptr);
    ~ShowData();
    void showData(const MY_DATA &data);

signals:
    void windowClose();

private:
    void init();

private:
    Ui::ShowData *ui;
    QStandardItemModel *m_model = nullptr;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif  // SHOWDATA_H
