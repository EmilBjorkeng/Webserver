#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <mutex>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    DEBUG
};

class Logger {
public:
    static Logger& getInstance(); // Singleton access

    void log(const std::string& message, LogLevel level = LogLevel::INFO);
    void setSessionID(const std::string& sessionID);
    void space();

    void setOutputFile(const std::string& filename);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    std::ofstream logFile;
    std::mutex logMutex;

    std::string sessionID;
    std::string getTimestamp();
    std::string levelToString(LogLevel level);
};

#endif

