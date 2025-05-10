#ifndef MASK_ATTACK_H
#define MASK_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "../database/db_manager.h"
#include <string>
#include <vector>
#include <future>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

class MaskAttack {
public:
    MaskAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
               Logger* logger, ThreadingUtils* threadingUtils, 
               DBManager* dbManager, const std::string& threadingStrategy);

    void execute();
    void setPasswordVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> passwordVerificationCallback;

    std::string threadingStrategy;
    std::mutex mtx;
    std::queue<std::string> passwordQueue;
    std::atomic<bool> stopFlag = false;

    void maskWorker();
    std::future<void> loadMasksAsync();
    std::vector<std::string> loadMasksFromFile();
    std::string generatePassword(const std::string& mask, const std::unordered_map<char, std::string>& charsets, const std::vector<int>& indices);
    void applyMachineLearningModel(std::vector<std::string>& passwords);
    void applyRulesToPasswords(std::vector<std::string>& passwords);
    void logMaskAttackDetails(const std::string& password);

    bool checkIfStop();

    void evaluateModel();
    void analyzeErrors();
    void manageResources();

    void connectToDatabase();
    void disconnectFromDatabase();
};

#endif // MASK_ATTACK_H




