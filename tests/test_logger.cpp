#include "../include/logger.h"

#define COLOR_GREEN "\033[32m"
#define COLOR_RESET "\033[0m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED "\033[31m"

bool file_exists(const std::string &name)
{
    std::ifstream f(name.c_str());
    return f.good();
}

int count_lines(const std::string &filename)
{
    std::ifstream f(filename);
    std::string line;
    int count = 0;
    while (std::getline(f, line))
    {
        count++;
    }
    return count;
}

bool file_contains(const std::string &filename, const std::string &text)
{
    std::ifstream f(filename);
    std::string line;
    while (std::getline(f, line))
    {
        if (line.find(text) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

void test_logger()
{
    std::cout << COLOR_YELLOW << "=== Running tests ===\n"
              << COLOR_RESET;

    const std::string test_file = "test.log";
    std::remove(test_file.c_str());

    std::cout << "Test 1: Creating logger... ";
    try
    {
        Logger log(test_file, LOG_INFO);
        std::cout << COLOR_GREEN << "PASSED\n"
                  << COLOR_RESET;
    }
    catch (...)
    {
        std::cout << COLOR_RED << "FAILED\n"
                  << COLOR_RESET;
        return;
    }

    std::cout << "Test 2: Writing message... ";
    {
        Logger log(test_file, LOG_INFO);
        log.write("Hello, World!", LOG_INFO);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (file_exists(test_file) && file_contains(test_file, "Hello, World!"))
        {
            std::cout << COLOR_GREEN << "PASSED\n"
                      << COLOR_RESET;
        }
        else
        {
            std::cout << COLOR_RED << "FAILED\n"
                      << COLOR_RESET;
        }
    }

    std::cout << "Test 3: Level filtering... ";
    {
        std::remove(test_file.c_str());
        Logger log(test_file, LOG_INFO);
        log.write("Debug message", LOG_DEBUG);
        log.write("Info message", LOG_INFO);
        log.write("Error message", LOG_ERROR);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        int lines = count_lines(test_file);
        if (lines == 2 && !file_contains(test_file, "Debug"))
        {
            std::cout << COLOR_GREEN << "PASSED\n"
                      << COLOR_RESET;
        }
        else
        {
            std::cout << COLOR_RED << "FAILED (lines=" << lines << ")\n"
                      << COLOR_RESET;
        }
    }

    std::cout << "Test 4: Changing level... ";
    {
        std::remove(test_file.c_str());
        Logger log(test_file, LOG_ERROR);
        log.write("Error message", LOG_ERROR);
        log.set_default_level(LOG_DEBUG);
        log.write("Debug message", LOG_DEBUG);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        int lines = count_lines(test_file);
        if (lines == 2)
        {
            std::cout << COLOR_GREEN << "PASSED\n"
                      << COLOR_RESET;
        }
        else
        {
            std::cout << COLOR_RED << "FAILED (lines=" << lines << ")\n"
                      << COLOR_RESET;
        }
    }

    std::cout << "Test 5: Thread safety... ";
    {
        std::remove(test_file.c_str());
        Logger log(test_file, LOG_INFO);

        std::vector<std::thread> threads;
        for (int i = 0; i < 5; i++)
        {
            threads.emplace_back([&log, i]()
                                 {
                for (int j = 0; j < 10; j++) {
                    log.write("Thread " + std::to_string(i) + " message", LOG_INFO);
                } });
        }

        for (auto &t : threads)
        {
            t.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        int lines = count_lines(test_file);
        if (lines == 50)
        {
            std::cout << COLOR_GREEN << "PASSED\n"
                      << COLOR_RESET;
        }
        else
        {
            std::cout << COLOR_RED << "FAILED (expected 50, got " << lines << ")\n"
                      << COLOR_RESET;
        }
    }

    std::remove(test_file.c_str());
    std::cout << COLOR_GREEN << "=== Tests completed ===\n"
              << COLOR_RESET;
    ;
}

int main()
{
    test_logger();
    return 0;
}