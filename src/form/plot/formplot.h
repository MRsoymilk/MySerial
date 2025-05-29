#ifndef FORMPLOT_H
#define FORMPLOT_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataProxy>
#include "enhance/mychartview.h"
#include "formplotdata.h"
#include "formplothistory.h"
#include "formplotsimulate.h"
#include "plotworker.h"

namespace Ui {
class FormPlot;
}

class FormPlot : public QWidget
{
    Q_OBJECT
public:
    enum class PLOT_ALGORITHM { NORMAL = 0, MAX_NEG_95 };

public:
    explicit FormPlot(QWidget *parent = nullptr);
    ~FormPlot();

signals:
    void newDataReceived(const QByteArray &data, const QString &name);
    void newDataReceived4k(const QByteArray &data14, const QByteArray &data24);

public slots:
    void onDataReceived(const QByteArray &data, const QString &name);
    void onDataReceived4k(const QByteArray &data14, const QByteArray &data24);

protected:
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void on_tBtnZoom_clicked();
    void on_tBtnData_clicked();
    void on_tBtn3D_clicked();
    void on_tBtnHistory_clicked();
    void on_tBtnSimulate_clicked();
    void plotDataClose();
    void plotHistoryClose();
    void plotSimulateClose();
    void updatePlot(const QString &name,
                    QLineSeries *line,
                    const int &points,
                    const double &min_y,
                    const double &max_y,
                    const double &min_x,
                    const double &max_x);
    void updatePlot4k(const QList<QPointF> &data14,
                      const QList<QPointF> &data24,
                      const double &xMin,
                      const double &xMax,
                      const double &yMin,
                      const double &yMax);
    void on_spinBox14Offset_valueChanged(int val);
    void on_spinBox24Offset_valueChanged(int val);
    void on_comboBoxAlgorithm_currentIndexChanged(int index);

private:
    void init();
    void getINI();
    void setINI();

private:
    Ui::FormPlot *ui;
    QLineSeries *m_series24;
    QLineSeries *m_series14;
    QChart *m_chart;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    bool m_autoZoom = true;
    bool m_showData = false;
    bool m_show3D = false;
    bool m_showHistory = false;
    bool m_showSimulate = false;
    double m_fixedYMin = -2.5;
    double m_fixedYMax = 2.5;
    QThread *m_workerThread;
    PlotWorker *m_worker;
    FormPlotData *m_plotData;
    FormPlotHistory *m_plotHistory;
    FormPlotSimulate *m_plotSimulate;

    Q3DSurface *m_surface = nullptr;
    QSurface3DSeries *m_surfaceSeries24 = nullptr;
    QSurface3DSeries *m_surfaceSeries14 = nullptr;
    QSurfaceDataProxy *m_surfaceProxy24 = nullptr;
    QSurfaceDataProxy *m_surfaceProxy14 = nullptr;
    QWidget *m_surfaceWidget = nullptr;
    QValue3DAxis *m_surfaceAxisX;
    QValue3DAxis *m_surfaceAxisY;
    QValue3DAxis *m_surfaceAxisZ;

    MyChartView *m_chartView = nullptr;

    QList<QList<QPointF>> m_points24;
    QList<QList<QPointF>> m_points14;
};

#endif // FORMPLOT_H
