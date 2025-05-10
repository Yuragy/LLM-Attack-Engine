#ifndef HYBRID_ATTACK_H
#define HYBRID_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../dictionary/dictionary_loader.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "../database/db_manager.h"
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

class HybridAttack {
public:
    HybridAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                 DictionaryLoader* dictionaryLoader, Logger* logger,
                 ThreadingUtils* threadingUtils, DBManager* dbManager,
                 const std::string& threadingStrategy);

    void execute(); 
    void setHybridVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    DictionaryLoader* dictionaryLoader;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> hybridVerificationCallback;
    std::string threadingStrategy;
    std::mutex mtx;
    std::queue<std::string> hybridQueue;
    std::atomic<bool> stopFlag = false;
    void hybridWorker(const std::vector<std::string>& masks);
    std::future<void> loadDictionariesAsync();
    void applyMachineLearningModel(std::vector<std::string>& words);
    void applyRulesToWords(std::vector<std::string>& words);
    void loadWordsFromDictionaries();
    std::vector<std::string> loadMasksFromFile(); 
    void logHybridAttackDetails(const std::string& word, const std::string& mask);
    bool checkIfStop();
    void connectToDatabase();
    void disconnectFromDatabase();
    void evaluateModel();
    void analyzeErrors();
    void manageResources();
};

#endif // HYBRID_ATTACK_H




