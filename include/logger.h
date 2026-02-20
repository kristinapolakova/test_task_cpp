#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <chrono>
#include <ctime>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cctype>
#include <atomic>
#include <csignal>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

enum LogLevel
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR
};

class Logger
{
public:
    Logger(const std::string &filename, LogLevel default_level);
    ~Logger();

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    void write(const std::string &message, LogLevel level);
    void set_default_level(LogLevel level);
    LogLevel get_default_level() const;

private:
    std::string get_current_time();
    std::string level_to_string(LogLevel level);
    bool should_write(LogLevel message_level) const;

private:
    std::ofstream file;
    LogLevel default_level;
    mutable std::mutex mtx;
};

#endif