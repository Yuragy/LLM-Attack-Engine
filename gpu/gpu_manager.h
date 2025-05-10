#ifndef GPU_MANAGER_H
#define GPU_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>
#include <future>
#include "gpu_utils.h"
#include "logging/logger.h"
#include "config/config.h"
#include "threading_utils.h"
#include "notifications/notification_manager.h"
#include "monitoring/monitor.h"

class GPU {
public:
    virtual bool initialize() = 0;
    virtual bool executeTask(const std::function<void()>& task) = 0;
    virtual std::string getStatus() const = 0;
    virtual std::map<std::string, std::string> getMetrics() const = 0;
    virtual bool configure(const std::map<std::string, std::string>& config) = 0;
    virtual bool managePower() = 0;
    virtual bool enableLogging() = 0;
    virtual bool optimizeMemory() = 0;
    virtual ~GPU() = default;
};
class NvidiaGPU : public GPU {
public:
    bool initialize() override;
    bool executeTask(const std::function<void()>& task) override;
    std::string getStatus() const override;
    std::map<std::string, std::string> getMetrics() const override;
    bool configure(const std::map<std::string, std::string>& config) override;
    bool managePower() override;
    bool enableLogging() override;
    bool optimizeMemory() override;
private:
    std::vector<nvmlDevice_t> devices;
};

class AMDGPU : public GPU {
public:
    bool initialize() override;
    bool executeTask(const std::function<void()>& task) override;
    std::string getStatus() const override;
    std::map<std::string, std::string> getMetrics() const override;
    bool configure(const std::map<std::string, std::string>& config) override;
    bool managePower() override;
    bool enableLogging() override;
    bool optimizeMemory() override;
private:
    std::vector<uint32_t> devices;
};

class GPUManager {
public:
    GPUManager(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config, std::shared_ptr<NotificationManager> notificationManager, std::shared_ptr<Monitor> monitor);
    bool initialize();
    bool configure(const std::map<std::string, std::string>& config);
    bool executeAttack(const std::function<void()>& attackTask, const std::string& strategy = "default");
    std::string getStatus() const;
    std::vector<std::string> getLogs() const;
    std::map<std::string, std::string> monitorGPU() const;
    void stopAllOperations();
    bool managePowerConsumption();
    bool enableAdvancedLogging();
    bool optimizeMemoryUsage();
    bool scheduleGPUJob(const std::function<void()>& job);
    bool monitorGPUJob(const std::string& jobId);
    void logGPUState();
    std::map<std::string, std::string> getGPUMetrics() const;
    std::map<std::string, std::string> getGPUConfig() const;
    void exportLogs(const std::string& filename) const;
    size_t getGPUCount() const { return gpus.size(); }

private:
    bool validateConfig(const std::map<std::string, std::string>& config) const;
    bool validateTask(const std::function<void()>& task) const;
    void log(const std::string& message);
    void handleException(const std::exception& e);
    std::string getStackTrace() const;
    std::vector<std::string> logs;
    mutable std::mutex logsMutex;
    std::map<std::string, std::string> gpuStatus;
    mutable std::mutex statusMutex;
    std::vector<std::unique_ptr<GPU>> gpus;
    std::unique_ptr<GPUUtils> gpuUtils;
    std::unique_ptr<ThreadingUtils> threadingUtils;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<Config> config;
    std::shared_ptr<NotificationManager> notificationManager;
    std::shared_ptr<Monitor> monitor;
};

#endif // GPU_MANAGER_H








