#include "db_manager.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <thread>
#include <sqlite3.h>

DBManager::DBManager(const std::string& dbPath, std::shared_ptr<DataUtils> dataUtils, std::shared_ptr<Logger> logger, 
                     std::shared_ptr<NotificationManager> notificationManager, std::shared_ptr<ThreadingUtils> threadingUtils, 
                     std::shared_ptr<Config> config)
    : dbPath(dbPath), connected(false), dataUtils(dataUtils), logger(logger), notificationManager(notificationManager), 
      threadingUtils(threadingUtils), config(config), db(nullptr) {
}

DBManager::~DBManager() {
    if (connected) {
        disconnect();
    }
}

bool DBManager::connect() {
    std::unique_lock<std::mutex> lock(connectionMutex);
    if (sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK) {
        connected = true;
        logDBOperation("Connect", "Success");
        return true;
    } else {
        logDBError("Failed to connect to the database");
        return false;
    }
}

void DBManager::disconnect() {
    std::unique_lock<std::mutex> lock(connectionMutex);
    if (db) {
        sqlite3_close(db);
        db = nullptr;
        connected = false;
        logDBOperation("Disconnect", "Success");
    }
}

bool DBManager::executeQuery(const std::string& query) {
    if (!connected) {
        logDBError("Attempt to execute query without connection");
        return false;
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::string error = errMsg ? errMsg : "Unknown error";
        logDBError(error);
        sqlite3_free(errMsg);

        notificationManager->sendEmail("admin@example.com", "Database Error", error);
        
        return false;
    }

    logDBOperation("Execute Query", "Success");

    notificationManager->sendPushNotification("device_token_here", "Query executed successfully");

    return true;
}

std::string DBManager::fetchData(const std::string& query) {
    if (!connected) {
        logDBError("Attempt to fetch data without connection");
        return "";
    }

    if (auto cachedResult = getCachedQueryResult(query); !cachedResult.empty()) {
        return cachedResult;
    }

    sqlite3_stmt* stmt;
    std::ostringstream result;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) << "\n";
        }
        sqlite3_finalize(stmt);
    } else {
        logDBError("Failed to fetch data");
    }

    auto data = result.str();
    cacheQueryResult(query, data);
    logDBOperation("Fetch Data", "Success");
    return data;
}

void DBManager::cacheQueryResult(const std::string& query, const std::string& result) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    queryCache[query] = result;
}

std::string DBManager::getCachedQueryResult(const std::string& query) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = queryCache.find(query);
    if (it != queryCache.end()) {
        return it->second;
    }
    return "";
}

void DBManager::logDBOperation(const std::string& operation, const std::string& status) {
    logger->info("DB Operation - " + operation + " | Status - " + status, {"DB", "Operation"});
    notificationManager->sendEmail("admin@example.com", "DB Operation - " + operation, "Status - " + status);

    config->logSystemParameterChange(operation, operation, status);
}

void DBManager::logDBError(const std::string& error) {
    logger->error("DB Error - " + error, {"DB", "Error"});
    notificationManager->sendEmail("admin@example.com", "DB Error", error);

    config->logSystemStatus("Database error occurred: " + error);
}

void DBManager::logQueryPerformance(const std::string& query, const std::chrono::duration<double>& duration) {
    std::string message = "DB Query Performance - " + query + " | Duration - " + std::to_string(duration.count()) + " seconds";
    logger->info(message, {"DB", "Performance"});
    notificationManager->sendPushNotification("device_token_here", message);

    config->logSystemParameterChange("QueryPerformance", query, message);
}

bool DBManager::validateData(const std::string& data) {
    return !data.empty() && data.size() < 1000 && std::all_of(data.begin(), data.end(), [](char c) { return std::isalnum(c) || std::isspace(c); });
}

nlohmann::json DBManager::parseAndValidateJSON(const std::string& jsonString, const std::string& schemaString) {
    if (!dataUtils->validateJSON(jsonString, schemaString)) {
        throw std::runtime_error("JSON validation failed");
    }
    return dataUtils->parseJSON(jsonString);
}

std::vector<std::unordered_map<std::string, std::string>> DBManager::parseAndValidateCSV(const std::string& csvString, const std::vector<std::string>& headers) {
    if (!dataUtils->validateCSV(csvString, headers)) {
        throw std::runtime_error("CSV validation failed");
    }
    return dataUtils->parseCSV(csvString);
}

nlohmann::json DBManager::parseAndValidateXML(const std::string& xmlString, const std::string& schemaString) {
    if (!dataUtils->validateXML(xmlString, schemaString)) {
        throw std::runtime_error("XML validation failed");
    }
    return dataUtils->parseXML(xmlString);
}

YAML::Node DBManager::parseAndValidateYAML(const std::string& yamlString, const std::string& schemaString) {
    if (!dataUtils->validateYAML(yamlString, schemaString)) {
        throw std::runtime_error("YAML validation failed");
    }
    return dataUtils->parseYAML(yamlString);
}

void DBManager::logEvent(const std::string& message, LogLevel level) {
    std::string levelStr = logLevelMap[level];
    std::string query = "INSERT INTO logs (message, level) VALUES ('" + message + "', '" + levelStr + "')";
    executeQuery(query);
}

void DBManager::monitorRealTime() {
    std::string query = "SELECT count(*) FROM sessions WHERE last_activity > datetime('now', '-10 minutes');";
    std::string result = fetchData(query);
    if (!result.empty()) {
        logDBOperation("Real-time Monitoring", "Active sessions in the last 10 minutes: " + result);
    }
}
bool DBManager::backupDatabase(const std::string& backupFilePath) {
    std::string command = "sqlite3 " + dbPath + " .backup " + backupFilePath;
    int result = system(command.c_str());
    if (result == 0) {
        logDBOperation("Backup Database", "Backup to " + backupFilePath + " successful");
        return true;
    } else {
        logDBError("Backup to " + backupFilePath + " failed");
        return false;
    }
}
bool DBManager::restoreDatabase(const std::string& backupFilePath) {
    std::string command = "sqlite3 " + dbPath + " .restore " + backupFilePath;
    int result = system(command.c_str());
    if (result == 0) {
        logDBOperation("Restore Database", "Restore from " + backupFilePath + " successful");
        return true;
    } else {
        logDBError("Restore from " + backupFilePath + " failed");
        return false;
    }
}












