#ifndef ADAPTIVE_ATTACK_MANAGER_H
#define ADAPTIVE_ATTACK_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <future>
#include <queue>
#include <condition_variable>
#include "../attack/attack_engine.h"
#include "../analytics/analytics_manager.h"
#include "../monitoring/monitor.h"
#include "../machine_learning/ml_model_trainer.h"
#include "../machine_learning/ml_predictor.h"
#include "../policy/policy_manager.h"
#include "../rules/rule_engine.h"
#include "../dictionary/dictionary_loader.h"
#include "../utils/notification_utils.h"
#include "../utils/threading_utils.h"
#include "../gpu/gpu_manager.h"
#include "../database/db_manager.h"
#include "../cloud/cloud_integration.h"
#include "../users/user_management.h"
#include "../recovery/auto_recovery.h"
#include "../utils/data_utils.h"  

class AdaptiveAttackManager {
public:
    AdaptiveAttackManager(AttackEngine* attackEngine, AnalyticsManager* analyticsManager, Monitor* monitor, MLModelTrainer* mlModelTrainer, MLPredictor* mlPredictor, PolicyManager* policyManager, RuleEngine* ruleEngine, DictionaryLoader* dictionaryLoader, NotificationUtils* notificationUtils, ThreadingUtils* threadingUtils, GPUManager* gpuManager, DBManager* dbManager, CloudIntegration* cloudIntegration, UserManagement* userManagement, AutoRecovery* autoRecovery, DataUtils* dataUtils); // Добавлен DataUtils

    bool initialize(const std::map<std::string, std::string>& config);
    bool startAdaptiveAttack(const std::string& attackType, const std::map<std::string, std::string>& parameters);
    bool stopAdaptiveAttack();
    bool pauseAdaptiveAttack();
    bool resumeAdaptiveAttack();
    std::string getAdaptiveAttackStatus() const;
    std::vector<std::string> getAdaptiveAttackLogs() const;
    void setAdaptiveAttackParameters(const std::map<std::string, std::string>& params);
    std::map<std::string, std::string> getCurrentConfig() const;
    void saveState();
    void restoreState();
    void startRecoveryProcess(const std::string& dataId);
    bool verifyDataIntegrity(const std::string& data);
    void saveConfigVersion();
    void revertConfigVersion();

    void logStrategyChange(const std::string& strategy);
    void logResourceAllocation(const std::string& resource, int amount);

    void startAttackCLI(const std::string& attackType, const std::string& parameter = "");
    void stopAttackCLI();
    void pauseAttackCLI();
    void resumeAttackCLI();
    std::string getStatusCLI() const;

    void backupDatabase();
    void restoreDatabase();

private:
    AttackEngine* attackEngine;
    AnalyticsManager* analyticsManager;
    Monitor* monitor;
    MLModelTrainer* mlModelTrainer;
    MLPredictor* mlPredictor;
    PolicyManager* policyManager;
    RuleEngine* ruleEngine;
    DictionaryLoader* dictionaryLoader;
    NotificationUtils* notificationUtils;
    ThreadingUtils* threadingUtils;
    GPUManager* gpuManager;
    DBManager* dbManager;
    CloudIntegration* cloudIntegration;
    UserManagement* userManagement;
    AutoRecovery* autoRecovery;
    DataUtils* dataUtils; 
    std::map<std::string, std::string> adaptiveConfig;
    std::vector<std::string> logs;
    mutable std::mutex logsMutex;
    std::future<void> monitoringFuture;
    std::vector<std::thread> threadPool;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    bool stopThreads = false;

    void updateStrategy();
    bool validateParameters(const std::map<std::string, std::string>& params) const;
    void log(const std::string& message);
    void analyzeResults();
    void monitorAttackProgress();
    void scaleResources();
    void trainMLModel();
    void predictBestAttack();
    void integrateWithExternalSystems();
    void dynamicPolicyManagement();
    void automatedResponseMechanisms();
    void startThreadPool(size_t threadCount);
    void stopThreadPool();
    void threadLoop();
    void handleException(const std::exception& e);
    void logPerformanceMetrics();
};

#endif // ADAPTIVE_ATTACK_MANAGER_H


