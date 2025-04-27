#ifndef FORMDATA_H
#define FORMDATA_H

#include <QStandardItemModel>
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

public slots:
    void onDataReceived(const QByteArray &data);

private slots:
    void exportToCSV();

private:
    void init();

private:
    Ui::FormData *ui;
    QStandardItemModel *model;
    void showContextMenu(const QPoint &pos);
};

#endif // FORMDATA_H
