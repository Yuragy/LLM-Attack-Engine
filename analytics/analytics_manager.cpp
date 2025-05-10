#include "analytics_manager.h"
#include <fstream>

AnalyticsManager::AnalyticsManager(DBManager* dbManager, CloudIntegration* cloudIntegration, RuleEngine* ruleEngine, Monitor* monitor, NotificationManager* notificationManager)
    : dbManager(dbManager), cloudIntegration(cloudIntegration), ruleEngine(ruleEngine), monitor(monitor), notificationManager(notificationManager) {
    if (!dbManager->connect()) {
        Logger::error("Failed to connect to the database");
        notificationManager->sendEmail("admin@example.com", "Database Connection Error", "Failed to connect to the database");
    }

    if (!ruleEngine->loadRules("rules_file.txt")) {
        Logger::error("Failed to load rules");
        notificationManager->sendEmail("admin@example.com", "Rule Loading Error", "Failed to load rules");
    }

    monitor->initialize("monitoring_config.json");
}

AnalyticsManager::~AnalyticsManager() {
    dbManager->disconnect();
}

double AnalyticsManager::evaluateRiskLevel() const {
    double riskLevel = calculateRiskLevel();
    logRiskLevel(riskLevel);
    notifyOnRiskLevel(riskLevel);
    monitor->monitorAnalytics("Risk Evaluation", std::to_string(riskLevel));

    return riskLevel;
}

void AnalyticsManager::logMetrics(const std::string& context, const std::map<std::string, double>& metrics) {
    for (const auto& [key, value] : metrics) {
        Logger::info(context + " metric " + key + ": " + std::to_string(value));
        currentMetrics[key] = value;
        std::string query = "INSERT INTO metrics (context, key, value) VALUES ('" + context + "', '" + key + "', " + std::to_string(value) + ")";
        dbManager->executeQuery(query);
        if (value > 100) {
            sendNotification("admin@example.com", "High value detected for " + key + ": " + std::to_string(value));
        }
    }

    auto transformedMetrics = applyRulesToMetrics();
    for (const auto& transformedMetric : transformedMetrics) {
        Logger::info("Transformed Metric: " + transformedMetric);
    }
    monitor->monitorAnalytics("Metrics Transformation", "Metrics transformed and logged.");
}

std::map<std::string, double> AnalyticsManager::getCurrentMetrics() const {
    return currentMetrics;
}

double AnalyticsManager::calculateRiskLevel() const {
    return ruleEngine->applyRules(currentMetrics).size();
}

void AnalyticsManager::logRiskLevel(double riskLevel) const {
    Logger::info("Current risk level: " + std::to_string(riskLevel));
    std::string query = "INSERT INTO risk_levels (level) VALUES (" + std::to_string(riskLevel) + ")";
    dbManager->executeQuery(query);
}

std::vector<std::string> AnalyticsManager::fetchExternalMetrics() const {
    std::string query = "SELECT metric_name FROM external_metrics";
    std::string result = dbManager->fetchData(query);
    
    std::vector<std::string> metrics;
    std::istringstream iss(result);
    for (std::string line; std::getline(iss, line); ) {
        metrics.push_back(line);
    }

    return metrics;
}

void AnalyticsManager::integrateWithExternalSystems() {
    auto externalMetrics = fetchExternalMetrics();
    for (const auto& metric : externalMetrics) {
        Logger::info("Fetched external metric: " + metric);
        currentMetrics[metric] = 1.0;
    }
}

void AnalyticsManager::generateReport(const std::string& reportName, const std::string& format) const {
    Logger::info("Generating report: " + reportName + " in format: " + format);
    
    dbManager->logDBOperation("Generate Report", "Success");

    std::ofstream reportFile(reportName + "." + format);
    for (const auto& [key, value] : currentMetrics) {
        reportFile << key << ": " << value << "\n";
    }
    reportFile.close();
}

std::vector<std::string> AnalyticsManager::getReportList() const {
    return {"report1.txt", "report2.csv", "report3.pdf"};
}

void AnalyticsManager::logAnalyticsData(const std::string& format) const {
    Logger::info("Logging analytics data in format: " + format);
    std::ofstream file("analytics_data." + format);
    for (const auto& [key, value] : currentMetrics) {
        file << key << ": " << value << "\n";
    }
    file.close();

    dbManager->logDBOperation("Log Analytics Data", "Success");
}

void AnalyticsManager::logMessage(const std::string& message, LogLevel level) {
    Logger::log(message, level);
}

std::string AnalyticsManager::viewRecentLogs(size_t numLines) const {
    return Logger::viewLogs(numLines);
}

std::string AnalyticsManager::filterLogs(LogLevel level, const std::string& tag, const std::string& regexPattern) const {
    return Logger::filterLogs(level, tag, regexPattern);
}

bool AnalyticsManager::backupAnalyticsData() {
    std::string dbPath = "analytics_db.db";  
    std::string backupPath = "cloud_backup/analytics_backup.db"; 
    return cloudIntegration->backupDatabase(dbPath, backupPath);
}

bool AnalyticsManager::restoreAnalyticsData() {
    std::string dbPath = "analytics_db.db";  
    std::string backupPath = "cloud_backup/analytics_backup.db"; 
    return cloudIntegration->restoreDatabase(backupPath, dbPath);
}

void AnalyticsManager::analyzeUserActivity() {
    Logger::info("Analyzing user activity...");
    dbManager->logEvent("Analyzing user activity", LogLevel::INFO);
}

void AnalyticsManager::generateUserActivityReport(const std::string& reportName, const std::string& format) const {
    Logger::info("Generating user activity report: " + reportName + " in format: " + format);
    std::ofstream reportFile(reportName + "_user_activity." + format);
    reportFile << "User activity data\n";
    reportFile.close();
}

void AnalyticsManager::sendNotification(const std::string& recipient, const std::string& message) {
    notificationManager->sendEmail(recipient, "Analytics Alert", message);
}

void AnalyticsManager::notifyOnRiskLevel(double riskLevel) {
    if (riskLevel > 75) {
        sendNotification("admin@example.com", "High Risk Level Detected: " + std::to_string(riskLevel));
    }
}

bool AnalyticsManager::loadRules(const std::string& filePath) {
    return ruleEngine->loadRules(filePath);
}

std::vector<std::string> AnalyticsManager::applyRulesToMetrics() const {
    std::vector<std::string> metrics;
    for (const auto& [key, value] : currentMetrics) {
        metrics.push_back(key + ": " + std::to_string(value));
    }
    return ruleEngine->applyRules(metrics);
}

void AnalyticsManager::monitorAnalytics(const std::string& analysisType, const std::string& result) {
    monitor->monitorAnalytics(analysisType, result);
}













