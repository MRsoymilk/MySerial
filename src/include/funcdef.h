#ifndef FUNCDEF_H
#define FUNCDEF_H

#include "keydef.h"

// FUNCTION ===================================================================

#include <QDateTime>
#define TIMESTAMP(format) (QDateTime::currentDateTime().toString(format))

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
            hexList << QString("0x%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper(); \
        } \
        return hexList.join(" "); \
    })(data)

#include "../common/mysetting.h"
#define SETTING_CONFIG_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::CONFIG, group, key, ##__VA_ARGS__)
#define SETTING_CONFIG_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::CONFIG, group, key, value)
#define SETTING_CONFIG_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::CONFIG)
#define SETTING_CONFIG_SYNC() MY_SETTING.sync(MySetting::SETTING::CONFIG)

#define SETTING_FRAME_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::FRAME, group, key, ##__VA_ARGS__)
#define SETTING_FRAME_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::FRAME, group, key, value)
#define SETTING_FRAME_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::FRAME)
#define SETTING_FRAME_SYNC() MY_SETTING.sync(MySetting::SETTING::FRAME)

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QTimer>
#define SHOW_AUTO_CLOSE_MSGBOX(PARENT, TITLE, TEXT) \
    do { \
        QMessageBox *msgBox = new QMessageBox(PARENT); \
        msgBox->setWindowTitle(TITLE); \
        msgBox->setText(TEXT); \
        msgBox->setAttribute(Qt::WA_DeleteOnClose); \
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
            }); \
        }); \
    } while (0)

// FUNCTION ===================================================================

#endif
