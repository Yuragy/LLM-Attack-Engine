#include "cloud_integration.h"
#include <iostream>
#include <fstream>

CloudIntegration::CloudIntegration(const std::string& service, const std::string& apiKey, std::shared_ptr<Config> config, std::shared_ptr<ThreadingUtils> threadingUtils, std::shared_ptr<NotificationManager> notificationManager)
    : service(service), apiKey(apiKey), config(config), cloudUtils(std::make_shared<CloudUtils>(config, Logger::getInstance())), threadingUtils(threadingUtils), notificationManager(notificationManager) {
    logOperation("Initialization", "Success", "Cloud integration initialized with service: " + service);
    notifyEvent("CloudIntegrationInitialized", "Service: " + service);

    config->registerChangeCallback([this](const std::string& key, const Config::ConfigValue& oldValue, const Config::ConfigValue& newValue) {
        logOperation("Config Change", "Updated", "Key: " + key + ", New Value: " + std::get<std::string>(newValue));
    });
}

void CloudIntegration::setService(const std::string& service) {
    this->service = service;
    logOperation("Set Service", "Success", "Service set to: " + service);
    notifyEvent("CloudServiceSet", "Service: " + service);
}

void CloudIntegration::setApiKey(const std::string& apiKey) {
    this->apiKey = apiKey;
    logOperation("Set API Key", "Success", "API key set");
    notifyEvent("CloudApiKeySet", "API Key set successfully");
}

bool CloudIntegration::uploadData(const std::string& filePath, const std::string& cloudPath) {
    std::function<void()> task = [this, filePath, cloudPath]() {
        bool success = cloudUtils->uploadFile(filePath, cloudPath);
        if (success) {
            logOperation("Upload Data", "Success", "File uploaded to: " + cloudPath);
            notifyEvent("CloudUploadSuccess", "File uploaded to: " + cloudPath);
            notificationManager->sendEmail("admin@example.com", "Upload Success", "File " + filePath + " was successfully uploaded to " + cloudPath);
        } else {
            logError("Upload Data failed for file: " + filePath);
            notifyEvent("CloudUploadFailure", "Failed to upload file: " + filePath);
            notificationManager->sendEmail("admin@example.com", "Upload Failure", "Failed to upload file " + filePath + " to " + cloudPath);
        }
    };
    
    threadingUtils->runInParallel({task}, "default");
    return true;
}

bool CloudIntegration::downloadData(const std::string& cloudPath, const std::string& localPath) {
    std::function<void()> task = [this, cloudPath, localPath]() {
        bool success = cloudUtils->downloadFile(cloudPath, localPath);
        if (success) {
            logOperation("Download Data", "Success", "File downloaded to: " + localPath);
            notifyEvent("CloudDownloadSuccess", "File downloaded to: " + localPath);
            notificationManager->sendEmail("admin@example.com", "Download Success", "File " + cloudPath + " was successfully downloaded to " + localPath);
        } else {
            logError("Download Data failed for cloud path: " + cloudPath);
            notifyEvent("CloudDownloadFailure", "Failed to download file from: " + cloudPath);
            notificationManager->sendEmail("admin@example.com", "Download Failure", "Failed to download file from " + cloudPath + " to " + localPath);
        }
    };
    
    threadingUtils->runInParallel({task}, "default");
    return true;
}

bool CloudIntegration::backupDatabase(const std::string& dbPath, const std::string& backupPath) {
    std::function<void()> task = [this, dbPath, backupPath]() {
        DBManager dbManager(dbPath, nullptr, Logger::getInstance(), notificationManager, threadingUtils, config);
        if (!dbManager.connect()) {
            logError("Backup failed: could not connect to database - " + dbPath);
            notifyEvent("CloudBackupFailure", "Could not connect to database: " + dbPath);
            notificationManager->sendEmail("admin@example.com", "Backup Failure", "Failed to backup database " + dbPath);
            return;
        }

        bool result = dbManager.backupDatabase(backupPath);
        dbManager.disconnect();

        if (result) {
            logOperation("Backup Database", "Success", "Database backed up to: " + backupPath);
            notifyEvent("CloudBackupSuccess", "Database backed up to: " + backupPath);
            notificationManager->sendEmail("admin@example.com", "Backup Success", "Database " + dbPath + " was successfully backed up to " + backupPath);
        } else {
            logError("Backup Database failed for: " + dbPath);
            notifyEvent("CloudBackupFailure", "Failed to backup database: " + dbPath);
            notificationManager->sendEmail("admin@example.com", "Backup Failure", "Failed to backup database " + dbPath);
        }
    };
    
    threadingUtils->runInParallel({task}, "default");
    return true;
}

