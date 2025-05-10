#ifndef FINGERPRINT_ATTACK_H
#define FINGERPRINT_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "../database/db_manager.h"
#include <unordered_map>
#include <string>
#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

class FingerprintAttack {
public:
    FingerprintAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, Logger* logger, 
                      ThreadingUtils* threadingUtils, DBManager* dbManager, 
                      const std::string& threadingStrategy);

    void execute();
    void setFingerprintVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> fingerprintVerificationCallback;

    std::string threadingStrategy;
    std::mutex mtx;
    std::queue<std::string> fingerprintQueue;
    std::atomic<bool> stopFlag = false;

    void fingerprintWorker();
    std::future<void> loadFingerprintDataAsync();
    void applyMachineLearningModel(std::unordered_map<std::string, std::string>& fingerprintData);
    void applyRulesToFingerprintData(std::unordered_map<std::string, std::string>& fingerprintData);
    void logFingerprintAttackDetails(const std::string& data);

    void evaluateModel();
    void analyzeErrors();
    void manageResources();

    bool checkIfStop();
    void loadFingerprintData();
};

#endif // FINGERPRINT_ATTACK_H


