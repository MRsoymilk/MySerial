#include "datainput.h"
#include "ui_datainput.h"

#include <QRegularExpression>

DataInput::DataInput(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DataInput)
{
    ui->setupUi(this);
}

DataInput::~DataInput()
{
    delete ui;
}

QVector<int> DataInput::getValues()
{
    return m_values;
}

void DataInput::on_textEdit_textChanged()
{
    QString txt = ui->textEdit->toPlainText().trimmed();

    QStringList parts = txt.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);

    m_values.reserve(parts.size());
    for (const QString &p : parts) {
        if (!p.isEmpty()) {
            m_values.append(p.toInt());
        }
    }
}
