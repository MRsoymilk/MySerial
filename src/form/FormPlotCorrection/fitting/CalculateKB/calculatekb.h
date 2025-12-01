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

    void on_spinBoxTemperatureColumn_valueChanged(int arg1);

private:
    void init();

private:
    Ui::CalculateKB *ui;
    QStandardItemModel *m_model;
    QJsonObject m_result;
    QString m_urlFitKB;
    int m_temp_column = 1;
};

#endif // CALCULATEKB_H
