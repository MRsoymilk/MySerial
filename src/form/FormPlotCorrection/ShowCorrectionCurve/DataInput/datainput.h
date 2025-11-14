#ifndef DATAINPUT_H
#define DATAINPUT_H

#include <QDialog>

namespace Ui {
class DataInput;
}

class DataInput : public QDialog
{
    Q_OBJECT

public:
    explicit DataInput(QWidget *parent = nullptr);
    ~DataInput();
    QVector<double> getValues();

private slots:
    void on_textEdit_textChanged();

private:
    Ui::DataInput *ui;
    QVector<double> m_values;
};

#endif // DATAINPUT_H
