#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>
#include <map>
#include "data_utils.h"
#include "logger.h"
#include "config.h"
#include "threading_utils.h"
#include "notification_manager.h"

class DBManager {
public:
    DBManager(const std::string& dbPath, std::shared_ptr<DataUtils> dataUtils, std::shared_ptr<Logger> logger, 
              std::shared_ptr<NotificationManager> notificationManager, std::shared_ptr<ThreadingUtils> threadingUtils, 
              std::shared_ptr<Config> config);
    ~DBManager();

    bool connect();
    void disconnect();
    bool executeQuery(const std::string& query);
    std::string fetchData(const std::string& query);
    void cacheQueryResult(const std::string& query, const std::string& result);
    std::string getCachedQueryResult(const std::string& query);
    void logDBOperation(const std::string& operation, const std::string& status);
    void logDBError(const std::string& error);
    void logQueryPerformance(const std::string& query, const std::chrono::duration<double>& duration);
    bool validateData(const std::string& data);
    void logEvent(const std::string& message, LogLevel level);
    void monitorRealTime();
    nlohmann::json parseAndValidateJSON(const std::string& jsonString, const std::string& schemaString);
    std::vector<std::unordered_map<std::string, std::string>> parseAndValidateCSV(const std::string& csvString, const std::vector<std::string>& headers);
    nlohmann::json parseAndValidateXML(const std::string& xmlString, const std::string& schemaString);
    YAML::Node parseAndValidateYAML(const std::string& yamlString, const std::string& schemaString);
    bool backupDatabase(const std::string& backupFilePath);
    bool restoreDatabase(const std::string& backupFilePath);

private:
    std::string dbPath;
    std::unordered_map<std::string, std::string> queryCache;
    std::mutex cacheMutex;
    bool connected;
    std::mutex connectionMutex;
    std::shared_ptr<DataUtils> dataUtils;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<NotificationManager> notificationManager;
    std::shared_ptr<ThreadingUtils> threadingUtils;
    std::shared_ptr<Config> config;

    sqlite3* db;
};

#endif // DB_MANAGER_H