bool CloudIntegration::restoreDatabase(const std::string& backupPath, const std::string& dbPath) {
    std::function<void()> task = [this, backupPath, dbPath]() {
        DBManager dbManager(dbPath, nullptr, Logger::getInstance(), notificationManager, threadingUtils, config);
        if (!dbManager.connect()) {
            logError("Restore failed: could not connect to database - " + dbPath);
            notifyEvent("CloudRestoreFailure", "Could not connect to database: " + dbPath);
            notificationManager->sendEmail("admin@example.com", "Restore Failure", "Failed to restore database " + dbPath + " from " + backupPath);
            return;
        }

        bool result = dbManager.restoreDatabase(backupPath);
        dbManager.disconnect();

        if (result) {
            logOperation("Restore Database", "Success", "Database restored from: " + backupPath);
            notifyEvent("CloudRestoreSuccess", "Database restored from: " + backupPath);
            notificationManager->sendEmail("admin@example.com", "Restore Success", "Database " + dbPath + " was successfully restored from " + backupPath);
        } else {
            logError("Restore Database failed for: " + dbPath);
            notifyEvent("CloudRestoreFailure", "Failed to restore database: " + dbPath);
            notificationManager->sendEmail("admin@example.com", "Restore Failure", "Failed to restore database " + dbPath + " from " + backupPath);
        }
    };
    
    threadingUtils->runInParallel({task}, "default");
    return true;
}

bool CloudIntegration::autoBackup() {
    std::string dbPath = config->get("db_path").value_or("");
    std::string backupPath = config->get("backup_path").value_or("");
    return backupDatabase(dbPath, backupPath);
}

bool CloudIntegration::autoRestore() {
    std::string dbPath = config->get("db_path").value_or("");
    std::string backupPath = config->get("backup_path").value_or("");
    return restoreDatabase(backupPath, dbPath);
}

void CloudIntegration::logOperation(const std::string& operation, const std::string& status, const std::string& details, LogLevel level) {
    Logger::log("Cloud Operation - " + operation + " | Status - " + status + " | Details - " + details, level);
}

void CloudIntegration::logError(const std::string& error) {
    Logger::log("Cloud Error - " + error, LogLevel::ERROR);
}

void CloudIntegration::notifyEvent(const std::string& event, const std::string& details) {
    Logger::log("Cloud Event - " + event + " | Details - " + details, LogLevel::INFO);
    notificationManager->sendPushNotification("admin_device_token", event + ": " + details);
}

std::string CloudIntegration::getCacheKey(const std::string& path) {
    return std::to_string(std::hash<std::string>{}(path));
}

void CloudIntegration::cacheData(const std::string& key, const std::string& data) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache[key] = data;
}

std::string CloudIntegration::getCachedData(const std::string& key) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    return "";
}

bool CloudIntegration::uploadToS3(const std::string& filePath, const std::string& cloudPath) {
    return cloudUtils->uploadFile(filePath, cloudPath);
}

bool CloudIntegration::downloadFromS3(const std::string& cloudPath, const std::string& localPath) {
    return cloudUtils->downloadFile(cloudPath, localPath);
}

bool CloudIntegration::uploadToGCS(const std::string& filePath, const std::string& cloudPath) {
    return cloudUtils->uploadFile(filePath, cloudPath);
}

bool CloudIntegration::downloadFromGCS(const std::string& cloudPath, const std::string& localPath) {
    return cloudUtils->downloadFile(cloudPath, localPath);
}

std::future<bool> CloudIntegration::uploadToS3Async(const std::string& filePath, const std::string& cloudPath) {
    return std::async(std::launch::async, &CloudIntegration::uploadToS3, this, filePath, cloudPath);
}

std::future<bool> CloudIntegration::downloadFromS3Async(const std::string& cloudPath, const std::string& localPath) {
    return std::async(std::launch::async, &CloudIntegration::downloadFromS3, this, cloudPath, localPath);
}

std::future<bool> CloudIntegration::uploadToGCSAsync(const std::string& filePath, const std::string& cloudPath) {
    return std::async(std::launch::async, &CloudIntegration::uploadToGCS, this, filePath, cloudPath);
}

std::future<bool> CloudIntegration::downloadFromGCSAsync(const std::string& cloudPath, const std::string& localPath) {
    return std::async(std::launch::async, &CloudIntegration::downloadFromGCS, this, cloudPath, localPath);
}













