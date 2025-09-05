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
    void retranslateUI();

public slots:
    void onDataReceived4k(const QByteArray &data14,
                          const QByteArray &data24,
                          const QByteArray &temperature = "");

private slots:
    void exportToCSV();
    void exportAllToCSV();
    void clearData();
    void on_lineEditDataLimit_editingFinished();

private:
    void init();
    void showContextMenu(const QPoint &pos);
    void getINI();
    void setINI();
    void exportSeleced(const QModelIndexList &selectedRows, QTextStream &stream);
    void exportAll(QTextStream &stream);

private:
    Ui::FormData *ui;
    QStandardItemModel *m_model;
    int m_limit;
};

#endif // FORMDATA_H
