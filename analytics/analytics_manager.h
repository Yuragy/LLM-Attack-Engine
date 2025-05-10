#ifndef ANALYTICS_MANAGER_H
#define ANALYTICS_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include "../cloud/cloud_integration.h"  
#include "../logging/logger.h"           
#include "../database/db_manager.h"      
#include "../notification_manager.h"     
#include "../rules/rule_engine.h"        
#include "../monitoring/monitor.h"       

class AnalyticsManager {
public:
    AnalyticsManager(DBManager* dbManager, CloudIntegration* cloudIntegration, RuleEngine* ruleEngine, Monitor* monitor, NotificationManager* notificationManager);
    ~AnalyticsManager();

    double evaluateRiskLevel() const;
    void logMetrics(const std::string& context, const std::map<std::string, double>& metrics);
    std::map<std::string, double> getCurrentMetrics() const;
    void integrateWithExternalSystems();
    void generateReport(const std::string& reportName, const std::string& format) const;
    std::vector<std::string> getReportList() const;
    void logAnalyticsData(const std::string& format) const;

    void logMessage(const std::string& message, LogLevel level);
    std::string viewRecentLogs(size_t numLines = 100) const;
    std::string filterLogs(LogLevel level, const std::string& tag = "", const std::string& regexPattern = "") const;

    bool backupAnalyticsData();
    bool restoreAnalyticsData();

    void analyzeUserActivity();
    void generateUserActivityReport(const std::string& reportName, const std::string& format) const;

    void sendNotification(const std::string& recipient, const std::string& message);
    void notifyOnRiskLevel(double riskLevel);


    bool loadRules(const std::string& filePath);
    std::vector<std::string> applyRulesToMetrics() const;

    void monitorAnalytics(const std::string& analysisType, const std::string& result);

private:
    std::map<std::string, double> currentMetrics;
    DBManager* dbManager;
    CloudIntegration* cloudIntegration;
    RuleEngine* ruleEngine;
    Monitor* monitor;
    NotificationManager* notificationManager;

    double calculateRiskLevel() const;
    void logRiskLevel(double riskLevel) const;
    std::vector<std::string> fetchExternalMetrics() const;
};

#endif // ANALYTICS_MANAGER_H



