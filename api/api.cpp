#include "api.h"
#include "web_app/app.h"
#include "logging/logger.h"
#include "users/user_management.h"
#include "scheduling/scheduler.h"
#include "rules/rule_engine.h"
#include "attack/attack_engine.h"
#include "database/db_manager.h"
#include "cache/cache_manager.h"
#include "notification/notification_utils.h"
#include "data/data_utils.h"
#include "gpu/gpu_manager.h"
#include <json/json.h>
#include <unordered_map>
#include <iostream>
#include <curl/curl.h> 

bool authenticateUser(const std::string& token) {
    if (token.empty()) {
        Logger::log("Authentication failed: empty token", "ERROR");
        return false;
    }
    bool isValid = DbManager::verifyToken(token);
    if (!isValid) {
        Logger::log("Authentication failed: invalid token", "ERROR");
    }
    return isValid;
}

bool authorizeUser(const std::string& token, const std::string& permission) {
    if (!authenticateUser(token)) {
        return false;
    }
    bool hasPermission = DbManager::checkUserPermission(token, permission);
    if (!hasPermission) {
        Logger::log("Authorization failed: insufficient permissions", "ERROR");
    }
    return hasPermission;
}

Json::Value filterLogs(const std::string& logLevel, const std::string& logTag, const std::string& logDate, const std::string& logUser, const std::string& logAction) {
    Logger::log("Filtering logs", "INFO");
    Json::Value logs = Logger::filter(logLevel, logTag, logDate, logUser, logAction);
    return logs;
}

Json::Value exportLogs(const std::string& format) {
    Logger::log("Exporting logs in format: " + format, "INFO");
    Json::Value exportedLogs = Logger::exportLogs(format);
    return exportedLogs;
}

bool addUser(const Json::Value& userData) {
    Logger::log("Adding user: " + userData["username"].asString(), "INFO");
    return UserManager::addUser(userData);
}

bool editUser(const std::string& username, const Json::Value& userData) {
    Logger::log("Editing user: " + username, "INFO");
    return UserManager::editUser(username, userData);
}

bool deleteUser(const std::string& username) {
    Logger::log("Deleting user: " + username, "INFO");
    return UserManager::deleteUser(username);
}

Json::Value getUser(const std::string& username) {
    Logger::log("Fetching user: " + username, "INFO");
    Json::Value user = CacheManager::get("user_" + username);
    if (user.isNull()) {
        user = UserManager::getUser(username);
        CacheManager::set("user_" + username, user);
    }
    return user;
}

Json::Value getUsers() {
    Logger::log("Fetching all users", "INFO");
    Json::Value users = CacheManager::get("users");
    if (users.isNull()) {
        users = UserManager::getUsers();
        CacheManager::set("users", users);
    }
    return users;
}

bool addTask(const Json::Value& taskData) {
    Logger::log("Adding task: " + taskData["name"].asString(), "INFO");
    return Scheduler::addTask(taskData);
}

bool editTask(const std::string& taskId, const Json::Value& taskData) {
    Logger::log("Editing task: " + taskId, "INFO");
    return Scheduler::editTask(taskId, taskData);
}

bool deleteTask(const std::string& taskId) {
    Logger::log("Deleting task: " + taskId, "INFO");
    return Scheduler::deleteTask(taskId);
}

Json::Value getTasks() {
    Logger::log("Fetching all tasks", "INFO");
    Json::Value tasks = CacheManager::get("tasks");
    if (tasks.isNull()) {
        tasks = Scheduler::getTasks();
        CacheManager::set("tasks", tasks);
    }
    return tasks;
}

Json::Value getTask(const std::string& taskId) {
    Logger::log("Fetching task: " + taskId, "INFO");
    Json::Value task = CacheManager::get("task_" + taskId);
    if (task.isNull()) {
        task = Scheduler::getTask(taskId);
        CacheManager::set("task_" + taskId, task);
    }
    return task;
}

bool startAttack(const Json::Value& attackData) {
    Logger::log("Starting attack: " + attackData["type"].asString(), "INFO");
    return AttackEngine::startAttack(attackData);
}

bool stopAttack(const std::string& attackId) {
    Logger::log("Stopping attack: " + attackId, "INFO");
    return AttackEngine::stopAttack(attackId);
}

Json::Value getActiveAttacks() {
    Logger::log("Fetching active attacks", "INFO");
    Json::Value attacks = CacheManager::get("active_attacks");
    if (attacks.isNull()) {
        attacks = AttackEngine::getActiveAttacks();
        CacheManager::set("active_attacks", attacks);
    }
    return attacks;
}

Json::Value getAttackHistory() {
    Logger::log("Fetching attack history", "INFO");
    Json::Value history = CacheManager::get("attack_history");
    if (history.isNull()) {
        history = AttackEngine::getAttackHistory();
        CacheManager::set("attack_history", history);
    }
    return history;
}

