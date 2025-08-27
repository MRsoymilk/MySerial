#ifndef FORMFITTINGSIN_H
#define FORMFITTINGSIN_H

#include <QStandardItem>
#include <QWidget>

namespace Ui {
class FormFittingSin;
}

class FormFittingSin : public QWidget
{
    Q_OBJECT
public:
    struct SIN
    {
        double A;
        double w;
        double xc;
        double y0;
    };

    struct ZeroCrossing
    {
        double xZero;
        double yLeft;
        double yRight;
    };

public:
    explicit FormFittingSin(QWidget *parent = nullptr);
    ~FormFittingSin();
    void doCorrection(const QVector<double> &v14, const QVector<double> &v24);

signals:
    void sendSin(const QByteArray &bytes);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_btnAdjust_clicked();
    void on_btnUpdate_clicked();
    void showContextMenu(const QPoint &pos);
    void exportAllToCSV();
    void on_btnGenerateThreshold_clicked();

private:
    void init();
    void fillFittingCurveData();
    QByteArray packageRawData(const QVector<QPointF> &points);
    void fillFixedFittingCurveData(const double &start);
    void packageRawData();

private:
    Ui::FormFittingSin *ui;
    QPixmap m_pixSin;
    QPixmap m_pixPeak;
    QStandardItemModel *m_model;
    SIN m_sin, m_sin_fixed;
    QVector<qint32> m_threshold_table;
};

#endif // FORMFITTINGSIN_H
