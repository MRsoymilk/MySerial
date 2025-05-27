#include "mylog.h"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> MyLog::s_logger = nullptr;

MyLog &MyLog::getInstance()
{
    static MyLog instance;
    return instance;
}

MyLog::~MyLog() { spdlog::drop_all(); }

void MyLog::init(const std::string &file_name, size_t max_size, size_t max_files)
{
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file_name, max_size, max_files));

    logSinks[0]->set_pattern("%^[%T] %n: %v%$");
    logSinks[1]->set_pattern("[%T] [%l] %n: %v");

    s_logger = std::make_shared<spdlog::logger>("MySerial", begin(logSinks), end(logSinks));
    spdlog::register_logger(s_logger);
    s_logger->set_level(spdlog::level::trace);
    s_logger->flush_on(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger> MyLog::getLogger() { return s_logger; }

MyLog::MyLog() { init(); }
