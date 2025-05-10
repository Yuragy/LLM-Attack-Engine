#ifndef BRUTE_FORCE_ATTACK_H
#define BRUTE_FORCE_ATTACK_H

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

class BruteForceAttack {
public:
    BruteForceAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                     Logger* logger, ThreadingUtils* threadingUtils, 
                     DBManager* dbManager, const std::string& charset, 
                     size_t maxPasswordLength, const std::string& threadingStrategy);

    void execute();
    void setPasswordVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> passwordVerificationCallback;

    std::string charset;
    size_t maxPasswordLength;
    std::string threadingStrategy;
    
    std::mutex mtx;
    std::queue<std::string> passwordQueue;
    std::atomic<bool> stopFlag = false;

    void passwordWorker();
    void generatePasswords(size_t length);
    void applyMachineLearningModel(std::vector<std::string>& passwords);
    void applyRulesToPasswords(std::vector<std::string>& passwords);
    void logBruteForceAttackDetails(const std::string& password);
    
    std::future<void> loadDictionariesAsync();
    void loadPasswordsFromDictionaries();
    void loadPasswordsFromDatabase();

    bool checkIfStop();

    void evaluateModel();
    void analyzeErrors();
    void manageResources();

    void connectToDatabase();
    void disconnectFromDatabase();
};

#endif // BRUTE_FORCE_ATTACK_H








