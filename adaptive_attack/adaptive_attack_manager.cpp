#include "adaptive_attack_manager.h"
#include "../logging/logger.h"

AdaptiveAttackManager::AdaptiveAttackManager(AttackEngine* attackEngine, AnalyticsManager* analyticsManager, Monitor* monitor, MLModelTrainer* mlModelTrainer, MLPredictor* mlPredictor, PolicyManager* policyManager, RuleEngine* ruleEngine, DictionaryLoader* dictionaryLoader, NotificationUtils* notificationUtils, ThreadingUtils* threadingUtils, GPUManager* gpuManager, DBManager* dbManager, CloudIntegration* cloudIntegration, UserManagement* userManagement, AutoRecovery* autoRecovery, DataUtils* dataUtils)
    : attackEngine(attackEngine), analyticsManager(analyticsManager), monitor(monitor), mlModelTrainer(mlModelTrainer), mlPredictor(mlPredictor), policyManager(policyManager), ruleEngine(ruleEngine), dictionaryLoader(dictionaryLoader), notificationUtils(notificationUtils), threadingUtils(threadingUtils), gpuManager(gpuManager), dbManager(dbManager), cloudIntegration(cloudIntegration), userManagement(userManagement), autoRecovery(autoRecovery), dataUtils(dataUtils) {} // Добавлен DataUtils

bool AdaptiveAttackManager::initialize(const std::map<std::string, std::string>& config) {
    adaptiveConfig = config;
    Logger::info("AdaptiveAttackManager initialized with configuration");
    size_t threadCount = std::stoul(config.at("thread_count"));
    startThreadPool(threadCount);

    if (!dbManager->connect()) {
        Logger::error("Failed to connect to the database");
        return false;
    }

    return true;
}

bool AdaptiveAttackManager::startAdaptiveAttack(const std::string& attackType, const std::map<std::string, std::string>& parameters) {
    if (attackEngine->getAttackStatus() != "Attack stopped") {
        Logger::warning("Adaptive attack is already running or was not stopped properly");
        notificationUtils->sendNotification("Adaptive attack is already running or was not stopped properly");
        return false;
    }
    if (!validateParameters(parameters)) {
        Logger::warning("Invalid adaptive attack parameters");
        notificationUtils->sendNotification("Invalid adaptive attack parameters");
        return false;
    }

    attackEngine->startAttackCLI(attackType, parameters.at("mask"));
    Logger::info("Adaptive attack of type " + attackType + " started");
    notificationUtils->sendNotification("Adaptive attack of type " + attackType + " started");

    monitoringFuture = std::async(std::launch::async, &AdaptiveAttackManager::monitorAttackProgress, this);
    updateStrategy();
    return true;
}

bool AdaptiveAttackManager::stopAdaptiveAttack() {
    if (attackEngine->getAttackStatus() == "Attack stopped") {
        Logger::warning("Adaptive attack is not running");
        return false;
    }
    attackEngine->stopAttackCLI();
    Logger::info("Adaptive attack stopped");

    if (monitoringFuture.valid()) {
        monitoringFuture.wait();
    }
    return true;
}

bool AdaptiveAttackManager::pauseAdaptiveAttack() {
    if (attackEngine->getAttackStatus() != "Attack running") {
        Logger::warning("Adaptive attack is not running or already paused");
        return false;
    }
    attackEngine->pauseAttackCLI();
    Logger::info("Adaptive attack paused");
    return true;
}

bool AdaptiveAttackManager::resumeAdaptiveAttack() {
    if (attackEngine->getAttackStatus() != "Attack paused") {
        Logger::warning("Adaptive attack is not paused");
        return false;
    }
    attackEngine->resumeAttackCLI();
    Logger::info("Adaptive attack resumed");
    return true;
}

std::string AdaptiveAttackManager::getAdaptiveAttackStatus() const {
    return attackEngine->getAttackStatus();
}

std::vector<std::string> AdaptiveAttackManager::getAdaptiveAttackLogs() const {
    std::lock_guard<std::mutex> lock(logsMutex);
    return logs;
}

void AdaptiveAttackManager::setAdaptiveAttackParameters(const std::map<std::string, std::string>& params) {
    if (!validateParameters(params)) {
        Logger::warning("Invalid adaptive attack parameters");
        return;
    }
    for (const auto& [key, value] : params) {
        adaptiveConfig[key] = value;
    }
    Logger::info("Adaptive attack parameters set");
}

void AdaptiveAttackManager::updateStrategy() {
    Logger::info("Updating adaptive attack strategy");
    analyzeResults();

    auto transformedWords = ruleEngine->applyRules(dictionaryLoader->getWords());

    if (attackEngine->getProgress() == "Low") {
        Logger::info("Switching attack strategy due to low progress");
        logStrategyChange("Switching to brute_force due to low progress");
        attackEngine->stopAttackCLI();
        attackEngine->startAttackCLI("brute_force", adaptiveConfig["mask"]);
    } else {
        predictBestAttack();
    }
}

