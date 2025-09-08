#ifndef CALCULATEKB_H
#define CALCULATEKB_H

#include <QDialog>
#include <QJsonObject>
#include <QStandardItemModel>

namespace Ui {
class CalculateKB;
}

class CalculateKB : public QDialog
{
    Q_OBJECT

public:
    explicit CalculateKB(QWidget *parent = nullptr);
    ~CalculateKB();
    QJsonObject getResult();

private slots:
    void on_btnCalculate_clicked();
    void on_textEditInput_textChanged();

    void showContextMenu(const QPoint &pos);
    void deleteData();
    void displayData();

private:
    void init();

private:
    Ui::CalculateKB *ui;
    QStandardItemModel *m_model;
    QJsonObject m_result;
    QString m_urlFitKB;
};

#endif // CALCULATEKB_H
