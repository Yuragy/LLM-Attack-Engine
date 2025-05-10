#ifndef API_H
#define API_H

#include <string>
#include <json/json.h>

bool authenticateUser(const std::string& token);
bool authorizeUser(const std::string& token, const std::string& permission);
Json::Value filterLogs(const std::string& logLevel, const std::string& logTag, const std::string& logDate, const std::string& logUser, const std::string& logAction);
Json::Value exportLogs(const std::string& format);
bool addUser(const Json::Value& userData);
bool editUser(const std::string& username, const Json::Value& userData);
bool deleteUser(const std::string& username);
Json::Value getUser(const std::string& username);
Json::Value getUsers();
bool addTask(const Json::Value& taskData);
bool editTask(const std::string& taskId, const Json::Value& taskData);
bool deleteTask(const std::string& taskId);
Json::Value getTasks();
Json::Value getTask(const std::string& taskId);
bool startAttack(const Json::Value& attackData);
bool stopAttack(const std::string& attackId);
Json::Value getActiveAttacks();
Json::Value getAttackHistory();
Json::Value getRules(const std::string& ruleType);
bool addRule(const std::string& ruleType, const Json::Value& ruleData);
bool editRule(const std::string& ruleType, const std::string& ruleId, const Json::Value& ruleData);
bool deleteRule(const std::string& ruleType, const std::string& ruleId);
bool sendNotification(const std::string& type, const Json::Value& notificationData);
bool addThreadedTask(const Json::Value& taskData);
bool manageThreadedTask(const std::string& taskId, const std::string& action);
Json::Value monitorThreadStatus(const std::string& taskId);
Json::Value fetchData(const std::string& dataId);
bool updateData(const std::string& dataId, const Json::Value& data);
bool sendGPUTask(const Json::Value& taskData);
Json::Value monitorGPUTask(const std::string& taskId);
Json::Value performAPIRequest(const std::string& endpoint, const Json::Value& requestData);
bool configureIntegration(const std::string& integrationName, const Json::Value& configData);
bool executeExternalService(const std::string& serviceName, const Json::Value& requestData);
Json::Value getExternalServiceStatus(const std::string& serviceName);

#endif // API_H