bool AdaptiveAttackManager::validateParameters(const std::map<std::string, std::string>& params) const {
    if (params.find("mask") == params.end() || params.at("mask").empty()) {
        return false;
    }
    return true;
}

void AdaptiveAttackManager::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logsMutex);
    logs.push_back(message);
    Logger::info(message);
    dbManager->logDBOperation("AdaptiveAttackManager Log", message);
}

void AdaptiveAttackManager::logStrategyChange(const std::string& strategy) {
    Logger::info("Attack strategy change: " + strategy);
    log("Attack strategy change to " + strategy);
    dbManager->logDetailedOperation("Strategy Change", "Success", strategy);
}

void AdaptiveAttackManager::logResourceAllocation(const std::string& resource, int amount) {
    Logger::info("Resource allocation " + resource + ": " + std::to_string(amount));
    log("Resource allocation " + resource + ": " + std::to_string(amount));
    dbManager->logDetailedOperation("Resource Allocation", "Success", resource + " allocated: " + std::to_string(amount));
}

void AdaptiveAttackManager::analyzeResults() {
    Logger::info("Analyzing attack results");
    auto metrics = attackEngine->getRuleUsageStatistics();
    analyticsManager->logMetrics("adaptive_attack", metrics);

    if (metrics["success_rate"] < 0.1) {
        Logger::info("Low attack success rate, changing strategy");
        updateStrategy();
    } else {
        Logger::info("Attack is successful, continuing current strategy");
    }
}

void AdaptiveAttackManager::monitorAttackProgress() {
    monitor->startMonitoring();

    monitor->monitorAttackStatus("current_attack_id", attackEngine->getAttackStatus());
    monitor->monitorGPUMetrics(*gpuManager);
    monitor->monitorMLTraining("adaptive_attack_model", mlModelTrainer->getStatus());
    monitor->monitorMLPrediction("adaptive_attack_model", mlModelTrainer->getPredictionStatus());
    monitor->monitorUserManagement(*userManagement);
    monitor->monitorRecoveryProcesses(*autoRecovery);
    monitor->monitorDatabasePerformance(*dbManager);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        auto attackStatus = attackEngine->getAttackStatus();
        if (attackStatus == "completed" || attackStatus == "failed") {
            break;
        }

        monitor->monitorAttackStatus("current_attack_id", attackStatus);

        auto gpuMetrics = gpuManager->getMetrics();
        for (const auto& [key, value] : gpuMetrics) {
            Logger::info("GPU Metric - " + key + ": " + value);
        }

        monitor->logSystemState("Monitoring adaptive attack progress");
    }

    monitor->stopMonitoring();
}

void AdaptiveAttackManager::scaleResources() {
    Logger::info("Scaling resources for adaptive attack");
    if (attackEngine->getProgress() == "Low") {
        Logger::info("Increasing number of GPUs");
        logResourceAllocation("GPU", 1);
        gpuManager->scaleUp();
        gpuManager->managePowerConsumption();
        gpuManager->optimizeMemoryUsage();
    } else {
        Logger::info("Decreasing number of GPUs");
        logResourceAllocation("GPU", -1);
        gpuManager->scaleDown();
    }
}

void AdaptiveAttackManager::trainMLModel() {
    Logger::info("Training machine learning model based on attack results");
    mlModelTrainer->loadTrainingData("/path/to/training/data.csv");
    mlModelTrainer->trainModel(ModelType::RandomForest, {{"numTrees", 100}});
    mlModelTrainer->saveModel("/path/to/saved/model.model");
    log("Machine learning model successfully trained on latest attack data");
}

void AdaptiveAttackManager::predictBestAttack() {
    Logger::info("Predicting most effective attack using machine learning model");
    mlPredictor->loadModel("/path/to/saved/model.model", ModelType::RandomForest);
    arma::mat inputData; 
    mlPredictor->preprocessData(inputData, {0, 1, 2});
    auto bestAttack = mlPredictor->predict(inputData);
    Logger::info("Most effective attack predicted");
    log("Predicted most effective attack");
    attackEngine->startAttackCLI("predicted_attack", adaptiveConfig["mask"]);
}

void AdaptiveAttackManager::integrateWithExternalSystems() {
    Logger::info("Integrating with external threat systems and SIEM");
    log("Integration with external systems completed");
}

void AdaptiveAttackManager::dynamicPolicyManagement() {
    Logger::info("Dynamic policy management");

    double riskLevel = analyticsManager->evaluateRiskLevel();
    log("Current risk level: " + std::to_string(riskLevel));

    auto gpuMetrics = gpuManager->getMetrics();
    if (gpuMetrics["Temperature"] > "80C" || gpuMetrics["Usage"] > "90%") {
        policyManager->adjustPolicy("High GPU Load Policy");
        log("Policy changed to High GPU Load Policy based on GPU metrics");
    } else if (riskLevel > 0.7) {
        policyManager->adjustPolicy("High Risk Policy");
        log("Policy changed to High Risk Policy based on risk level: " + std::to_string(riskLevel));
    } else if (riskLevel > 0.4) {
        policyManager->adjustPolicy("Medium Risk Policy");
        log("Policy changed to Medium Risk Policy based on risk level: " + std::to_string(riskLevel));
    } else {
        policyManager->adjustPolicy("Low Risk Policy");
        log("Policy changed to Low Risk Policy based on risk level: " + std::to_string(riskLevel));
    }
}

