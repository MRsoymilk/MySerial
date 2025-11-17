#ifndef MYSETTING_H
#define MYSETTING_H

#include <QSettings>
#include <QString>

class MySetting
{
public:
    enum class SETTING {
        CONFIG,
        FRAME_F15_SINGLE,
        FRAME_F15_CURVES,
        FRAME_F30_SINGLE,
        FRAME_F30_CURVES,
        PLAY_MPU6050,
    };

public:
    static MySetting &getInstance();
    ~MySetting();
    void setValue(SETTING s, const QString &group, const QString &key, const QString &val);
    QString getValue(SETTING s,
                     const QString &group,
                     const QString &key,
                     const QString &val_dft = "");
    QStringList getGroups(SETTING s);
    void sync(SETTING s);

private:
    MySetting();

private:
    QMap<SETTING, QSettings *> m_settings;
};

#define MY_SETTING MySetting::getInstance()

#endif  // MYSETTING_H
