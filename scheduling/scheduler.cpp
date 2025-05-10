#include "scheduler.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>

Scheduler::Scheduler(std::shared_ptr<NotificationManager> notificationManager) 
    : logger(), config(), notificationManager(notificationManager), dbManager(), threadingUtils(), dataUtils(), cloudUtils(), api(), stopFlag(false) {
    workerThread = std::thread(&Scheduler::taskRunner, this);
    applyConfig();
    startMonitoring();
}

Scheduler::~Scheduler() {
    {
        std::lock_guard<std::mutex> lock(taskMutex);
        stopFlag = true;
    }
    taskCondition.notify_all();
    workerThread.join();
}

void Scheduler::scheduleTask(const std::string& name, const TaskFunc& func, const std::chrono::system_clock::time_point& time, TaskPriority priority, bool isPeriodic, std::chrono::seconds interval) {
    std::lock_guard<std::mutex> lock(taskMutex);
    taskQueue.push(Task{name, func, time, priority, isPeriodic, interval});
    taskCondition.notify_all();
    sendTaskNotification(name, "Task scheduled");
}

void Scheduler::scheduleMonthlyTask(const std::string& name, const TaskFunc& func, int dayOfMonth, int hour, int minute) {
    auto now = std::chrono::system_clock::now();
    std::tm* timeinfo = std::localtime(std::chrono::system_clock::to_time_t(now));
    timeinfo->tm_mday = dayOfMonth;
    timeinfo->tm_hour = hour;
    timeinfo->tm_min = minute;
    auto scheduledTime = std::chrono::system_clock::from_time_t(std::mktime(timeinfo));

    if (scheduledTime < now) {
        timeinfo->tm_mon++;
        scheduledTime = std::chrono::system_clock::from_time_t(std::mktime(timeinfo));
    }

    scheduleTask(name, func, scheduledTime, TaskPriority::MEDIUM, true, std::chrono::seconds(30*24*60*60)); // 30 дней
}

void Scheduler::scheduleYearlyTask(const std::string& name, const TaskFunc& func, int month, int day, int hour, int minute) {
    auto now = std::chrono::system_clock::now();
    std::tm* timeinfo = std::localtime(std::chrono::system_clock::to_time_t(now));
    timeinfo->tm_mon = month - 1;
    timeinfo->tm_mday = day;
    timeinfo->tm_hour = hour;
    timeinfo->tm_min = minute;
    auto scheduledTime = std::chrono::system_clock::from_time_t(std::mktime(timeinfo));

    if (scheduledTime < now) {
        timeinfo->tm_year++;
        scheduledTime = std::chrono::system_clock::from_time_t(std::mktime(timeinfo));
    }

    scheduleTask(name, func, scheduledTime, TaskPriority::MEDIUM, true, std::chrono::seconds(365*24*60*60)); // 365 дней
}

void Scheduler::addDependency(const std::string& taskName, const std::string& dependency) {
    std::lock_guard<std::mutex> lock(taskMutex);
    taskDependencies[taskName].insert(dependency);
    dependentTasks[dependency].insert(taskName);
}

void Scheduler::removeDependency(const std::string& taskName, const std::string& dependency) {
    std::lock_guard<std::mutex> lock(taskMutex);
    taskDependencies[taskName].erase(dependency);
    dependentTasks[dependency].erase(taskName);
}

void Scheduler::clearDependencies(const std::string& taskName) {
    std::lock_guard<std::mutex> lock(taskMutex);
    for (const auto& dependency : taskDependencies[taskName]) {
        dependentTasks[dependency].erase(taskName);
    }
    taskDependencies[taskName].clear();
}

void Scheduler::applyConfig() {
    auto configMap = config.getSchedulerConfig();
    maxRetries = std::get<int>(configMap["max_retries"]);
    configCache = configMap;
}

void Scheduler::startMonitoring() {
}

