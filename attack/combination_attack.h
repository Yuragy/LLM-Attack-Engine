#ifndef COMBINATION_ATTACK_H
#define COMBINATION_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../dictionary/dictionary_loader.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "../database/db_manager.h"
#include <vector>
#include <string>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <atomic>

class CombinationAttack {
public:
    CombinationAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                      DictionaryLoader* dictionaryLoader, Logger* logger,
                      ThreadingUtils* threadingUtils, DBManager* dbManager, 
                      const std::string& threadingStrategy);

    void execute();
    void setCombinationVerificationCallback(std::function<bool(const std::string&, const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    DictionaryLoader* dictionaryLoader;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&, const std::string&)> combinationVerificationCallback;

    std::string threadingStrategy;

    std::mutex mtx;
    std::queue<std::pair<std::string, std::string>> combinationQueue;
    std::atomic<bool> stopFlag = false;

    void combinationWorker();
    void generateCombinations();
    void applyMachineLearningModel(std::vector<std::string>& words1, std::vector<std::string>& words2);
    void applyRulesToWords(std::vector<std::string>& words);
    std::future<void> loadDictionariesAsync();
    void loadWordsFromDictionaries();
    void logCombinationAttackDetails(const std::string& word1, const std::string& word2);

    void evaluateModel();
    void analyzeErrors();
    void manageResources();
    
    bool checkIfStop();
};

#endif // COMBINATION_ATTACK_H




