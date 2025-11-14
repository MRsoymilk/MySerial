#include "mysetting.h"

MySetting &MySetting::getInstance()
{
    static MySetting c_setting;
    return c_setting;
}

MySetting::~MySetting() {}

void MySetting::setValue(SETTING s, const QString &group, const QString &key, const QString &val)
{
    m_settings[s]->setValue(QString("%1/%2").arg(group, key), val);
}

QString MySetting::getValue(SETTING s,
                            const QString &group,
                            const QString &key,
                            const QString &val_dft)
{
    QString val = m_settings[s]->value(QString("%1/%2").arg(group, key)).toString();
    if (val.isEmpty()) {
        val = val_dft;
        m_settings[s]->setValue(QString("%1/%2").arg(group, key), val_dft);
    }
    return val;
}

QStringList MySetting::getGroups(SETTING s)
{
    return m_settings[s]->childGroups();
}

void MySetting::sync(SETTING s)
{
    m_settings[s]->sync();
}

MySetting::MySetting()
{
    m_settings[SETTING::CONFIG] = new QSettings("config/config.ini", QSettings::IniFormat);
    m_settings[SETTING::FRAME] = new QSettings("config/frame.ini", QSettings::IniFormat);
    m_settings[SETTING::FRAME_F30_SINGLE] = new QSettings("config/frame_F30_single.ini",
                                                          QSettings::IniFormat);
    m_settings[SETTING::FRAME_F30_CURVES] = new QSettings("config/frame_F30_curves.ini",
                                                          QSettings::IniFormat);
}
