#ifndef THREADING_UTILS_H
#define THREADING_UTILS_H

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <map>
#include <memory>
#include <chrono>
#include <nvml.h>
#include "../logging/logger.h"
#include "../config/config.h"

class ThreadingUtils {
public:
    ThreadingUtils(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config);
    ~ThreadingUtils();

    void runInParallel(const std::vector<std::function<void()>>& tasks, const std::string& strategy = "default");
    unsigned int getAvailableThreads() const;
    void stopThreads();
    void addTask(const std::function<void()>& task, int priority = 0);
    void setThreadCount(unsigned int count);
    void pauseTasks();
    void resumeTasks();

    void enableMonitoring();
    void disableMonitoring();

    std::vector<std::string> getMetrics();

private:
    struct Task {
        std::function<void()> func;
        int priority;

        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    void worker(size_t index);
    void logEvent(const std::string& event, LogLevel level = LogLevel::INFO);
    void monitorThreads();
    void restartThread(size_t index);
    void handleException(const std::exception& e, const std::string& context);
    void handleUnknownException(const std::string& context);

    void executeTaskWithStrategy(const Task& task, const std::string& strategy);

    std::vector<std::thread> threads;
    std::priority_queue<Task> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    std::atomic<bool> pauseFlag;
    std::atomic<unsigned int> activeThreads;
    std::atomic<bool> monitoringEnabled;
    std::atomic<unsigned int> maxThreads;
    std::atomic<bool> restartThreads;
    std::map<std::thread::id, size_t> threadIndexMap;

    std::shared_ptr<Logger> logger;
    std::shared_ptr<Config> config;

    unsigned int getAvailableGPUs() const;
    std::vector<std::string> getGPUMetrics();
};

#endif // THREADING_UTILS_H



