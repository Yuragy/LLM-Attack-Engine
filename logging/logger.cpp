#include "logger.h"
#include "db_manager.h"
#include "config.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <regex>
#include <iomanip>

using json = nlohmann::json;

std::queue<std::tuple<std::string, LogLevel, std::unordered_set<std::string>>> Logger::logQueue;
std::mutex Logger::queueMutex;
std::condition_variable Logger::condition;
bool Logger::running = false;
std::thread Logger::workerThread;
std::ofstream Logger::logFileStream;
LogLevel Logger::currentLogLevel;
std::map<LogLevel, std::string> Logger::logLevelMap = {
    {LogLevel::TRACE, "TRACE"},
    {LogLevel::DEBUG, "DEBUG"},
    {LogLevel::INFO, "INFO"},
    {LogLevel::WARNING, "WARNING"},
    {LogLevel::ERROR, "ERROR"},
    {LogLevel::CRITICAL, "CRITICAL"}
};
std::string Logger::logFilePath;
size_t Logger::maxFileSize = 10 * 1024 * 1024; // 10 MB
size_t Logger::currentFileSize = 0;
int Logger::fileIndex = 0;
std::chrono::system_clock::time_point Logger::lastRotationTime;
std::chrono::hours Logger::rotationInterval = std::chrono::hours(24);
std::map<LogLevel, size_t> Logger::logStatistics;
bool Logger::enableRESTServer = false;
httplib::Server Logger::restServer;

void Logger::initialize(const std::string& configFilePath) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (running) {
        return;
    }

    loadConfig(configFilePath);
    validateConfig();

    logFileStream.open(logFilePath, std::ios::out | std::ios::app);
    logFileStream.seekp(0, std::ios::end);
    currentFileSize = logFileStream.tellp();
    lastRotationTime = std::chrono::system_clock::now();
    running = true;
    workerThread = std::thread(&Logger::processEntries);

    if (enableRESTServer) {
        startRESTServer();
    }
}

void Logger::shutdown() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        running = false;
    }
    condition.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }
    logFileStream.close();

    if (enableRESTServer) {
        stopRESTServer();
    }
}

void Logger::log(const std::string& message, LogLevel level, const std::unordered_set<std::string>& tags) {
    if (level < currentLogLevel) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.emplace(message, level, tags);
        logStatistics[level]++;
    }
    condition.notify_one();
}

void Logger::trace(const std::string& message, const std::unordered_set<std::string>& tags) {
    log(message, LogLevel::TRACE, tags);
}

void Logger::debug(const std::string& message, const std::unordered_set<std::string>& tags) {
    log(message, LogLevel::DEBUG, tags);
}

void Logger::info(const std::string& message, const std::unordered_set<std::string>& tags) {
    log(message, LogLevel::INFO, tags);
}

void Logger::warning(const std::string& message, const std::unordered_set<std::string>& tags) {
    log(message, LogLevel::WARNING, tags);
}

void Logger::error(const std::string& message, const std::unordered_set<std::string>& tags) {
    log(message, LogLevel::ERROR, tags);
}

void Logger::critical(const std::string& message, const std::unordered_set<std::string>& tags) {
    log(message, LogLevel::CRITICAL, tags);
}

std::string Logger::viewLogs(size_t numLines) {
    std::ifstream logFile(logFilePath);
    if (!logFile.is_open()) {
        return "Не удалось открыть файл логов.";
    }

    std::deque<std::string> lines;
    std::string line;
    while (std::getline(logFile, line)) {
        lines.push_back(line);
        if (lines.size() > numLines) {
            lines.pop_front();
        }
    }

    std::ostringstream oss;
    for (const auto& l : lines) {
        oss << l << "\n";
    }

    return oss.str();
}

