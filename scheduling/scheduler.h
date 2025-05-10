#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../logging/logger.h"
#include "../config/config.h"
#include "../notifications/notification_manager.h"
#include "../database/db_manager.h"
#include "../attack/attack_engine.h"
#include "../rules/rule_engine.h"
#include "../gpu/gpu_manager.h"
#include "../recovery/auto_recovery.h"
#include "../users/user_management.h"
#include "../utils/threading_utils.h"
#include "../utils/data_utils.h"
#include "../utils/cloud_utils.h"
#include "../api/api.h"
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include <set>
#include <future>

enum class TaskPriority {
    LOW,
    MEDIUM,
    HIGH
};

class Scheduler {
public:
    using TaskFunc = std::function<void()>;

    Scheduler(std::shared_ptr<NotificationManager> notificationManager);
    ~Scheduler();

    void scheduleTask(const std::string& name, const TaskFunc& func, const std::chrono::system_clock::time_point& time, TaskPriority priority, bool isPeriodic = false, std::chrono::seconds interval = std::chrono::seconds(0));
    void scheduleMonthlyTask(const std::string& name, const TaskFunc& func, int dayOfMonth, int hour, int minute);
    void scheduleYearlyTask(const std::string& name, const TaskFunc& func, int month, int day, int hour, int minute);
    void addDependency(const std::string& taskName, const std::string& dependency);
    void removeDependency(const std::string& taskName, const std::string& dependency);
    void clearDependencies(const std::string& taskName);
    void applyConfig();
    void startMonitoring();
    void sendAlert(const std::string& message);
    void scheduleAPICall(const std::string& name, const std::string& apiEndpoint, const std::chrono::system_clock::time_point& time);
    void scheduleEventTask(const std::string& name, const std::string& eventName, const std::chrono::system_clock::time_point& time);
    void loadUserScript(const std::string& scriptPath);
    void scheduleBackupTask(const std::string& name, const std::string& dataId, const std::chrono::system_clock::time_point& time);
    void scheduleRecoveryTask(const std::string& name, const std::string& dataId, const std::chrono::system_clock::time_point& time);
    void scheduleUserExport(const std::string& name, const std::chrono::system_clock::time_point& time);
    void scheduleUserImport(const std::string& name, const std::chrono::system_clock::time_point& time);
    void applyDatabaseSession();
    void monitorDatabaseSession();
    void logTaskState(const std::string& taskName, const std::string& state);
    void sendTaskNotification(const std::string& taskName, const std::string& message);
    void addTaskDependency(const std::string& taskName, const std::string& dependency);
    bool checkTaskDependencies(const std::string& taskName);
    void reloadConfig();
    void callExternalAPI(const std::string& apiEndpoint, const std::string& payload);
    void addTask(const std::string& name, const TaskFunc& func);
    void removeTask(const std::string& name);
    void updateTask(const std::string& name, const TaskFunc& func);
    void integrateWithCalendar();

private:
    struct Task {
        std::string name;
        TaskFunc func;
        std::chrono::system_clock::time_point time;
        TaskPriority priority;
        bool isPeriodic;
        std::chrono::seconds interval;
        int retryCount = 0;
    };

    struct TaskComparator {
        bool operator()(const Task& lhs, const Task& rhs) const {
            return lhs.time > rhs.time;
        }
    };

    void taskRunner();
    void executeTask(const Task& task);
    void executeTaskInternal(const Task& task);
    void retryTask(const Task& task, const std::string& errorMsg);
    void logTask(const std::string& taskName, const std::string& status);
    void notifyTaskStatus(const std::string& taskName, const std::string& status);
    void createFailureReport(const Task& task, const std::string& errorMsg);
    bool canExecuteTask(const std::string& taskName) const;

    Logger logger;
    Config config;
    std::shared_ptr<NotificationManager> notificationManager;
    DBManager dbManager;
    AttackEngine attackEngine;
    RuleEngine ruleEngine;
    GPUManager gpuManager;
    AutoRecovery autoRecovery;
    UserManagement userManagement;
    ThreadingUtils threadingUtils;
    DataUtils dataUtils;
    CloudUtils cloudUtils;
    API api;

    std::priority_queue<Task, std::vector<Task>, TaskComparator> taskQueue;
    std::unordered_map<std::string, std::set<std::string>> taskDependencies;
    std::unordered_map<std::string, std::set<std::string>> dependentTasks;
    std::unordered_map<std::string, Config::ConfigValue> configCache;
    std::mutex taskMutex;
    std::condition_variable taskCondition;
    std::thread workerThread;
    bool stopFlag = false;

    int maxRetries = 3;
};

#endif // SCHEDULER_H



