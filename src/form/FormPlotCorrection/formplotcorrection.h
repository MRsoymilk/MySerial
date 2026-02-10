#ifndef FORMPLOTCORRECTION_H
#define FORMPLOTCORRECTION_H

#include <QWidget>
#include "global.h"

class FormFittingKB;
class FormFittingSin;
class FormFittingSelf;
class FormFittingArcSin;
class FormFittingPoints;
class ShowCorrectionCurve;

namespace Ui {
class FormPlotCorrection;
}

class FormPlotCorrection : public QWidget
{
    Q_OBJECT
public:
    explicit FormPlotCorrection(QWidget *parent = nullptr);
    ~FormPlotCorrection();
    void retranslateUI();

signals:
    void toExternalSpectral(const QJsonObject &spectrum);
    void windowClose();
    void sendKB(const QByteArray &bytes);
    void sendSin(const QByteArray &bytes);
    void sendSerialArcSin(const QByteArray &bytes);
    void sendParamsArcSin(const PARAMS_ARCSIN &params);
    void onShowCorrectionCurve(const QList<QPointF> &data,
                               const double &xMin,
                               const double &xMax,
                               const double &yMin,
                               const double &yMax,
                               const double &temperature);
    void useLoadedThreshold(bool isUse, QVector<double> v);
    void useLoadedThreadsholdOption(const QJsonObject &option);
    void useAutoUpdateThreshold(bool isUse);
    void toCollectionFittingPoints(const QString &dir, const QString &file, const int &count);
    void doFile(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void onEpochCorrection(const QVector<double> &v14, const QVector<double> &v24);
    void onTemperature(double temperature);
    void onThresholdStatus(const QString &status);
    void onCollectionFitingPointsFinish(bool status);

private slots:
    void on_btnStart_clicked();
    void on_comboBoxAlgorithm_currentTextChanged(const QString &arg1);
    void on_tBtnShowCorrectionCurve_clicked();

private:
    void init();
    void findV14_MaxMinIdx(const QVector<double> &v14, int &idx_min, int &idx_max);

private:
    Ui::FormPlotCorrection *ui;
    FormFittingKB *m_formKB;
    FormFittingSin *m_formSin;
    FormFittingSelf *m_formSelf;
    FormFittingArcSin *m_formArcSin;
    FormFittingPoints *m_formPoints;
    bool m_start;
    bool m_show = false;

    ShowCorrectionCurve *sinShow;
    ShowCorrectionCurve *arcSinShow;
};

#endif // FORMPLOTCORRECTION_H
