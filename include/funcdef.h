#ifndef FUNCDEF_H
#define FUNCDEF_H

#include "keydef.h"

// FUNCTION ===================================================================

#include <QDateTime>
#define TIMESTAMP(format) (QDateTime::currentDateTime().toString(format))

#include "../common/mysetting.h"
#define SETTING_GET(group, key, ...) MY_SETTING.getValue(group, key, ##__VA_ARGS__)
#define SETTING_SET(group, key, value) MY_SETTING.setValue(group, key, value)

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
