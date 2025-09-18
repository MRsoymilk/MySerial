#ifndef FORMTIP_H
#define FORMTIP_H

#include <QWidget>

namespace Ui {
class FormTip;
}

class FormTip : public QWidget
{
    Q_OBJECT

public:
    explicit FormTip(QWidget *parent = nullptr);
    ~FormTip();
    void retranslateUI();

public slots:
    void onFetchUpdates(const QString &url);

private:
    Ui::FormTip *ui;
};

#endif // FORMTIP_H
