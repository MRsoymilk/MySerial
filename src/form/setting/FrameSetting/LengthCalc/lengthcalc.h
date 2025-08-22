#ifndef LENGTHCALC_H
#define LENGTHCALC_H

#include <QDialog>

namespace Ui {
class LengthCalc;
}

class LengthCalc : public QDialog
{
    Q_OBJECT

public:
    explicit LengthCalc(QWidget *parent = nullptr);
    ~LengthCalc();
    void setFrame(const QString &head, const QString &foot);
    int getLength();

private slots:
    void on_btnCalculate_clicked();

private:
    Ui::LengthCalc *ui;
    QString m_head;
    QString m_foot;
    int m_length;
};

#endif // LENGTHCALC_H