void Scheduler::sendAlert(const std::string& message) {
    notificationManager->sendEmail("admin@example.com", "Scheduler Alert", message);
}

void Scheduler::scheduleAPICall(const std::string& name, const std::string& apiEndpoint, const std::chrono::system_clock::time_point& time) {
    auto taskFunc = [this, apiEndpoint]() {
        api.callAPI(apiEndpoint);
    };
    scheduleTask(name, taskFunc, time, TaskPriority::HIGH, false, std::chrono::seconds(0));
}

void Scheduler::scheduleEventTask(const std::string& name, const std::string& eventName, const std::chrono::system_clock::time_point& time) {
    auto taskFunc = [this, eventName]() {
        api.triggerEvent(eventName);
    };
    scheduleTask(name, taskFunc, time, TaskPriority::HIGH, false, std::chrono::seconds(0));
}

void Scheduler::loadUserScript(const std::string& scriptPath) {
    auto taskFunc = [scriptPath]() {
        std::cout << "Executing script: " << scriptPath << std::endl;
    };
    auto now = std::chrono::system_clock::now();
    scheduleTask("UserScript_" + scriptPath, taskFunc, now, TaskPriority::LOW, false, std::chrono::seconds(0));
}

void Scheduler::scheduleBackupTask(const std::string& name, const std::string& dataId, const std::chrono::system_clock::time_point& time) {
    auto taskFunc = [this, dataId]() {
        if (autoRecovery.backupDataToCloud(dataId, "data")) {
            logger.info("Backup successful for data ID: " + dataId);
            notifyTaskStatus(name, "Backup successful");
        } else {
            logger.error("Backup failed for data ID: " + dataId);
            notifyTaskStatus(name, "Backup failed");
        }
    };
    scheduleTask(name, taskFunc, time, TaskPriority::HIGH, false, std::chrono::seconds(0));
}

void Scheduler::scheduleRecoveryTask(const std::string& name, const std::string& dataId, const std::chrono::system_clock::time_point& time) {
    auto taskFunc = [this, dataId]() {
        if (autoRecovery.restoreDataFromCloud(dataId)) {
            logger.info("Recovery successful for data ID: " + dataId);
            notifyTaskStatus(name, "Recovery successful");
        } else {
            logger.error("Recovery failed for data ID: " + dataId);
            notifyTaskStatus(name, "Recovery failed");
        }
    };
    scheduleTask(name, taskFunc, time, TaskPriority::HIGH, false, std::chrono::seconds(0));
}

void Scheduler::scheduleUserExport(const std::string& name, const std::chrono::system_clock::time_point& time) {
    auto taskFunc = [this]() {
        userManagement.exportUsers("path/to/export/file");
        logger.info("User export completed successfully.");
        notifyTaskStatus(name, "User export completed");
    };
    scheduleTask(name, taskFunc, time, TaskPriority::MEDIUM, false, std::chrono::seconds(0));
}

void Scheduler::scheduleUserImport(const std::string& name, const std::chrono::system_clock::time_point& time) {
    auto taskFunc = [this]() {
        userManagement.importUsers("path/to/import/file");
        logger.info("User import completed successfully.");
        notifyTaskStatus(name, "User import completed");
    };
    scheduleTask(name, taskFunc, time, TaskPriority::MEDIUM, false, std::chrono::seconds(0));
}

void Scheduler::taskRunner() {
    threadingUtils.runInThreads([this]() {
        while (true) {
            std::unique_lock<std::mutex> lock(taskMutex);
            if (stopFlag && taskQueue.empty()) {
                break;
            }
            if (!taskQueue.empty()) {
                auto task = taskQueue.top();
                if (task.time <= std::chrono::system_clock::now() && canExecuteTask(task.name)) {
                    taskQueue.pop();
                    lock.unlock();
                    executeTask(task);
                    lock.lock();
                } else {
                    taskCondition.wait_until(lock, task.time);
                }
            } else {
                taskCondition.wait(lock);
            }
        }
    });
}

