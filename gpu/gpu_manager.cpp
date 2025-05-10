#include "gpu_manager.h"
#include <iostream>
#include <exception>
#include <sstream>
#include <fstream>
#include <cuda_runtime.h>
#include <nvml.h>
#include <rocm_smi/rocm_smi.h>
#include <execinfo.h>
#include <vector>

GPUManager::GPUManager(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config, std::shared_ptr<NotificationManager> notificationManager, std::shared_ptr<Monitor> monitor)
    : logger(logger), config(config), notificationManager(notificationManager), monitor(monitor) {
    gpuUtils = std::make_unique<GPUUtils>(logger, config);
    threadingUtils = std::make_unique<ThreadingUtils>(logger, config);
    notificationManager->init("notification_config.json");
}

void GPUManager::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logsMutex);
    logs.push_back(message);
    logger->info(message);
    notificationManager->sendNotification("GPU Manager", message);
    monitor->logEvent(message);
}

void GPUManager::handleException(const std::exception& e) {
    std::string errorMessage = "Ошибка: " + std::string(e.what());
    log(errorMessage);
    logger->error(errorMessage);
    notificationManager->sendEmail("admin@example.com", "Critical GPU Error", errorMessage);
    notificationManager->processTriggers("system_error");
    monitor->logCriticalEvent(errorMessage);
    std::string stackTrace = getStackTrace();
    log("Трассировка стека: " + stackTrace);
    logger->error("Трассировка стека: " + stackTrace);
    monitor->logEvent("Трассировка стека: " + stackTrace);
}

std::string GPUManager::getStackTrace() const {
    const int maxFrames = 100;
    void *frames[maxFrames];
    int numFrames = backtrace(frames, maxFrames);
    char **symbols = backtrace_symbols(frames, numFrames);

    std::ostringstream oss;
    for (int i = 0; i < numFrames; ++i) {
        oss << symbols[i] << "\n";
    }
    free(symbols);
    return oss.str();
}

bool GPUManager::initialize() {
    bool success = true;
    log("Инициализация видеокарт");
    auto nvidiaGPU = std::make_unique<NvidiaGPU>();
    if (nvidiaGPU->initialize()) {
        gpus.push_back(std::move(nvidiaGPU));
        log("Nvidia GPU инициализирован");
    } else {
        log("Ошибка инициализации Nvidia GPU");
        success = false;
    }
    auto amdGPU = std::make_unique<AMDGPU>();
    if (amdGPU->initialize()) {
        gpus.push_back(std::move(amdGPU));
        log("AMD GPU инициализирован");
    } else {
        log("Ошибка инициализации AMD GPU");
        success = false;
    }
    if (gpus.empty()) {
        std::string errorMessage = "Не удалось инициализировать ни одну GPU";
        log(errorMessage);
        notificationManager->sendEmail("admin@example.com", "Critical GPU Error", errorMessage);
        return false;
    }
    return success;
}

bool GPUManager::configure(const std::map<std::string, std::string>& config) {
    if (!validateConfig(config)) {
        return false;
    }
    bool success = true;
    log("Настройка параметров GPU");
    for (const auto& gpu : gpus) {
        if (!gpu->configure(config)) {
            std::string errorMessage = "Ошибка настройки GPU";
            log(errorMessage);
            notificationManager->sendEmail("admin@example.com", "GPU Configuration Error", errorMessage);
            success = false;
        }
    }
    return success;
}

bool GPUManager::validateConfig(const std::map<std::string, std::string>& config) const {
    if (config.empty()) {
        log("Конфигурация пуста, настройка GPU невозможна.");
        return false;
    }
    auto powerLimitIt = config.find("power_limit");
    if (powerLimitIt == config.end()) {
        log("Отсутствует параметр 'power_limit' в конфигурации.");
        return false;
    }
    try {
        int powerLimit = std::stoi(powerLimitIt->second); 
        if (powerLimit <= 0) {
            log("Недопустимое значение 'power_limit': " + powerLimitIt->second);
            return false;
        }
    } catch (const std::exception& e) {
        log("Ошибка в преобразовании параметра 'power_limit': " + std::string(e.what()));
        return false;
    }
    return true;
}

