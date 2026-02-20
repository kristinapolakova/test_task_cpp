#include "../include/logger.h"

Logger::Logger(const std::string &filename, LogLevel level)
    : default_level(level)
{
    file.open(filename, std::ios::app);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open log file: " + filename);
    }
    file.rdbuf()->pubsetbuf(nullptr, 0);
}

Logger::~Logger()
{
    if (file.is_open())
    {
        file.close();
    }
}

std::string Logger::get_current_time()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
    return std::string(buffer);
}

std::string Logger::level_to_string(LogLevel level)
{
    switch (level)
    {
    case LOG_DEBUG:
        return "DEBUG";
    case LOG_INFO:
        return "INFO";
    case LOG_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

bool Logger::should_write(LogLevel message_level) const
{
    std::lock_guard<std::mutex> lock(mtx);
    return message_level >= default_level;
}

void Logger::set_default_level(LogLevel level)
{
    std::lock_guard<std::mutex> lock(mtx);
    default_level = level;
}

LogLevel Logger::get_default_level() const
{
    std::lock_guard<std::mutex> lock(mtx);
    return default_level;
}

void Logger::write(const std::string &message, LogLevel level)
{
    if (!should_write(level))
    {
        return;
    }

    std::string log_line = "[" + get_current_time() + "] " +
                           "[" + level_to_string(level) + "] " +
                           message + '\n';

    std::lock_guard<std::mutex> lock(mtx);

    if (!file.is_open())
    {
        std::cerr << "Error: log file is closed!" << '\n';
        return;
    }

    file.write(log_line.c_str(), log_line.size());
    file.flush();
}