#ifndef FUNCDEF_H
#define FUNCDEF_H

#include "keydef.h"

// FUNCTION ===================================================================

#include <QDateTime>
#define TIMESTAMP_0() (QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
#define TIMESTAMP_1(format) (QDateTime::currentDateTime().toString(format))
#define TIMESTAMP(...) TIMESTAMP_MACRO(__VA_ARGS__, TIMESTAMP_1, TIMESTAMP_0)(__VA_ARGS__)

#define TIMESTAMP_MACRO(_1, _2, NAME, ...) NAME

#include "../common/mylog.h"
#define LOG_TRACE(...) MY_LOG.getLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) MY_LOG.getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) MY_LOG.getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) MY_LOG.getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) MY_LOG.getLogger()->critical(__VA_ARGS__)
#define FORMAT_HEX(data) \
    ([](const QByteArray &d) { \
        QStringList hexList; \
        for (uchar byte : d) { \
            hexList << QString("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper(); \
        } \
        return hexList.join(""); \
    })(data)

#include "../common/mysetting.h"
#define SETTING_FRAME_F30Single_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::FRAME_F30_SINGLE, group, key, ##__VA_ARGS__)
#define SETTING_FRAME_F30Single_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::FRAME_F30_SINGLE, group, key, value)
#define SETTING_FRAME_F30Single_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::FRAME_F30_SINGLE)
#define SETTING_FRAME_F30Single_SYNC() MY_SETTING.sync(MySetting::SETTING::FRAME_F30_SINGLE)

#define SETTING_FRAME_F30Curves_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::FRAME_F30_CURVES, group, key, ##__VA_ARGS__)
#define SETTING_FRAME_F30Curves_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::FRAME_F30_CURVES, group, key, value)
#define SETTING_FRAME_F30Curves_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::FRAME_F30_CURVES)
#define SETTING_FRAME_F30Curves_SYNC() MY_SETTING.sync(MySetting::SETTING::FRAME_F30_CURVES)

#define SETTING_FRAME_F15Curves_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::FRAME_F15_CURVES, group, key, ##__VA_ARGS__)
#define SETTING_FRAME_F15Curves_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::FRAME_F15_CURVES, group, key, value)
#define SETTING_FRAME_F15Curves_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::FRAME_F15_CURVES)
#define SETTING_FRAME_F15Curves_SYNC() MY_SETTING.sync(MySetting::SETTING::FRAME_F15_CURVES)

#define SETTING_FRAME_F15Single_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::FRAME_F15_SINGLE, group, key, ##__VA_ARGS__)
#define SETTING_FRAME_F15Single_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::FRAME_F15_SINGLE, group, key, value)
#define SETTING_FRAME_F15Single_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::FRAME_F15_SINGLE)
#define SETTING_FRAME_F15Single_SYNC() MY_SETTING.sync(MySetting::SETTING::FRAME_F15_SINGLE)

#define SETTING_FRAME_LLC_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::FRAME_LLC_CURVES, group, key, ##__VA_ARGS__)
#define SETTING_FRAME_LLC_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::FRAME_LLC_CURVES, group, key, value)
#define SETTING_FRAME_LLC_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::FRAME_LLC_CURVES)
#define SETTING_FRAME_LLC_SYNC() MY_SETTING.sync(MySetting::SETTING::FRAME_LLC_CURVES)

#define SETTING_FRAME_PLAY_MPU6050_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::PLAY_MPU6050, group, key, ##__VA_ARGS__)
#define SETTING_FRAME_PLAY_MPU6050_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::PLAY_MPU6050, group, key, value)
#define SETTING_FRAME_PLAY_MPU6050_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::PLAY_MPU6050)
#define SETTING_FRAME_PLAY_MPU6050_SYNC() MY_SETTING.sync(MySetting::SETTING::PLAY_MPU6050)

#define SETTING_CONFIG_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::CONFIG, group, key, ##__VA_ARGS__)
#define SETTING_CONFIG_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::CONFIG, group, key, value)
#define SETTING_CONFIG_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::CONFIG)
#define SETTING_CONFIG_SYNC() MY_SETTING.sync(MySetting::SETTING::CONFIG)

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QTimer>
#define SHOW_AUTO_CLOSE_MSGBOX(PARENT, TITLE, TEXT) \
    do { \
        QMessageBox *msgBox = new QMessageBox(PARENT); \
        msgBox->setWindowTitle(TITLE); \
        msgBox->setText(TEXT); \
        msgBox->setAttribute(Qt::WA_DeleteOnClose, false); \
        msgBox->show(); \
\
        QPropertyAnimation *fadeAnim = new QPropertyAnimation(msgBox, "windowOpacity", msgBox); \
        fadeAnim->setDuration(1000); \
        fadeAnim->setStartValue(1.0); \
        fadeAnim->setEndValue(0.0); \
        fadeAnim->setEasingCurve(QEasingCurve::InOutQuad); \
\
        QTimer::singleShot(1000, [msgBox, fadeAnim]() { \
            fadeAnim->start(); \
            QObject::connect(fadeAnim, &QPropertyAnimation::finished, msgBox, [msgBox]() { \
                msgBox->close(); \
                msgBox->deleteLater(); \
            }); \
        }); \
    } while (0)

// FUNCTION ===================================================================

#endif
