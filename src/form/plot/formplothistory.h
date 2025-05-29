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
    const int INDEX_14 = 1;
    const int INDEX_24 = 2;

public:
    explicit FormPlotHistory(QWidget *parent = nullptr);
    ~FormPlotHistory();
    void updateData(const QList<QList<QPointF>> &p14, const QList<QList<QPointF>> &p24);

signals:
    void windowClose();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_tBtnNext14_clicked();
    void on_tBtnPrev14_clicked();
    void on_tBtnNext24_clicked();
    void on_tBtnPrev24_clicked();
    void on_lineEdit14Go_editingFinished();
    void on_lineEdit24Go_editingFinished();
    void on_checkBoxMix_checkStateChanged(const Qt::CheckState &state);
    void on_checkBoxSplit_checkStateChanged(const Qt::CheckState &state);

private:
    void updatePlot14();
    void updatePlot24();
    void updatePlot(int index = 0);
    void init();

private:
    Ui::FormPlotHistory *ui;
    QList<QList<QPointF>> m_p14, m_p24;
    int m_index_14, m_index_24;
    QChartView *m_chartView14;
    QChartView *m_chartView24;
    QChartView *m_chartView14Split = nullptr;
    QChartView *m_chartView24Split = nullptr;
    QChartView *m_chartMix;
    QWidget *m_charSplit;
    bool m_bMix;
    bool m_bSplit;
};

#endif // FORMPLOTHISTORY_H
