#ifndef LINESEND_H
#define LINESEND_H

#include <QWidget>

namespace Ui {
class LineSend;
}

class LineSend : public QWidget
{
    Q_OBJECT

public:
    explicit LineSend(int index, QWidget *parent = nullptr);
    ~LineSend();
    void setLabel(const QString &label);
    void setCmd(const QString &cmd);
    void setBtn(const QString &name);

    QString getLabel() const;
    QString getCmd() const;

signals:
    void cmdEdited(int index, const QString &cmd);
    void labelEdited(int index, const QString &label);
    void cmdSendClicked(int index, const QString &cmd);

private slots:
    void on_lineEdit_label_editingFinished();
    void on_lineEdit_cmd_editingFinished();
    void on_tBtn_clicked();

private:
    Ui::LineSend *ui;
    int m_index;
};

#endif // LINESEND_H
