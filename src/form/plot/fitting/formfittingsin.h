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
private:
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
    void doCorrection(const QVector<double> &v14);

signals:
    void sendSin(const QByteArray &bytes);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void init();
    void fillFittingCurveData();
    FormFittingSin::ZeroCrossing findPositiveToNegativeZeroCrossing();
    QByteArray packageRawData(const QVector<QPointF> &points);
    void saveCenteredAroundZeroCrossing(double xZero);

private:
    Ui::FormFittingSin *ui;
    QPixmap m_pixmap;
    QStandardItemModel *m_model;
    SIN m_sin;
};

#endif // FORMFITTINGSIN_H
