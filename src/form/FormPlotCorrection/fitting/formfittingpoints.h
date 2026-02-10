#ifndef FORMFITTINGPOINTS_H
#define FORMFITTINGPOINTS_H

#include <QStandardItem>
#include <QWidget>

namespace Ui {
class FormFittingPoints;
}

class FormFittingPoints : public QWidget
{
    Q_OBJECT
public:
    enum ITEM { WAVELENGTH, FILE_NAME, FILE_PATH, STATUS, INTENSITY };
    enum ITEM_STATUS { NOT_COLLECTED, COLLECTED, VALUE_WRITE };
    QStringList item_status{tr("not_collected"), tr("collected"), tr("value_write")};

public:
    explicit FormFittingPoints(QWidget *parent = nullptr);
    ~FormFittingPoints();

    void retranslateUI();
    void updateCollectionStatus(bool status);
    void setTargetIntensity(const double &avg);

signals:
    void toCollectionFittingPoints(const QString &dir, const QString &file, const int &count);
    void doFile(const QString &path);

private slots:
    void on_tBtnSelectDir_clicked();
    void on_btnCollect_clicked();
    void on_tBtnRefresh_clicked();
    void on_tableViewCollectStatus_clicked(const QModelIndex &index);
    void on_tableViewCollectStatus_doubleClicked(const QModelIndex &index);
    void onTableContextMenu(const QPoint &pos);
    void exportWavelengthIntensityToCsv();

private:
    void init();
    void refreshCollectTable();

private:
    Ui::FormFittingPoints *ui;
    QStandardItemModel *m_collectModel = nullptr;
};

#endif // FORMFITTINGPOINTS_H
