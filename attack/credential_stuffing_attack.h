#ifndef CREDENTIAL_STUFFING_ATTACK_H
#define CREDENTIAL_STUFFING_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../dictionary/dictionary_loader.h"
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

class CredentialStuffingAttack {
public:
    CredentialStuffingAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                             Logger* logger, ThreadingUtils* threadingUtils, 
                             DBManager* dbManager);

    void execute();
    void setCredentialVerificationCallback(std::function<bool(const std::string&, const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&, const std::string&)> credentialVerificationCallback;

    std::mutex mtx;
    std::queue<std::pair<std::string, std::string>> credentialsQueue;
    std::atomic<bool> stopFlag = false;

    std::future<void> loadCredentialsAsync();
    std::unordered_map<std::string, std::string> loadStolenCredentials(const std::string& filePath);
    void applyMachineLearningModel(std::unordered_map<std::string, std::string>& stolenCredentials);
    void applyRulesToCredentials(std::unordered_map<std::string, std::string>& stolenCredentials);
    void logCredentialAttackDetails(const std::string& username, const std::string& password);

    void credentialsWorker();
    bool checkIfStop();
    void evaluateModel();
    void analyzeErrors();
    void manageResources();
};

#endif // CREDENTIAL_STUFFING_ATTACK_H


