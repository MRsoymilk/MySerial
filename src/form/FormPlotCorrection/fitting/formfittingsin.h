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
        double T;
        double k1;
        double b1;
        double k2;
        double b2;
    };

public:
    explicit FormFittingSin(QWidget *parent = nullptr);
    ~FormFittingSin();
    void doCorrection(const QVector<double> &v14, const QVector<double> &v24);
    void setTemperature(double temperature);
    QJsonObject getParams();
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
    void on_btnCalculate_k1b1_k2b2_clicked();
    void on_btnSetTemperature_clicked();
    void on_btnSendFormula_clicked();
    void on_btnSendR_kb_clicked();

    void on_btnSendSegmentFormula_clicked();

    void on_tBtnAddStep1_clicked();
    void on_tBtnAddStep2_clicked();

private:
    void init();
    QByteArray packageRawData(const QVector<QPointF> &points);
    void fillFixedFittingCurveData(const double &start);
    void packageRawData(bool isSend = false);
    std::optional<QPair<double, double> > solveSinParams_hard(
        double x1, double y1, double x2, double y2, double A, double y0);
    QByteArray buildFrame();
    void startFitting();

private:
    Ui::FormFittingSin *ui;
    QPixmap m_pixSin;
    QPixmap m_pixPeak;
    QStandardItemModel *m_model;
    SIN m_sin, m_sin_fixed;
    QVector<qint32> m_threshold_table;
    QVector<double> m_v14;
    QVector<double> m_v24;
    double m_k;
    double m_b;
    bool m_data_ready = false;
    SIN m_step_1, m_step_2;
    QString m_urlCalculate;
    QString m_urlFindPeak;
};

#endif // FORMFITTINGSIN_H