Json::Value getRules(const std::string& ruleType) {
    Logger::log("Fetching rules of type: " + ruleType, "INFO");
    Json::Value rules = CacheManager::get("rules_" + ruleType);
    if (rules.isNull()) {
        rules = RuleEngine::getRules(ruleType);
        CacheManager::set("rules_" + ruleType, rules);
    }
    return rules;
}

bool addRule(const std::string& ruleType, const Json::Value& ruleData) {
    Logger::log("Adding rule of type: " + ruleType, "INFO");
    return RuleEngine::addRule(ruleType, ruleData);
}

bool editRule(const std::string& ruleType, const std::string& ruleId, const Json::Value& ruleData) {
    Logger::log("Editing rule: " + ruleId + " of type: " + ruleType, "INFO");
    return RuleEngine::editRule(ruleType, ruleId, ruleData);
}

bool deleteRule(const std::string& ruleType, const std::string& ruleId) {
    Logger::log("Deleting rule: " + ruleId + " of type: " + ruleType, "INFO");
    return RuleEngine::deleteRule(ruleType, ruleId);
}

bool sendNotification(const std::string& type, const Json::Value& notificationData) {
    Logger::log("Sending notification of type: " + type, "INFO");
    return NotificationUtils::sendNotification(type, notificationData);
}

bool addThreadedTask(const Json::Value& taskData) {
    Logger::log("Adding threaded task: " + taskData["name"].asString(), "INFO");
    return Scheduler::addThreadedTask(taskData);
}

bool manageThreadedTask(const std::string& taskId, const std::string& action) {
    Logger::log("Managing threaded task: " + taskId + " with action: " + action, "INFO");
    return Scheduler::manageThreadedTask(taskId, action);
}

Json::Value monitorThreadStatus(const std::string& taskId) {
    Logger::log("Monitoring status of thread: " + taskId, "INFO");
    return Scheduler::getThreadStatus(taskId);
}

Json::Value fetchData(const std::string& dataId) {
    Logger::log("Fetching data: " + dataId, "INFO");
    Json::Value data = CacheManager::get("data_" + dataId);
    if (data.isNull()) {
        data = DataUtils::fetchData(dataId);
        CacheManager::set("data_" + dataId, data);
    }
    return data;
}

bool updateData(const std::string& dataId, const Json::Value& data) {
    Logger::log("Updating data: " + dataId, "INFO");
    bool result = DataUtils::updateData(dataId, data);
    if (result) {
        CacheManager::set("data_" + dataId, data);
    }
    return result;
}

bool sendGPUTask(const Json::Value& taskData) {
    Logger::log("Sending GPU task: " + taskData["name"].asString(), "INFO");
    return GPUManager::sendTask(taskData);
}

Json::Value monitorGPUTask(const std::string& taskId) {
    Logger::log("Monitoring GPU task: " + taskId, "INFO");
    return GPUManager::monitorTask(taskId);
}

Json::Value performAPIRequest(const std::string& endpoint, const Json::Value& requestData) {
    Logger::log("Performing API request to endpoint: " + endpoint, "INFO");

    CURL* curl;
    CURLcode res;
    Json::Value jsonResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.toStyledString().c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, void* userp) -> size_t {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        });

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            Logger::log("API request failed: " + std::string(curl_easy_strerror(res)), "ERROR");
        } else {
            Json::CharReaderBuilder readerBuilder;
            std::string errs;
            std::istringstream s(response);
            if (!Json::parseFromStream(readerBuilder, s, &jsonResponse, &errs)) {
                Logger::log("Failed to parse API response: " + errs, "ERROR");
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return jsonResponse;
}

bool configureIntegration(const std::string& integrationName, const Json::Value& configData) {
    Logger::log("Configuring integration: " + integrationName, "INFO");

    bool result = DbManager::saveIntegrationConfig(integrationName, configData);
    if (result) {
        Logger::log("Integration configured successfully: " + integrationName, "INFO");
    } else {
        Logger::log("Failed to configure integration: " + integrationName, "ERROR");
    }

    return result;
}

bool executeExternalService(const std::string& serviceName, const Json::Value& requestData) {
    Logger::log("Executing external service: " + serviceName, "INFO");
    Json::Value response = performAPIRequest("https://external-service.com/api/" + serviceName, requestData);
    if (!response.isNull()) {
        Logger::log("External service " + serviceName + " executed successfully.", "INFO");
        return true;
    } else {
        Logger::log("Failed to execute external service " + serviceName, "ERROR");
        return false;
    }
}

Json::Value getExternalServiceStatus(const std::string& serviceName) {
    Logger::log("Fetching status for external service: " + serviceName, "INFO");
    Json::Value status = performAPIRequest("https://external-service.com/api/" + serviceName + "/status", Json::Value());
    if (!status.isNull()) {
        Logger::log("Status for external service " + serviceName + " fetched successfully.", "INFO");
    } else {
        Logger::log("Failed to fetch status for external service " + serviceName, "ERROR");
    }
    return status;
}



