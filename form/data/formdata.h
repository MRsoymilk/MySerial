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
    void onDataReceived(const QByteArray &data, const QString &name);

private slots:
    void exportToCSV();
    void clearData();

private:
    void init();
    void showContextMenu(const QPoint &pos);

private:
    Ui::FormData *ui;
    QStandardItemModel *m_model;
};

#endif // FORMDATA_H
