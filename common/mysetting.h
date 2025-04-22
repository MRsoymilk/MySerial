#ifndef MYSETTING_H
#define MYSETTING_H

#include <QSettings>
#include <QString>

class MySetting
{
   public:
    static MySetting &getInstance();
    ~MySetting();
    void setValue(const QString &group, const QString &key, const QString &val);
    QString getValue(const QString &group, const QString &key, const QString &val_dft = "");

   private:
    MySetting();

   private:
    QSettings *m_settings;
};

#define MY_SETTING MySetting::getInstance()

#endif  // MYSETTING_H
