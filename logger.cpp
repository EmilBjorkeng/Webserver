#include <iostream>
#include <iomanip>

#include "include/logger.hpp"

Logger::Logger() : sessionID("") {
    // Default file
    logFile.open("debug.log", std::ios::app); // Open in append mode
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: debug.log" << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open())
    { logFile.close(); }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open())
    {
        if (!sessionID.empty()) // Only add session ID if it's not empty
            logFile << "[" << sessionID << "] ";

        logFile << "[" << getTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;
    }
}

void Logger::setSessionID(const std::string& newSessionID) {
    sessionID = newSessionID;
}

void Logger::space() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open())
    {
        logFile << std::endl;
    }
}

void Logger::setOutputFile(const std::string& filename) {
    if (logFile.is_open())
    { logFile.close(); }
    logFile.open(filename, std::ios::app); // Open in append mode
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

