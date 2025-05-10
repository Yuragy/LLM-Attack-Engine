#include "auto_recovery.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <unordered_set>

AutoRecovery::AutoRecovery(std::shared_ptr<Config> config, 
                           std::shared_ptr<Logger> logger, 
                           std::shared_ptr<NotificationManager> notificationManager, 
                           std::shared_ptr<ThreadingUtils> threadingUtils,
                           std::shared_ptr<CloudIntegration> cloudIntegration,
                           std::shared_ptr<DBManager> dbManager)
    : config(config), 
      logger(logger), 
      notificationManager(notificationManager), 
      threadingUtils(threadingUtils), 
      cloudIntegration(cloudIntegration),
      dbManager(dbManager) {
    if (!config->validate()) {
        logger->error("Invalid configuration during auto recovery initialization.");
        throw std::runtime_error("Invalid configuration.");
    }
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database during auto recovery initialization.", {"AutoRecovery", "Database"});
    } else {
        logger->info("Successfully connected to the database during auto recovery initialization.", {"AutoRecovery", "Database"});
    }
}

void AutoRecovery::startRecovery(const std::string& dataId) {
    logger->info("Starting recovery process for data ID: " + dataId, {"AutoRecovery"});
    try {
        std::unique_lock<std::mutex> lock(dataMutex);
        dataCondition.wait(lock, [this, &dataId] { return containsData(dataId); });
        
        dbManager->logDBOperation("Recovery", "Success for data ID: " + dataId);

        std::string templateMessage = notificationManager->getTemplate("recovery_success");
        notificationManager->sendEmail("admin@example.com", "Recovery Success", templateMessage);

        sendNotification("Recovery process completed for data ID: " + dataId);
        logger->info("Recovery process completed for data ID: " + dataId, {"AutoRecovery"});
    } catch (const std::exception& e) {
        logger->error("Error in recovery process for data ID " + dataId + ": " + std::string(e.what()), {"AutoRecovery"});
        dbManager->logDBError("Recovery process failed for data ID: " + dataId);

        std::string templateMessage = notificationManager->getTemplate("recovery_failure");
        notificationManager->sendEmail("admin@example.com", "Recovery Failed", templateMessage);

        sendNotification("Recovery process failed for data ID: " + dataId);
    }
}

void AutoRecovery::parallelBackupAndRestore(const std::vector<std::string>& dataIds) {
    std::vector<std::function<void()>> tasks;
    
    for (const auto& dataId : dataIds) {
        tasks.push_back([this, dataId] {
            if (!backupDataToCloud(dataId, recoveredData[dataId])) {
                logger->error("Failed to backup data for ID: " + dataId, {"AutoRecovery"});
            }
            if (!restoreDataFromCloud(dataId)) {
                logger->error("Failed to restore data for ID: " + dataId, {"AutoRecovery"});
            }
        });
    }
    threadingUtils->runInParallel(tasks, "default");
}

bool AutoRecovery::backupDatabase(const std::string& backupFilePath) {
    return dbManager->backupDatabase(backupFilePath);
}

bool AutoRecovery::restoreDatabase(const std::string& backupFilePath) {
    return dbManager->restoreDatabase(backupFilePath);
}

void AutoRecovery::addRecoveredData(const std::string& dataId, const std::string& data) {
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        recoveredData[dataId] = data;
        logger->info("Data added for recovery. ID: " + dataId, {"AutoRecovery", "Data"});
    }
    dataCondition.notify_all();
}

bool AutoRecovery::removeRecoveredData(const std::string& dataId) {
    std::lock_guard<std::mutex> lock(dataMutex);
    if (recoveredData.erase(dataId)) {
        logger->info("Data removed from recovery. ID: " + dataId, {"AutoRecovery", "Data"});
        return true;
    }
    logger->warning("Data not found for removal. ID: " + dataId, {"AutoRecovery", "Data"});
    return false;
}

bool AutoRecovery::containsData(const std::string& dataId) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return recoveredData.find(dataId) != recoveredData.end();
}

std::optional<std::string> AutoRecovery::getDataByIndex(size_t index) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    if (index >= recoveredData.size()) {
        logger->warning("Invalid index requested in getDataByIndex.", {"AutoRecovery", "Data"});
        return std::nullopt;
    }
    auto it = recoveredData.begin();
    std::advance(it, index);
    return it->first;
}

void AutoRecovery::clearData() {
    std::lock_guard<std::mutex> lock(dataMutex);
    recoveredData.clear();
    logger->info("All recovered data cleared.", {"AutoRecovery", "Data"});
}

bool AutoRecovery::backupDataToCloud(const std::string& dataId, const std::string& data) {
    try {
        if (cloudIntegration->uploadData(dataId, data)) {
            logger->info("Data backed up to cloud. ID: " + dataId, {"AutoRecovery", "Cloud"});
            dbManager->logDBOperation("Backup to Cloud", "Success for data ID: " + dataId);
            return true;
        }
    } catch (const std::exception& e) {
        logger->error("Failed to back up data to cloud. ID: " + dataId + ". Error: " + std::string(e.what()), {"AutoRecovery", "Cloud"});
        dbManager->logDBError("Backup to Cloud failed for data ID: " + dataId);
    }
    return false;
}

bool AutoRecovery::restoreDataFromCloud(const std::string& dataId) {
    try {
        std::string data = cloudIntegration->downloadData(dataId);
        if (verifyDataIntegrity(data)) {
            addRecoveredData(dataId, data);
            logger->info("Data restored from cloud. ID: " + dataId, {"AutoRecovery", "Cloud"});
            dbManager->logDBOperation("Restore from Cloud", "Success for data ID: " + dataId);
            return true;
        }
    } catch (const std::exception& e) {
        logger->error("Failed to restore data from cloud. ID: " + dataId + ". Error: " + std::string(e.what()), {"AutoRecovery", "Cloud"});
        dbManager->logDBError("Restore from Cloud failed for data ID: " + dataId);
    }
    return false;
}

void AutoRecovery::sendNotification(const std::string& message) {
    try {
        notificationManager->sendEmail("admin@example.com", "Notification", message);
        logger->info("Notification sent: " + message, {"AutoRecovery", "Notification"});
    } catch (const std::exception& e) {
        logger->error("Failed to send notification. Error: " + std::string(e.what()), {"AutoRecovery", "Notification"});
    }
}

std::future<bool> AutoRecovery::asyncBackupToCloud(const std::string& dataId, const std::string& data) {
    return std::async(std::launch::async, &AutoRecovery::backupDataToCloud, this, dataId, data);
}

std::future<bool> AutoRecovery::asyncRestoreFromCloud(const std::string& dataId) {
    return std::async(std::launch::async, &AutoRecovery::restoreDataFromCloud, this, dataId);
}

bool AutoRecovery::verifyDataIntegrity(const std::string& data) {
    if (data.empty()) {
        logger->error("Data integrity check failed: data is empty.", {"AutoRecovery", "Integrity"});
        return false;
    }
    logger->info("Data integrity check passed.", {"AutoRecovery", "Integrity"});
    return true;
}

void AutoRecovery::reloadConfiguration(const std::string& configFilePath) {
    if (config->reloadConfig(configFilePath)) {
        logger->info("Configuration reloaded successfully from: " + configFilePath, {"AutoRecovery", "Config"});
    } else {
        logger->error("Failed to reload configuration from: " + configFilePath, {"AutoRecovery", "Config"});
    }
}












