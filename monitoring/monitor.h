#ifndef MONITOR_H
#define MONITOR_H

#include <string>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <map>
#include <chrono>
#include <memory>
#include <vector>
#include "../logging/logger.h"
#include "../config/config.h"
#include "../notification/notification_manager.h"
#include "../database/db_manager.h"
#include "../utils/threading_utils.h"
#include <matplotlibcpp.h>

class Monitor {
public:
    Monitor(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger, std::shared_ptr<DBManager> dbManager);
    ~Monitor();

    void initialize(const std::string& configFilePath);
    void startMonitoring();
    void stopMonitoring();
    void monitorAttackStatus(const std::string& attackId, const std::string& status);
    void monitorGPUMetrics(const std::map<std::string, double>& metrics);
    void monitorMLTraining(const std::string& modelId, const std::string& status);
    void monitorMLPrediction(const std::string& modelId, const std::string& status);
    void monitorRuleApplication(const std::string& ruleName, const std::string& target, bool success);
    void monitorDictionaryUsage(const std::string& dictionaryName, bool loaded);
    void monitorUserManagement(const std::string& username, const std::string& action, bool success);
    void monitorTaskScheduling(const std::string& taskId, const std::string& status);
    void monitorNotification(const std::string& notificationType, const std::string& recipient, bool success);
    void monitorWebApp(const std::string& endpoint, const std::string& status);
    void monitorCloudResources(const std::string& resourceName, const std::string& status);
    void monitorSocialEngineering(const std::string& campaignName, bool success);
    void monitorAnalytics(const std::string& analysisType, const std::string& result);
    void monitorCLIAndAPI(const std::string& requestType, const std::string& endpoint, bool success);
    void generateAttackStatusReport(std::ofstream& reportFile);
    void generateGPUMetricsReport(std::ofstream& reportFile);
    void generateMLTrainingReport(std::ofstream& reportFile);
    void generateMLPredictionReport(std::ofstream& reportFile);
    void generateRuleApplicationReport(std::ofstream& reportFile);
    void generateDictionaryUsageReport(std::ofstream& reportFile);
    void generateUserManagementReport(std::ofstream& reportFile);
    void generateTaskSchedulingReport(std::ofstream& reportFile);
    void generateNotificationReport(std::ofstream& reportFile);
    void generateWebAppReport(std::ofstream& reportFile);
    void generateCloudResourcesReport(std::ofstream& reportFile);
    void generateSocialEngineeringReport(std::ofstream& reportFile);
    void generateAnalyticsReport(std::ofstream& reportFile);
    void generateCLIAndAPIReport(std::ofstream& reportFile);
    void generateAttackStatusGraph(const std::string& outputPath);
    void generateGPUMetricsGraph(const std::string& outputPath);
    void generateMLTrainingGraph(const std::string& outputPath);
    void generateMLPredictionGraph(const std::string& outputPath);
    void generateRuleApplicationGraph(const std::string& outputPath);
    void generateDictionaryUsageGraph(const std::string& outputPath);
    void generateUserManagementGraph(const std::string& outputPath);
    void generateTaskSchedulingGraph(const std::string& outputPath);
    void generateNotificationGraph(const std::string& outputPath);
    void generateWebAppGraph(const std::string& outputPath);
    void generateCloudResourcesGraph(const std::string& outputPath);
    void generateSocialEngineeringGraph(const std::string& outputPath);
    void generateAnalyticsGraph(const std::string& outputPath);
    void generateCLIAndAPIGraph(const std::string& outputPath);
    void generateReports();
    void generateGraphs();

private:
    void loadMonitoringConfig(const std::string& configFilePath);
    void monitoringLoop();
    void sendCriticalNotification(const std::string& message);
    void logAndNotify(const std::string& message, LogLevel level, const std::unordered_set<std::string>& tags = {});
    void monitorDatabasePerformance();
    void monitorThreading();

    std::shared_ptr<Config> config;
    std::atomic<bool> isMonitoring;
    std::thread monitoringThread;
    std::map<std::string, int> notificationFrequency;

    NotificationManager notificationManager;
    std::shared_ptr<DBManager> dbManager;
    ThreadingUtils threadingUtils;
    std::vector<std::pair<std::string, std::string>> attackStatusHistory;
    std::map<std::string, std::vector<double>> gpuMetricsHistory;
    std::map<std::string, std::vector<std::string>> mlTrainingHistory;
    std::map<std::string, std::vector<std::string>> mlPredictionHistory;
    std::vector<std::tuple<std::string, std::string, bool>> ruleApplicationHistory;
    std::vector<std::pair<std::string, bool>> dictionaryUsageHistory;
    std::vector<std::tuple<std::string, std::string, bool>> userManagementHistory;
    std::vector<std::pair<std::string, std::string>> taskSchedulingHistory;
    std::vector<std::tuple<std::string, std::string, bool>> notificationHistory;
    std::vector<std::pair<std::string, std::string>> webAppHistory;
    std::vector<std::pair<std::string, std::string>> cloudResourcesHistory;
    std::vector<std::pair<std::string, bool>> socialEngineeringHistory;
    std::vector<std::pair<std::string, std::string>> analyticsHistory;
    std::vector<std::tuple<std::string, std::string, bool>> cliAndAPIHistory;
};

#endif // MONITOR_H








