#ifndef FORMFITTINGARCSIN_H
#define FORMFITTINGARCSIN_H

#include <QStandardItem>
#include <QWidget>

namespace Ui {
class FormFittingArcSin;
}

class FormFittingArcSin : public QWidget
{
    Q_OBJECT

public:
    explicit FormFittingArcSin(QWidget *parent = nullptr);
    ~FormFittingArcSin();
    void updateParams();
    void retranslateUI();

signals:
    void sendSin(const QByteArray &bytes);

private slots:
    void on_btnSendR_clicked();
    void on_btnCalculate_k1b1k2b2_clicked();
    void on_btnFittingArcSin_clicked();
    void on_btnSendFormula_clicked();
    void on_btnSetTemperatureT_clicked();
    void on_btnUpdateFormula_clicked();
    void on_btnGenerateThreshold_clicked();
    void on_tBtnAdd_clicked();
    void on_tBtnDelete_clicked();

    void on_radioButtonRight_clicked();
    void on_radioButtonLeft_clicked();

    void exportThresholdToHex();
    void exportThresholdToArray();

private:
    void init();
    void showContextMenu(const QPoint &pos);
    void exportThresholdToCSV();
    double calculate(const double &idx);
    void updateFormulaLambda();

private:
    Ui::FormFittingArcSin *ui;
    struct FORMULA_LAMBDA
    {
        double k;
        double b;
        double d;
        double alpha;
    } m_formula_lambda_l, m_formula_lambda_r;
    struct FORMULA_Y
    {
        double k1;
        double b1;
        double k2;
        double b2;
        double T;
    } m_formula_y;
    QStandardItemModel *m_modelPoints;
    QStandardItemModel *m_modelThreshold;
    QString m_urlFitArcSin;
};

#endif // FORMFITTINGARCSIN_H
