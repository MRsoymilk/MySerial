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
    // void onDataReceived(const QByteArray &data, const QString &name);
    void onDataReceived4k(const QByteArray &data14, const QByteArray &data24);

private slots:
    void exportToCSV();
    void clearData();

    void on_lineEditDataLimit_editingFinished();

private:
    void init();
    void showContextMenu(const QPoint &pos);
    void getINI();
    void setINI();

private:
    Ui::FormData *ui;
    QStandardItemModel *m_model;
    int m_limit;
};

#endif // FORMDATA_H
