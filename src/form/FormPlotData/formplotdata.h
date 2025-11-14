#ifndef FORMPLOTDATA_H
#define FORMPLOTDATA_H

#include <QLineSeries>
#include <QStandardItem>
#include <QWidget>

namespace Ui {
class FormPlotData;
}

class FormPlotData : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlotData(QWidget *parent = nullptr);
    ~FormPlotData();
    void retranslateUI();

signals:
    void windowClose();

public slots:
    void updateTable4k(const QVector<double> &v14,
                       const QVector<double> &v24,
                       const QVector<double> &raw14,
                       const QVector<double> &raw24);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void showContextMenu(const QPoint &pos);
    void exportCurrentToCSV();
    void exportAllToCSV();
    void clearData();
    void on_tBtnPrev_clicked();
    void on_tBtnNext_clicked();
    void on_lineEditGo_editingFinished();
    void on_checkBoxChooseGroup_checkStateChanged(const Qt::CheckState &state);

private:
    void init();
    void displayData(const QVector<double> &v14,
                     const QVector<double> &v24,
                     const QVector<double> &raw14,
                     const QVector<double> &raw24);

private:
    Ui::FormPlotData *ui;
    QStandardItemModel *m_model;
    QList<QVector<double>> listV14;
    QList<QVector<double>> listV24;
    QList<QVector<double>> listRaw14;
    QList<QVector<double>> listRaw24;
    int m_count = 0;
    int m_current = 0;
};

#endif // FORMPLOTDATA_H