bool GPUManager::validateTask(const std::function<void()>& task) const {
    if (!task) {
        log("Переданная задача пуста, выполнение невозможно.");
        return false;
    }
    return true;
}
bool GPUManager::executeAttack(const std::function<void()>& attackTask, const std::string& strategy) {
    if (!validateTask(attackTask)) {
        return false;
    }

    try {
        log("Выполнение атаки с использованием стратегии распределения: " + strategy);

        std::vector<std::future<void>> futures;

        for (size_t i = 0; i < gpus.size(); ++i) {
            futures.push_back(std::async(std::launch::async, [this, attackTask, strategy, gpuId = i]() {
                gpuUtils->addTask(attackTask);
                gpuUtils->runTasks(strategy);
            }));
        }
        for (auto& future : futures) {
            future.get();
        }

        notificationManager->processTriggers("task_completed");
        return true;
    } catch (const std::exception& e) {
        handleException(e);
        return false;
    }
}

std::string GPUManager::getStatus() const {
    std::lock_guard<std::mutex> lock(statusMutex);
    std::stringstream status;
    for (const auto& gpu : gpus) {
        status << gpu->getStatus() << "\n";
    }
    return status.str();
}
std::vector<std::string> GPUManager::getLogs() const {
    std::lock_guard<std::mutex> lock(logsMutex);
    return logs;
}
std::map<std::string, std::string> GPUManager::monitorGPU() const {
    return gpuUtils->getMetrics();
}
void GPUManager::stopAllOperations() {
    threadingUtils->stopThreads();
    gpuUtils->stopTasks();
}
bool GPUManager::managePowerConsumption() {
    bool success = true;
    log("Управление энергопотреблением GPU");
    for (const auto& gpu : gpus) {
        if (!gpu->managePower()) {
            std::string errorMessage = "Ошибка управления энергопотреблением GPU";
            log(errorMessage);
            notificationManager->sendEmail("admin@example.com", "Power Management Error", errorMessage);
            success = false;
        }
    }
    return success;
}
bool GPUManager::enableAdvancedLogging() {
    bool success = true;
    log("Включение расширенного логирования");
    for (const auto& gpu : gpus) {
        if (!gpu->enableLogging()) {
            std::string errorMessage = "Ошибка включения расширенного логирования GPU";
            log(errorMessage);
            notificationManager->sendEmail("admin@example.com", "Logging Error", errorMessage);
            success = false;
        }
    }
    return success;
}
bool GPUManager::optimizeMemoryUsage() {
    bool success = true;
    log("Оптимизация использования памяти GPU");
    for (const auto& gpu : gpus) {
        if (!gpu->optimizeMemory()) {
            std::string errorMessage = "Ошибка оптимизации памяти GPU";
            log(errorMessage);
            notificationManager->sendEmail("admin@example.com", "Memory Optimization Error", errorMessage);
            success = false;
        }
    }
    return success;
}
bool GPUManager::scheduleGPUJob(const std::function<void()>& job) {
    if (!validateTask(job)) {
        return false;
    }

    try {
        log("Планирование задачи для выполнения на GPU");
        gpuUtils->addTask(job);
        return true;
    } catch (const std::exception& e) {
        handleException(e);
        return false;
    }
}
bool GPUManager::monitorGPUJob(const std::string& jobId) {
    try {
        log("Мониторинг задачи на GPU: " + jobId);
        log("Статус задачи: В процессе");
        return true;
    } catch (const std::exception& e) {
        handleException(e);
        return false;
    }
}
void GPUManager::logGPUState() {
    try {
        log("Логирование состояния GPU");
        for (const auto& gpu : gpus) {
            log(gpu->getStatus());
        }
    } catch (const std::exception& e) {
        handleException(e);
    }
}
std::map<std::string, std::string> GPUManager::getGPUMetrics() const {
    return gpuUtils->getMetrics();
}
std::map<std::string, std::string> GPUManager::getGPUConfig() const {
    return {{"Parameter1", "Value1"}, {"Parameter2", "Value2"}};
}
void GPUManager::exportLogs(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(logsMutex);
    std::ofstream file(filename);
    for (const auto& log : logs) {
        file << log << std::endl;
    }
}














