#include "threading_utils.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <algorithm>

ThreadingUtils::ThreadingUtils(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config)
    : logger(logger), config(config), stopFlag(false), pauseFlag(false), activeThreads(0), monitoringEnabled(false), maxThreads(getAvailableThreads()), restartThreads(true) {
    nvmlInit();
    setThreadCount(maxThreads);
}

ThreadingUtils::~ThreadingUtils() {
    stopThreads();
    nvmlShutdown(); 
}

void ThreadingUtils::runInParallel(const std::vector<std::function<void()>>& tasks, const std::string& strategy) {
    if (strategy == "round-robin") {
        size_t i = 0;
        for (const auto& task : tasks) {
            addTask([task, i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(10 * (i % 10)));
                task();
            });
            i++;
        }
    } else if (strategy == "fair-scheduling") {
        for (const auto& task : tasks) {
            addTask(task, rand() % 10);
        }
    } else if (strategy == "gpu") {
        for (const auto& task : tasks) {
            addTask([task]() {
                cudaSetDevice(rand() % getAvailableGPUs()); 
                task();
            });
        }
    } else {
        for (const auto& task : tasks) {
            addTask(task);
        }
    }
}

unsigned int ThreadingUtils::getAvailableThreads() const {
    return std::thread::hardware_concurrency();
}

unsigned int ThreadingUtils::getAvailableGPUs() const {
    unsigned int deviceCount;
    nvmlDeviceGetCount(&deviceCount);
    return deviceCount;
}

void ThreadingUtils::stopThreads() {
    stopFlag = true;
    condition.notify_all();

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads.clear();
}

void ThreadingUtils::addTask(const std::function<void()>& task, int priority) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        taskQueue.push({task, priority});
    }
    condition.notify_one();
}

void ThreadingUtils::setThreadCount(unsigned int count) {
    stopThreads();
    stopFlag = false;

    maxThreads = count;
    threads.resize(maxThreads);
    for (size_t i = 0; i < maxThreads; ++i) {
        threads[i] = std::thread(&ThreadingUtils::worker, this, i);
        threadIndexMap[threads[i].get_id()] = i;
    }
}

void ThreadingUtils::pauseTasks() {
    pauseFlag = true;
}

void ThreadingUtils::resumeTasks() {
    pauseFlag = false;
    condition.notify_all();
}

void ThreadingUtils::worker(size_t index) {
    while (!stopFlag) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stopFlag || (!taskQueue.empty() && !pauseFlag); });

            if (stopFlag && taskQueue.empty()) {
                return;
            }

            if (pauseFlag) {
                continue;
            }

            task = std::move(taskQueue.top());
            taskQueue.pop();
        }

        activeThreads++;
        try {
            executeTaskWithStrategy(task, "default");
        } catch (const std::exception& e) {
            handleException(e, "Thread task execution");
        } catch (...) {
            handleUnknownException("Thread task execution");
        }
        activeThreads--;

        if (restartThreads) {
            restartThread(index);
        }
    }
}

void ThreadingUtils::logEvent(const std::string& event, LogLevel level) {
    if (logger) {
        logger->log(event, level);
    } else {
        std::cerr << event << std::endl;
    }
}

void ThreadingUtils::enableMonitoring() {
    monitoringEnabled = true;
    std::thread(&ThreadingUtils::monitorThreads, this).detach();
}

void ThreadingUtils::disableMonitoring() {
    monitoringEnabled = false;
}

void ThreadingUtils::monitorThreads() {
    while (monitoringEnabled) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        logEvent("Monitoring threads: active threads = " + std::to_string(activeThreads) + ", task queue size = " + std::to_string(taskQueue.size()), LogLevel::INFO);
        std::vector<std::string> gpuMetrics = getGPUMetrics();
        for (const auto& metric : gpuMetrics) {
            logEvent(metric, LogLevel::INFO);
        }
    }
}

void ThreadingUtils::restartThread(size_t index) {
    if (index < threads.size()) {
        if (threads[index].joinable()) {
            threads[index].join();
        }
        threads[index] = std::thread(&ThreadingUtils::worker, this, index);
        threadIndexMap[threads[index].get_id()] = index;
    }
}

void ThreadingUtils::handleException(const std::exception& e, const std::string& context) {
    logEvent("Exception caught in context: " + context + " - " + e.what(), LogLevel::ERROR);
    auto it = threadIndexMap.find(std::this_thread::get_id());
    if (it != threadIndexMap.end()) {
        size_t index = it->second;
        restartThread(index);
    }
}

void ThreadingUtils::handleUnknownException(const std::string& context) {
    logEvent("Unknown exception caught in context: " + context, LogLevel::ERROR);
    auto it = threadIndexMap.find(std::this_thread::get_id());
    if (it != threadIndexMap.end()) {
        size_t index = it->second;
        restartThread(index);
    }
}

std::vector<std::string> ThreadingUtils::getMetrics() {
    std::vector<std::string> metrics;
    metrics.push_back("Active threads: " + std::to_string(activeThreads));
    metrics.push_back("Task queue size: " + std::to_string(taskQueue.size()));
    return metrics;
}

std::vector<std::string> ThreadingUtils::getGPUMetrics() {
    std::vector<std::string> metrics;
    for (unsigned int i = 0; i < getAvailableGPUs(); ++i) {
        nvmlDevice_t device;
        nvmlDeviceGetHandleByIndex(i, &device);

        nvmlMemory_t memory;
        nvmlDeviceGetMemoryInfo(device, &memory);

        nvmlUtilization_t utilization;
        nvmlDeviceGetUtilizationRates(device, &utilization);

        unsigned int temperature;
        nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);

        metrics.push_back("GPU " + std::to_string(i) + " - Memory Usage: " + std::to_string(memory.used / 1024 / 1024) + "MB/" + std::to_string(memory.total / 1024 / 1024) + "MB, Utilization: " + std::to_string(utilization.gpu) + "%, Temperature: " + std::to_string(temperature) + "C");
    }
    return metrics;
}

void ThreadingUtils::executeTaskWithStrategy(const Task& task, const std::string& strategy) {
    if (strategy == "default") {
        task.func();
    } else if (strategy == "round-robin") {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        task.func();
    } else if (strategy == "fair-scheduling") {
        std::this_thread::sleep_for(std::chrono::milliseconds(5 * task.priority));
        task.func();
    } else if (strategy == "gpu") {
        cudaSetDevice(rand() % getAvailableGPUs());
        task.func();
    }
}








