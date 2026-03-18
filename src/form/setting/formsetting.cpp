#include "formsetting.h"

#include <QDir>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOperatingSystemVersion>
#include <QProcess>

#include "funcdef.h"
#include "ui_formsetting.h"

FormSetting::FormSetting(QWidget *parent) : QWidget(parent), ui(new Ui::FormSetting) {
    ui->setupUi(this);
    init();
}

FormSetting::~FormSetting() {
    SETTING_CONFIG_SYNC();
    delete ui;
}

void FormSetting::retranslateUI() { ui->retranslateUi(this); }

void FormSetting::initThreshold() {
    bool enable_local_threshold = false;
    if (enable_local_threshold) {
        QString path = ui->lineEditThreshold->text();
        readThresholdCSV(path);
    }

    bool enable_double = false;
    if (ui->radioButtonUseDouble->isChecked()) {
        enable_double = true;
    }
    bool enable_interpolation = false;
    if (ui->checkBoxEnableInterpolation->isChecked()) {
        enable_interpolation = true;
    }
    emit sendThresholdOption({{"enable_double", enable_double}, {"interpolation", enable_interpolation}});
}

void FormSetting::init() {
    QString F30_shown_mode = SETTING_CONFIG_GET(CFG_GROUP_F30_SHOWN, CFG_F30_SHOWN_MODE, CFG_F30_MODE_DOUBLE);
    if (F30_shown_mode == CFG_F30_MODE_DOUBLE) {
        ui->radioButtonUseDouble->setChecked(true);
    } else if (F30_shown_mode == CFG_F30_MODE_SINGLE) {
        ui->radioButtonUseSingle->setChecked(true);
    }
    QString path = SETTING_CONFIG_GET(CFG_GROUP_F30_SHOWN, CFG_F30_MODE_DOUBLE_THRESHOLD);
    ui->lineEditThreshold->setText(path);
    QString status = SETTING_CONFIG_GET(CFG_GROUP_F30_SHOWN, CFG_F30_MODE_DOUBLE_INTERPOLATION, VAL_ENABLE);
    if (status == VAL_ENABLE) {
        ui->checkBoxEnableInterpolation->setChecked(true);
    }
    qApp->setProperty("debug", "disable");
}

void FormSetting::closeEvent(QCloseEvent *event) { emit windowClose(); }

void FormSetting::on_radioButtonUseSingle_clicked(bool checked) {
    if (checked) {
        SETTING_CONFIG_SET(CFG_GROUP_F30_SHOWN, CFG_F30_SHOWN_MODE, CFG_F30_MODE_SINGLE);
        emit sendThreshold(false, {});
    }
}

void FormSetting::on_radioButtonUseDouble_clicked(bool checked) {
    if (checked) {
        SETTING_CONFIG_SET(CFG_GROUP_F30_SHOWN, CFG_F30_SHOWN_MODE, CFG_F30_MODE_DOUBLE);
    }
}

void FormSetting::readThresholdCSV(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, TITLE_ERROR, tr("can't open file: %1").arg(fileName));
        return;
    }
    SETTING_CONFIG_SET(CFG_GROUP_F30_SHOWN, CFG_F30_MODE_DOUBLE_THRESHOLD, fileName);

    QTextStream in(&file);

    QList<double> values;

    bool firstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        if (firstLine) {
            firstLine = false;
            continue;
        }

        QStringList parts = line.split(",");
        if (parts.size() < 2) continue;

        double thresholdVal = parts[1].toDouble();

        values.append(thresholdVal);
    }

    file.close();

    emit sendThreshold(true, values);
}

void FormSetting::on_tBtnLoadThreshold_clicked() {
    QString fileName =
        QFileDialog::getOpenFileName(this, "choose CSV file", QDir::homePath(), "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty()) {
        QMessageBox::warning(this, TITLE_ERROR, tr("empty path: %1").arg(fileName));
        return;
    }
    ui->lineEditThreshold->setText(fileName);
    SETTING_CONFIG_SET(CFG_GROUP_F30_SHOWN, CFG_F30_MODE_DOUBLE_THRESHOLD, fileName);
    readThresholdCSV(fileName);
}

void FormSetting::on_checkBoxEnableInterpolation_clicked() {
    if (ui->checkBoxEnableInterpolation->isChecked()) {
        emit sendThresholdOption({{"interpolation", true}});
        SETTING_CONFIG_SET(CFG_GROUP_F30_SHOWN, CFG_F30_MODE_DOUBLE_INTERPOLATION, VAL_ENABLE);
    } else {
        emit sendThresholdOption({{"interpolation", false}});
        SETTING_CONFIG_SET(CFG_GROUP_F30_SHOWN, CFG_F30_MODE_DOUBLE_INTERPOLATION, VAL_DISABLE);
    }
}

void FormSetting::on_checkBoxEnableDebug_checkStateChanged(const Qt::CheckState &state) {
    if (state == Qt::Checked) {
        qApp->setProperty("debug", "enable");
    } else {
        qApp->setProperty("debug", "disable");
    }
}

void FormSetting::on_checkBoxEnableLocalThreshold_clicked() {
    if (ui->checkBoxEnableLocalThreshold->isChecked()) {
    } else {
    }
}
