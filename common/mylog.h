#ifndef MYLOG_H
#define MYLOG_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <memory>

#include "spdlog/spdlog.h"

class MyLog
{
   public:
    static MyLog &getInstance();
    ~MyLog();
    void init(const std::string &file_name = "./log/myYolo.log", size_t max_size = 10 * 1024 * 1024,
              size_t max_files = 10);
    static std::shared_ptr<spdlog::logger> getLogger();

   private:
    MyLog();
    static std::shared_ptr<spdlog::logger> s_logger;
};

template<>
struct fmt::formatter<QString> : formatter<std::string>
{
    template<typename FormatContext>
    auto format(QString s, FormatContext &ctx) const
    {
        return formatter<std::string>::format(s.toStdString(), ctx);
    }
};

template<>
struct fmt::formatter<QByteArray> : fmt::formatter<std::string>
{
    template<typename FormatContext>
    auto format(const QByteArray &s, FormatContext &ctx) const
    {
        return fmt::formatter<std::string>::format(s.toStdString(), ctx);
    }
};

template <>
struct fmt::formatter<QStringList> : formatter<std::string>
{
    template <typename FormatContext>
    auto format(QStringList s, FormatContext &ctx) const
    {
        return formatter<std::string>::format(s.join(',').toStdString(), ctx);
    }
};

#define MY_LOG MyLog::getInstance()

#endif  // MYLOG_H
