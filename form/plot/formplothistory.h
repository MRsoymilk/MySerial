#ifndef FORMPLOTHISTORY_H
#define FORMPLOTHISTORY_H

#include <QChartView>
#include <QWidget>

namespace Ui {
class FormPlotHistory;
}

class FormPlotHistory : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlotHistory(QWidget *parent = nullptr);
    ~FormPlotHistory();

    void updateData(const QList<QList<QPointF>> &p14, const QList<QList<QPointF>> &p24);

signals:
    void windowClose();
private slots:
    void on_tBtnNext14_clicked();
    void on_tBtnPrev14_clicked();
    void on_tBtnNext24_clicked();
    void on_tBtnPrev24_clicked();

    void on_lineEdit14Go_editingFinished();
    void on_lineEdit24Go_editingFinished();

private:
    void updatePlot14();
    void updatePlot24();
    void init();

private:
    Ui::FormPlotHistory *ui;
    QList<QList<QPointF>> m_p14, m_p24;
    int m_index_14, m_index_24;
    QChartView *m_chartView14;
    QChartView *m_chartView24;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // FORMPLOTHISTORY_H
