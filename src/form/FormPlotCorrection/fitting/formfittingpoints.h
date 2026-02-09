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
    explicit FormFittingPoints(QWidget *parent = nullptr);
    ~FormFittingPoints();

    void retranslateUI();
    void updateCollectionStatus(bool status);
signals:
    void toCollectionFittingPoints(const QString &dir, const QString &file, const int &count);

private slots:
    void on_tBtnSelectDir_clicked();
    void on_btnCollect_clicked();
    void on_tBtnRefresh_clicked();
    void on_tableViewCollectStatus_clicked(const QModelIndex &index);

private:
    void init();
    void refreshCollectTable();

private:
    Ui::FormFittingPoints *ui;
    QStandardItemModel *m_collectModel = nullptr;
};

#endif // FORMFITTINGPOINTS_H