std::string Logger::filterLogs(LogLevel level, const std::string& tag, const std::string& regexPattern,
                               const std::chrono::system_clock::time_point& startTime,
                               const std::chrono::system_clock::time_point& endTime, size_t numLines) {
    std::ifstream logFile(logFilePath);
    if (!logFile.is_open()) {
        return "Не удалось открыть файл логов.";
    }

    std::deque<std::string> lines;
    std::string line;
    std::regex pattern(regexPattern);
    while (std::getline(logFile, line)) {
        if ((line.find(logLevelMap[level]) != std::string::npos) &&
            (tag.empty() || line.find(tag) != std::string::npos) &&
            (regexPattern.empty() || std::regex_search(line, pattern))) {
            if (startTime != std::chrono::system_clock::time_point{} &&
                endTime != std::chrono::system_clock::time_point{}) {
                std::istringstream iss(line);
                std::tm logTimeTm = {};
                iss >> std::get_time(&logTimeTm, "[%Y-%m-%d %H:%M:%S]");
                auto logTime = std::chrono::system_clock::from_time_t(std::mktime(&logTimeTm));
                if (logTime < startTime || logTime > endTime) {
                    continue;
                }
            }

            lines.push_back(line);
            if (lines.size() > numLines) {
                lines.pop_front();
            }
        }
    }

    std::ostringstream oss;
    for (const auto& l : lines) {
        oss << l << "\n";
    }

    return oss.str();
}

std::string Logger::exportLogs(const std::string& format) {
    std::ifstream logFile(logFilePath);
    if (!logFile.is_open()) {
        return "Не удалось открыть файл логов.";
    }

    std::ostringstream oss;
    std::string line;
    if (format == "json") {
        json root;
        while (std::getline(logFile, line)) {
            root.push_back(line);
        }
        return root.dump();
    } else if (format == "csv") {
        oss << "log\n";
        while (std::getline(logFile, line)) {
            oss << "\"" << line << "\"\n";
        }
        return oss.str();
    } else {
        return "Неизвестный формат экспорта.";
    }
}

void Logger::clearLogs() {
    std::lock_guard<std::mutex> lock(queueMutex);
    logFileStream.close();
    logFileStream.open(logFilePath, std::ios::out | std::ios::trunc);
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(queueMutex);
    currentLogLevel = level;
}

std::map<LogLevel, size_t> Logger::getLogStatistics() {
    std::lock_guard<std::mutex> lock(queueMutex);
    return logStatistics;
}

void Logger::processEntries() {
    while (running || !logQueue.empty()) {
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock, [] { return !logQueue.empty() || !running; });

        while (!logQueue.empty()) {
            auto entry = logQueue.front();
            logQueue.pop();
            lock.unlock();

            std::string formattedMessage = formatMessage(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry));
            if (logFileStream.is_open()) {
                logFileStream << formattedMessage << std::endl;
                currentFileSize += formattedMessage.size() + 1;
                if (currentFileSize >= maxFileSize || (std::chrono::system_clock::now() - lastRotationTime) >= rotationInterval) {
                    rotateLogs();
                }
            }
            std::cout << formattedMessage << std::endl;

            logToDatabase(formattedMessage, std::get<1>(entry), std::get<2>(entry));

            lock.lock();
        }
    }
}

std::string Logger::formatMessage(const std::string& message, LogLevel level, const std::unordered_set<std::string>& tags) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
#ifdef _WIN32
    localtime_s(&now_tm, &now_time_t);
#else
    localtime_r(&now_tm, &now_time_t);
#endif

    std::ostringstream oss;
    oss << "[" << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "] "
        << logLevelMap[level] << ": " << message;
    if (!tags.empty()) {
        oss << " [Tags: ";
        for (const auto& tag : tags) {
            oss << tag << " ";
        }
        oss << "]";
    }
    return oss.str();
}

void Logger::rotateLogs() {
    logFileStream.close();
    fileIndex++;
    std::string newLogFilePath = logFilePath + "." + std::to_string(fileIndex);
    logFileStream.open(newLogFilePath, std::ios::out | std::ios::app);
    currentFileSize = 0;
    lastRotationTime = std::chrono::system_clock::now();
}

