#ifndef APP_H
#define APP_H

#include <string>
#include <vector>
#include <map>
#include <crow.h>
#include "../dictionary/dictionary_loader.h"
#include "../rules/rule_engine.h"
#include "../attack/attack_engine.h"
#include "../gpu/gpu_manager.h"
#include "../users/user_management.h"
#include "../notifications/notification_manager.h"
#include "../analytics/analytics_manager.h"
#include "../ml/ml_model_trainer.h"
#include "../database/db_manager.h"
#include "../utils/data_utils.h"
#include "../utils/cloud_utils.h"

class WebApp {
public:
    bool initialize(const std::string &configPath);
    void run();
    void update();

private:
    crow::SimpleApp app;
    std::string configPath;
    DictionaryLoader dictLoader;
    RuleEngine ruleEngine;
    AttackEngine attackEngine;
    GPUManager gpuManager;
    UserManagement userManagement;
    NotificationManager notificationManager;
    AnalyticsManager analyticsManager;
    MLModelTrainer mlModelTrainer;
    DBManager dbManager;
    DataUtils dataUtils;
    CloudUtils cloudUtils;

    void setupRoutes();
    void handleRoot(const crow::request& req, crow::response& res);
    void handleStartAttack(const crow::request& req, crow::response& res);
    void handleStopAttack(const crow::request& req, crow::response& res);
    void handleStatus(const crow::request& req, crow::response& res);
    void handleConfig(const crow::request& req, crow::response& res);
    void handleLogs(const crow::request& req, crow::response& res);
    void handleAddUser(const crow::request& req, crow::response& res);
    void handleRemoveUser(const crow::request& req, crow::response& res);
    void handleUserManagement(const crow::request& req, crow::response& res);
    void handleSchedule(const crow::request& req, crow::response& res);
    void handleAnalytics(const crow::request& req, crow::response& res);
    void handleNotifications(const crow::request& req, crow::response& res);
    void handleDictionaryLoad(const crow::request& req, crow::response& res);
    void handleDictionarySave(const crow::request& req, crow::response& res);
    void handleDictionaryAdd(const crow::request& req, crow::response& res);
    void handleDictionaryStats(const crow::request& req, crow::response& res);
    void handleRulesLoad(const crow::request& req, crow::response& res);
    void handleRulesAdd(const crow::request& req, crow::response& res);
    void handleRulesSave(const crow::request& req, crow::response& res);
    void handleRulesStats(const crow::request& req, crow::response& res);
    void handleAttackStart(const crow::request& req, crow::response& res);
    void handleAttackStop(const crow::request& req, crow::response& res);
    void handleAttackStatus(const crow::request& req, crow::response& res);
    void handleGPUMonitoring(const crow::request& req, crow::response& res);
    void handleGPUMemoryOptimization(const crow::request& req, crow::response& res);
    void handleGPUPowerManagement(const crow::request& req, crow::response& res);
    void handleMLModelTrain(const crow::request& req, crow::response& res);
    void handleMLModelEvaluate(const crow::request& req, crow::response& res);
    void handleMLModelCrossValidate(const crow::request& req, crow::response& res);
    void handleMLModelReport(const crow::request& req, crow::response& res);
    void handleRoleAdd(const crow::request &req, crow::response &res);
    void handleRoleRemove(const crow::request &req, crow::response &res);
    void handleRoleAssign(const crow::request &req, crow::response &res);
    void handleRoleRevoke(const crow::request &req, crow::response &res);
    void handleCloudBackup(const crow::request &req, crow::response &res);
    void handleCloudRestore(const crow::request &req, crow::response &res);
    void handleCloudConfig(const crow::request &req, crow::response &res);
    void handleLogDownload(const crow::request &req, crow::response &res);
    void handleLogArchive(const crow::request &req, crow::response &res);
    void handleCustomNotificationCreate(const crow::request &req, crow::response &res);
    void handleMonitoringSettings(const crow::request &req, crow::response &res);
    void handleLogin(const crow::request &req, crow::response &res);
    void handleLogout(const crow::request &req, crow::response &res);
    void handleUserRoles(const crow::request &req, crow::response &res);
    void handleDBQuery(const crow::request &req, crow::response &res);
    void handleDBMonitor(const crow::request &req, crow::response &res);
    void handleDBBackup(const crow::request &req, crow::response &res);
    void handleDBRestore(const crow::request &req, crow::response &res);
    void logRequest(const crow::request &req);
    void logResponse(const crow::response &res);
    void logError(const std::string &errorMsg);
    void handleLogsPage(const crow::request &req, crow::response &res);
    void handleSchedulePage(const crow::request &req, crow::response &res);
    void handleUserManagementPage(const crow::request &req, crow::response &res);
    void handleReportsPage(const crow::request &req, crow::response &res);
    void handleLoginPage(const crow::request &req, crow::response &res);
    void handleExportLogsJSON(const crow::request &req, crow::response &res);
    void handleExportLogsXML(const crow::request &req, crow::response &res);
    void handleExportLogsCSV(const crow::request &req, crow::response &res);
    void handleCreateTask(const crow::request &req, crow::response &res);
    void handleDeleteTask(const crow::request &req, crow::response &res);
    void handleUpdateTask(const crow::request &req, crow::response &res);
    void handleGenerateReport(const crow::request &req, crow::response &res);
    void handleExportReport(const crow::request &req, crow::response &res);
    void handleTwoFactorAuth(const crow::request &req, crow::response &res);
    void handleSocialAuth(const crow::request &req, crow::response &res);
    void handleRealTimeNotifications(const crow::request &req, crow::response &res);
    void handleLanguageChange(const crow::request &req, crow::response &res);
    void handleThemeChange(const crow::request &req, crow::response &res);
    void sendNotification(const std::string &message);
};

#endif // APP_H






