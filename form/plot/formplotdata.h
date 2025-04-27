#ifndef FORMPLOTDATA_H
#define FORMPLOTDATA_H

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
    void updateTable(const QList<QPointF> &points);

private slots:
    void showContextMenu(const QPoint &pos);
    void exportToCSV();

    void clearData();

private:
    void init();

private:
    Ui::FormPlotData *ui;
    QStandardItemModel *model;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // FORMPLOTDATA_H