void Logger::logToDatabase(const std::string& message, LogLevel level, const std::unordered_set<std::string>& tags) {
    DBManager::logMessageToDB(message, logLevelMap[level], tags);
}

void Logger::loadConfig(const std::string& configFilePath) {
    std::ifstream configFile(configFilePath);
    if (!configFile.is_open()) {
        throw std::runtime_error("Не удалось открыть файл конфигурации.");
    }

    json config;
    configFile >> config;

    logFilePath = config["logFilePath"].get<std::string>();
    maxFileSize = config["maxFileSize"].get<size_t>();
    rotationInterval = std::chrono::hours(config["rotationIntervalHours"].get<int>());
    enableRESTServer = config["enableRESTServer"].get<bool>();

    std::string logLevelStr = config["logLevel"].get<std::string>();
    if (logLevelStr == "TRACE") {
        currentLogLevel = LogLevel::TRACE;
    } else if (logLevelStr == "DEBUG") {
        currentLogLevel = LogLevel::DEBUG;
    } else if (logLevelStr == "INFO") {
        currentLogLevel = LogLevel::INFO;
    } else if (logLevelStr == "WARNING") {
        currentLogLevel = LogLevel::WARNING;
    } else if (logLevelStr == "ERROR") {
        currentLogLevel = LogLevel::ERROR;
    } else if (logLevelStr == "CRITICAL") {
        currentLogLevel = LogLevel::CRITICAL;
    } else {
        throw std::runtime_error("Неизвестный уровень логирования в конфигурации.");
    }
}

void Logger::validateConfig() {
    if (logFilePath.empty() || maxFileSize == 0 || rotationInterval.count() == 0) {
        throw std::runtime_error("Invalid configuration.");
    }
}

void Logger::startRESTServer() {
    try {
        std::thread serverThread([&]() {
            restServer.Get("/logs", [](const httplib::Request& req, httplib::Response& res) {
                res.set_content(Logger::viewLogs(100), "text/plain");
            });
            restServer.Get("/logs/filter", [](const httplib::Request& req, httplib::Response& res) {
                LogLevel level = LogLevel::INFO;
                std::string tag;
                std::string regex;
                std::chrono::system_clock::time_point startTime = {};
                std::chrono::system_clock::time_point endTime = {};
                
                if (req.has_param("level")) {
                    std::string levelStr = req.get_param_value("level");
                    if (levelStr == "TRACE") level = LogLevel::TRACE;
                    else if (levelStr == "DEBUG") level = LogLevel::DEBUG;
                    else if (levelStr == "WARNING") level = LogLevel::WARNING;
                    else if (levelStr == "ERROR") level = LogLevel::ERROR;
                    else if (levelStr == "CRITICAL") level = LogLevel::CRITICAL;
                }
                if (req.has_param("tag")) {
                    tag = req.get_param_value("tag");
                }
                if (req.has_param("regex")) {
                    regex = req.get_param_value("regex");
                }
                res.set_content(Logger::filterLogs(level, tag, regex, startTime, endTime, 100), "text/plain");
            });
            restServer.Get("/logs/export", [](const httplib::Request& req, httplib::Response& res) {
                std::string format = "json";
                if (req.has_param("format")) {
                    format = req.get_param_value("format");
                }
                res.set_content(Logger::exportLogs(format), "application/json");
            });

            if (!restServer.listen("localhost", 8080)) {
                std::cerr << "Failed to start REST server on port 8080" << std::endl;
            }
        });
        serverThread.detach();
    } catch (const std::exception& e) {
        std::cerr << "Exception in startRESTServer: " << e.what() << std::endl;
    }
}

void Logger::stopRESTServer() {
    try {
        restServer.stop();
    } catch (const std::exception& e) {
        std::cerr << "Exception in stopRESTServer: " << e.what() << std::endl;
    }
}

















