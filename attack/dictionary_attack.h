#ifndef DICTIONARY_ATTACK_H
#define DICTIONARY_ATTACK_H

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
#include <thread>
#include <mutex>
#include <atomic>

class DictionaryAttack {
public:
    DictionaryAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                     DictionaryLoader* dictionaryLoader, Logger* logger, 
                     ThreadingUtils* threadingUtils, DBManager* dbManager, 
                     const std::string& threadingStrategy);

    void execute();
    void setDictionaryVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    DictionaryLoader* dictionaryLoader;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> dictionaryVerificationCallback;
    std::string threadingStrategy;

    std::mutex mtx;
    std::queue<std::string> wordQueue;
    std::atomic<bool> stopFlag = false;

    void dictionaryWorker();
    std::future<void> loadDictionariesAsync();
    void applyMachineLearningModel(std::vector<std::string>& words);
    void applyRulesToWords(std::vector<std::string>& words);
    void logDictionaryAttackDetails(const std::string& word);

    void evaluateModel();
    void analyzeErrors();
    void manageResources();

    bool checkIfStop();
    void loadWordsFromDictionaries();
};

#endif // DICTIONARY_ATTACK_H


