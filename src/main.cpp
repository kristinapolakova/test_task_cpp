#include "../include/logger.h"

#define COLOR_CYAN "\033[36m"
#define RESET "\033[0m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_GREEN "\033[1;32m"
#define BOLD_ORANGE "\033[1;38;5;208m"

struct Task
{
    std::string message;
    LogLevel level;

    Task(const std::string &msg, LogLevel lvl) : message(msg), level(lvl) {}
};

class TaskQueue
{
public:
    void push(const Task &task)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(task);
        }
        cv.notify_one();
    }

    bool pop(Task &task)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]
                { return !tasks.empty() || stop; });

        if (tasks.empty())
        {
            return false;
        }

        task = tasks.front();
        tasks.pop();
        return true;
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return tasks.size();
    }

private:
    std::queue<Task> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
};

void worker_thread(TaskQueue &tq, Logger &log)
{
    try
    {
        while (true)
        {
            Task task("", LOG_INFO);
            if (!tq.pop(task))
            {
                break;
            }
            log.write(task.message, task.level);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Worker thread error: " << e.what() << '\n';
    }
}

LogLevel parse_level(const std::string &s)
{
    std::string str = s;
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    if (first != std::string::npos && last != std::string::npos)
    {
        str = str.substr(first, last - first + 1);
    }

    if (str == "DEBUG")
        return LOG_DEBUG;
    if (str == "INFO")
        return LOG_INFO;
    if (str == "ERROR")
        return LOG_ERROR;
    return LOG_INFO;
}

std::string level_str(LogLevel l)
{
    switch (l)
    {
    case LOG_DEBUG:
        return "DEBUG";
    case LOG_INFO:
        return "INFO";
    case LOG_ERROR:
        return "ERROR";
    default:
        return "INFO";
    }
}

std::string clean_input(const std::string &input)
{
    std::string result;
    for (char c : input)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc >= 32 || uc == '\t' || uc == '\r' || uc == '\n')
        {
            result += c;
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <log_file> <level>\n";
        std::cerr << "Levels: DEBUG, INFO, ERROR\n";
        return 1;
    }

    try
    {
        std::string filename = argv[1];
        LogLevel default_level = parse_level(argv[2]);

        std::cout << BOLD_YELLOW << "~~~ System start-up ~~~\n"
                  << RESET;
        std::cout << COLOR_CYAN << "File: " << filename << '\n'
                  << RESET;
        std::cout << COLOR_CYAN << "Default level: " << level_str(default_level) << '\n'
                  << RESET;
        std::cout << '\n';

        Logger logger(filename, default_level);

        TaskQueue task_queue;
        std::thread worker(worker_thread, std::ref(task_queue), std::ref(logger));

        std::cout << "Enter messages. Formats:\n";
        std::cout << "  simple message            - default level\n";
        std::cout << "  [DEBUG] message           - DEBUG level\n";
        std::cout << "  [INFO] message            - INFO level\n";
        std::cout << "  [ERROR] message           - ERROR level\n";
        std::cout << "  level DEBUG|INFO|ERROR    - change default level\n";
        std::cout << "  exit                      - quit\n";
        std::cout << "----------------------------------------\n";

        std::string line;
        while (true)
        {
            std::cout << "> " << std::flush;

            if (!std::getline(std::cin, line))
            {
                break;
            }

            std::string cleaned = clean_input(line);

            if (cleaned == "exit")
            {
                break;
            }

            if (cleaned.empty())
            {
                continue;
            }

            try
            {
                if (cleaned.size() >= 6 && cleaned.substr(0, 6) == "level ")
                {
                    std::string new_level_str = cleaned.substr(6);
                    LogLevel new_level = parse_level(new_level_str);
                    logger.set_default_level(new_level);
                    default_level = new_level;
                    std::cout << BOLD_ORANGE << "Default level changed to " << level_str(new_level) << "\n"
                              << RESET;
                    continue;
                }

                LogLevel level = default_level;
                std::string message = cleaned;

                if (cleaned[0] == '[')
                {
                    size_t end = cleaned.find(']');
                    if (end != std::string::npos)
                    {
                        std::string level_part = cleaned.substr(1, end - 1);
                        LogLevel parsed_level = parse_level(level_part);

                        if (end + 1 < cleaned.size())
                        {
                            message = cleaned.substr(end + 1);
                            size_t start = message.find_first_not_of(" \t");
                            if (start != std::string::npos)
                            {
                                message = message.substr(start);
                            }
                            else
                            {
                                message = "";
                            }
                            level = parsed_level;
                        }
                    }
                }

                if (!message.empty())
                {
                    task_queue.push(Task(message, level));
                    std::cout << COLOR_CYAN << "Task added (level: " << level_str(level) << "): \"" << message << "\"\n"
                              << RESET;
                }
                else
                {
                    std::cout << "Empty message, ignored\n";
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << "\n";
            }
        }

        std::cout << BOLD_YELLOW << "\nShutting down...\n"
                  << RESET;

        task_queue.shutdown();
        worker.join();

        logger.write("~~~ Logger stopped ~~~", LOG_INFO);
        std::cout << BOLD_GREEN << "Done.\n"
                  << RESET;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}