void Scheduler::executeTask(const Task& task) {
    applyDatabaseSession();
    monitorDatabaseSession();
    executeTaskInternal(task);
}

void Scheduler::executeTaskInternal(const Task& task) {
    try {
        task.func();
        logTask(task.name, "completed");
        notifyTaskStatus(task.name, "completed");
    } catch (const std::exception& e) {
        logger.error("Error executing task: " + task.name + ", Error: " + e.what());
        logTask(task.name, "failed");
        notifyTaskStatus(task.name, "failed");
        retryTask(task, e.what());
    }
}

void Scheduler::retryTask(const Task& task, const std::string& errorMsg) {
    if (task.retryCount < maxRetries) {
        logger.info("Retrying task: " + task.name + " after error: " + errorMsg);
        Task newTask = task;
        newTask.retryCount++;
        newTask.time += std::chrono::seconds(static_cast<int>(std::pow(2, newTask.retryCount)));
        taskQueue.push(newTask);
        taskCondition.notify_all();
    } else {
        logger.error("Task " + task.name + " failed after " + std::to_string(maxRetries) + " retries.");
        createFailureReport(task, errorMsg);
    }
}

void Scheduler::logTask(const std::string& taskName, const std::string& status) {
    dataUtils.appendToFile("task_log.txt", "Task: " + taskName + " Status: " + status + "\n");
}

void Scheduler::logTaskState(const std::string& taskName, const std::string& state) {
    logger.info("Task " + taskName + " state: " + state);
}

void Scheduler::sendTaskNotification(const std::string& taskName, const std::string& message) {
    notificationManager->sendEmail("admin@example.com", "Task Notification: " + taskName, message);
}

void Scheduler::notifyTaskStatus(const std::string& taskName, const std::string& status) {
    notificationManager->sendEmail("admin@example.com", "Task Status: " + taskName, status);
}

void Scheduler::createFailureReport(const Task& task, const std::string& errorMsg) {
    dataUtils.writeToFile("failure_report_" + task.name + ".txt", "Task: " + task.name + "\nError: " + errorMsg + "\nRetries: " + std::to_string(task.retryCount) + "\n");
}

bool Scheduler::canExecuteTask(const std::string& taskName) const {
    auto it = taskDependencies.find(taskName);
    if (it != taskDependencies.end()) {
        for (const auto& dependency : it->second) {
            if (taskQueue.find(dependency) != taskQueue.end()) {
                return false;
            }
        }
    }
    return true;
}

void Scheduler::applyDatabaseSession() {
    dbManager.startSession();
}

void Scheduler::monitorDatabaseSession() {
    dbManager.monitorSessions();
}

void Scheduler::addTaskDependency(const std::string& taskName, const std::string& dependency) {
    addDependency(taskName, dependency);
}

bool Scheduler::checkTaskDependencies(const std::string& taskName) {
    return canExecuteTask(taskName);
}

void Scheduler::reloadConfig() {
    applyConfig();
}

void Scheduler::callExternalAPI(const std::string& apiEndpoint, const std::string& payload) {
    auto taskFunc = [this, apiEndpoint, payload]() {
        api.callAPI(apiEndpoint, payload);
    };
    auto now = std::chrono::system_clock::now();
    scheduleTask("ExternalAPI_" + apiEndpoint, taskFunc, now, TaskPriority::HIGH, false, std::chrono::seconds(0));
}

void Scheduler::addTask(const std::string& name, const TaskFunc& func) {
    auto now = std::chrono::system_clock::now();
    scheduleTask(name, func, now, TaskPriority::MEDIUM, false, std::chrono::seconds(0));
}

void Scheduler::removeTask(const std::string& name) {
    std::lock_guard<std::mutex> lock(taskMutex);
}

void Scheduler::updateTask(const std::string& name, const TaskFunc& func) {
    removeTask(name);
    addTask(name, func);
}

void Scheduler::integrateWithCalendar() {
}




