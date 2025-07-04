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

public:
    explicit FormFittingSin(QWidget *parent = nullptr);
    ~FormFittingSin();
    void doCorrection(const QVector<double> &v14);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void init();
    void fillFittingCurveData();

private:
    Ui::FormFittingSin *ui;
    QPixmap m_pixmap;
    QStandardItemModel *m_model;
    SIN m_sin;
};

#endif // FORMFITTINGSIN_H
