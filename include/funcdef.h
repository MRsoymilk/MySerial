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

#include "../common/mysetting.h"
#define SETTING_GET(group, key, ...) MY_SETTING.getValue(group, key, ##__VA_ARGS__)
#define SETTING_SET(group, key, value) MY_SETTING.setValue(group, key, value)
#define SETTING_SYNC() MY_SETTING.sync();

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QTimer>
#define SHOW_AUTO_CLOSE_MSGBOX(PARENT, TITLE, TEXT) \
    do { \
        QMessageBox *msgBox = new QMessageBox(PARENT); \
        msgBox->setWindowTitle(TITLE); \
        msgBox->setText(TEXT); \
        msgBox->setIcon(QMessageBox::Warning); \
        msgBox->setAttribute(Qt::WA_DeleteOnClose); \
        msgBox->show(); \
\
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(msgBox); \
        msgBox->setGraphicsEffect(effect); \
\
        QPropertyAnimation *fadeAnim = new QPropertyAnimation(effect, "opacity", msgBox); \
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
