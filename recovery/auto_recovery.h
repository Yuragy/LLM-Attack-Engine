#ifndef AUTO_RECOVERY_H
#define AUTO_RECOVERY_H

#include "../logging/logger.h"           
#include "../config/config.h"               
#include "../database/db_manager.h"        
#include "../notifications/notification_manager.h"  
#include "../cloud/cloud_integration.h"     
#include "../utils/threading_utils.h"       
#include <string>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <future>

class AutoRecovery {
public:
    AutoRecovery(std::shared_ptr<Config> config, 
                 std::shared_ptr<Logger> logger, 
                 std::shared_ptr<NotificationManager> notificationManager, 
                 std::shared_ptr<ThreadingUtils> threadingUtils,
                 std::shared_ptr<CloudIntegration> cloudIntegration,
                 std::shared_ptr<DBManager> dbManager);

    void startRecovery(const std::string& dataId);
    void addRecoveredData(const std::string& dataId, const std::string& data);
    bool removeRecoveredData(const std::string& dataId);
    bool containsData(const std::string& dataId) const;
    std::optional<std::string> getDataByIndex(size_t index) const;
    void clearData();
    bool backupDataToCloud(const std::string& dataId, const std::string& data);
    bool restoreDataFromCloud(const std::string& dataId);
    bool backupDatabase(const std::string& backupFilePath);
    bool restoreDatabase(const std::string& backupFilePath);
    void parallelBackupAndRestore(const std::vector<std::string>& dataIds);
    void sendNotification(const std::string& message);
    bool verifyDataIntegrity(const std::string& data);
    void reloadConfiguration(const std::string& configFilePath);

private:
    std::shared_ptr<DBManager> dbManager;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<Config> config;
    std::shared_ptr<NotificationManager> notificationManager;
    std::shared_ptr<ThreadingUtils> threadingUtils;
    std::shared_ptr<CloudIntegration> cloudIntegration;
    std::unordered_map<std::string, std::string> recoveredData;
    mutable std::mutex dataMutex;
    std::condition_variable dataCondition;
    std::future<bool> asyncBackupToCloud(const std::string& dataId, const std::string& data);
    std::future<bool> asyncRestoreFromCloud(const std::string& dataId);
};

#endif // AUTO_RECOVERY_H







