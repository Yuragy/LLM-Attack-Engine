#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <map>
#include <vector>
#include <unordered_set>
#include <memory>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "db_manager.h"
#include "config.h"  

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static void initialize(const std::string& configFilePath);
    static void shutdown();
    static void log(const std::string& message, LogLevel level = LogLevel::INFO, const std::unordered_set<std::string>& tags = {});
    static void trace(const std::string& message, const std::unordered_set<std::string>& tags = {});
    static void debug(const std::string& message, const std::unordered_set<std::string>& tags = {});
    static void info(const std::string& message, const std::unordered_set<std::string>& tags = {});
    static void warning(const std::string& message, const std::unordered_set<std::string>& tags = {});
    static void error(const std::string& message, const std::unordered_set<std::string>& tags = {});
    static void critical(const std::string& message, const std::unordered_set<std::string>& tags = {});

    static std::string viewLogs(size_t numLines);
    static std::string filterLogs(LogLevel level, const std::string& tag = "", const std::string& regexPattern = "",
                                  const std::chrono::system_clock::time_point& startTime = {}, 
                                  const std::chrono::system_clock::time_point& endTime = {}, size_t numLines = 100);
    static std::string exportLogs(const std::string& format);
    static void clearLogs();
    static void setLogLevel(LogLevel level);
    static std::map<LogLevel, size_t> getLogStatistics();

    static void startRESTServer();
    static void stopRESTServer();

private:
    static void processEntries();
    static std::string formatMessage(const std::string& message, LogLevel level, const std::unordered_set<std::string>& tags = {});
    static void rotateLogs();
    static void logToDatabase(const std::string& message, LogLevel level, const std::unordered_set<std::string>& tags);
    static void loadConfig(const std::string& configFilePath);
    static void validateConfig();

    static std::queue<std::tuple<std::string, LogLevel, std::unordered_set<std::string>>> logQueue;
    static std::mutex queueMutex;
    static std::condition_variable condition;
    static bool running;
    static std::thread workerThread;
    static std::ofstream logFileStream;
    static LogLevel currentLogLevel;
    static std::map<LogLevel, std::string> logLevelMap;
    static std::string logFilePath;
    static size_t maxFileSize;
    static size_t currentFileSize;
    static int fileIndex;
    static std::chrono::system_clock::time_point lastRotationTime;
    static std::chrono::hours rotationInterval;
    static std::map<LogLevel, size_t> logStatistics;

    static bool enableRESTServer;
    static httplib::Server restServer;
};

#endif // LOGGER_H







