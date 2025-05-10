#ifndef GPU_UTILS_H
#define GPU_UTILS_H

#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <nvml.h>
#include "../logging/logger.h"
#include "../config/config.h"

enum class LogLevel { INFO, WARNING, ERROR };

class GPUUtils {
public:
    GPUUtils(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config);
    ~GPUUtils();

    void initialize();
    void releaseResources();
    void addTask(const std::function<void()>& task, int priority = 0);
    void runTasks(const std::string& strategy = "default");
    void stopTasks();
    void monitorGPU();
    std::vector<std::string> getMetrics();

private:
    struct Task {
        std::function<void()> func;
        int priority;

        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    void worker();
    void logEvent(const std::string& event, LogLevel level = LogLevel::INFO);
    void handleException(const std::exception& e, const std::string& context);
    void handleUnknownException(const std::string& context);
    void restartTask(const std::function<void()>& task);
    void runWithStrategy(const std::string& strategy);
    void roundRobin();
    void fairScheduling();
    void leastLoaded();
    void temperatureBased();

    std::vector<std::thread> threads;
    std::priority_queue<Task> taskQueue;
    std::priority_queue<Task> restartQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    std::atomic<unsigned int> activeTasks;
    std::atomic<bool> monitoringEnabled;
    std::atomic<unsigned int> maxGPU;
    std::atomic<bool> restartTasks;

    std::shared_ptr<Logger> logger;
    std::shared_ptr<Config> config;
};

#endif // GPU_UTILS_H

