#ifndef CLOUD_INTEGRATION_H
#define CLOUD_INTEGRATION_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <future>
#include "../logging/logger.h"
#include "../config/config.h"
#include "../notifications/notification_manager.h"
#include "../utils/threading_utils.h"
#include "../utils/cloud_utils.h"
#include "../database/db_manager.h"

class CloudIntegration {
public:
    CloudIntegration(const std::string& service, const std::string& apiKey, std::shared_ptr<Config> config, std::shared_ptr<ThreadingUtils> threadingUtils, std::shared_ptr<NotificationManager> notificationManager);

    bool uploadData(const std::string& filePath, const std::string& cloudPath);
    bool downloadData(const std::string& cloudPath, const std::string& localPath);
    bool backupDatabase(const std::string& dbPath, const std::string& backupPath);
    bool restoreDatabase(const std::string& backupPath, const std::string& dbPath);

    bool autoBackup();
    bool autoRestore();

    void setService(const std::string& service);
    void setApiKey(const std::string& apiKey);

private:
    std::string service;
    std::string apiKey;
    std::unordered_map<std::string, std::string> cache;
    std::mutex cacheMutex;
    std::shared_ptr<Config> config;
    std::shared_ptr<CloudUtils> cloudUtils;
    std::shared_ptr<ThreadingUtils> threadingUtils;
    std::shared_ptr<NotificationManager> notificationManager;

    void logOperation(const std::string& operation, const std::string& status, const std::string& details = "", LogLevel level = LogLevel::INFO);
    void logError(const std::string& error);
    void notifyEvent(const std::string& event, const std::string& details = "");

    std::string getCacheKey(const std::string& path);
    void cacheData(const std::string& key, const std::string& data);
    std::string getCachedData(const std::string& key);

    bool uploadToS3(const std::string& filePath, const std::string& cloudPath);
    bool downloadFromS3(const std::string& cloudPath, const std::string& localPath);
    bool uploadToGCS(const std::string& filePath, const std::string& cloudPath);
    bool downloadFromGCS(const std::string& cloudPath, const std::string& localPath);

    std::future<bool> uploadToS3Async(const std::string& filePath, const std::string& cloudPath);
    std::future<bool> downloadFromS3Async(const std::string& cloudPath, const std::string& localPath);
    std::future<bool> uploadToGCSAsync(const std::string& filePath, const std::string& cloudPath);
    std::future<bool> downloadFromGCSAsync(const std::string& cloudPath, const std::string& localPath);
};

#endif // CLOUD_INTEGRATION_H










