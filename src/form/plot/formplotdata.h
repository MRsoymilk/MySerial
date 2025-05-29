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
signals:
    void windowClose();

public slots:
    void updateTable(QLineSeries *line);
    void updateTable4k(const QVector<double> &v14, const QVector<double> &v24);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void showContextMenu(const QPoint &pos);
    void exportToCSV();
    void clearData();

private:
    void init();

private:
    Ui::FormPlotData *ui;
    QStandardItemModel *m_model;
};

#endif // FORMPLOTDATA_H