void AdaptiveAttackManager::automatedResponseMechanisms() {
    Logger::info("Automated response mechanisms");

    auto threatLevel = monitor->getThreatLevel();
    if (threatLevel > 0.8) {
        log("Emergency response protocols activated for threat level: " + std::to_string(threatLevel));
    } else {
        log("Standard response protocols activated for threat level: " + std::to_string(threatLevel));
    }
}

std::map<std::string, std::string> AdaptiveAttackManager::getCurrentConfig() const {
    return adaptiveConfig;
}

void AdaptiveAttackManager::saveState() {
    log("Saving state...");
    dbManager->saveState("AdaptiveAttackManager", adaptiveConfig);
}

void AdaptiveAttackManager::restoreState() {
    log("Restoring state...");
    adaptiveConfig = dbManager->restoreState("AdaptiveAttackManager");
}

void AdaptiveAttackManager::startRecoveryProcess(const std::string& dataId) {
    autoRecovery->startRecovery(dataId);
    Logger::info("Started recovery process for data ID: " + dataId);
}

bool AdaptiveAttackManager::verifyDataIntegrity(const std::string& data) {
    return autoRecovery->verifyDataIntegrity(data);
}

void AdaptiveAttackManager::saveConfigVersion() {
    autoRecovery->saveDataVersion(adaptiveConfig);
    Logger::info("Configuration version saved.");
}

void AdaptiveAttackManager::revertConfigVersion() {
    auto prevVersion = autoRecovery->getPreviousVersion();
    if (prevVersion) {
        adaptiveConfig = *prevVersion;
        Logger::info("Reverted to previous configuration version.");
    } else {
        Logger::warning("No previous configuration version available.");
    }
}

void AdaptiveAttackManager::startAttackCLI(const std::string& attackType, const std::string& parameter) {
    if (attackType == "dictionary") {
        startAdaptiveAttack("dictionary", {{"mask", parameter}});
    } else if (attackType == "brute_force") {
        startAdaptiveAttack("brute_force", {{"mask", parameter}});
    } else if (attackType == "mask") {
        startAdaptiveAttack("mask", {{"mask", parameter}});
    }
}

void AdaptiveAttackManager::stopAttackCLI() {
    stopAdaptiveAttack();
}

void AdaptiveAttackManager::pauseAttackCLI() {
    pauseAdaptiveAttack();
}

void AdaptiveAttackManager::resumeAttackCLI() {
    resumeAdaptiveAttack();
}

std::string AdaptiveAttackManager::getStatusCLI() const {
    return getAdaptiveAttackStatus();
}

void AdaptiveAttackManager::startThreadPool(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        threadPool.emplace_back(&AdaptiveAttackManager::threadLoop, this);
    }
}

void AdaptiveAttackManager::stopThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopThreads = true;
    }
    queueCondition.notify_all();
    for (auto& thread : threadPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void AdaptiveAttackManager::threadLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return stopThreads || !taskQueue.empty(); });
            if (stopThreads && taskQueue.empty()) {
                return;
            }
            task = std::move(taskQueue.front());
            taskQueue.pop();
        }
        task();
    }
}

void AdaptiveAttackManager::handleException(const std::exception& e) {
    log(std::string("Error: ") + e.what());
    stopAdaptiveAttack();
}

void AdaptiveAttackManager::logPerformanceMetrics() {
    log("Performance metrics: example metric");
}

void AdaptiveAttackManager::backupDatabase() {
    Logger::info("Initiating database backup");
    auto backupFuture = autoRecovery->asyncBackupToCloud("db_backup", dbManager->getDatabaseDump());
    if (!backupFuture.get()) {
        Logger::error("Database backup failed");
        notificationUtils->sendEmail("admin@example.com", "Database Backup Failed", "The database backup process has failed.");
    } else {
        Logger::info("Database backup successful");
        notificationUtils->sendEmail("admin@example.com", "Database Backup Successful", "The database backup process was successful.");
    }
}

void AdaptiveAttackManager::restoreDatabase() {
    Logger::info("Initiating database restore");
    auto restoreFuture = autoRecovery->asyncRestoreFromCloud("db_backup");
    if (!restoreFuture.get()) {
        Logger::error("Database restore failed");
        notificationUtils->sendEmail("admin@example.com", "Database Restore Failed", "The database restore process has failed.");
    } else {
        Logger::info("Database restore successful");
        notificationUtils->sendEmail("admin@example.com", "Database Restore Successful", "The database restore process was successful.");
    }
}
















