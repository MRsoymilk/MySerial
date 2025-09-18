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
    QVector<int> getValues();

private slots:
    void on_textEdit_textChanged();

private:
    Ui::DataInput *ui;
    QVector<int> m_values;
};

#endif // DATAINPUT_H
