#ifndef PATTERN_BASED_ATTACK_H
#define PATTERN_BASED_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../dictionary/dictionary_loader.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "../database/db_manager.h"
#include <string>
#include <vector>
#include <future>
#include <functional>
#include <queue>
#include <mutex>
#include <atomic>

class PatternBasedAttack {
public:
    PatternBasedAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                      DictionaryLoader* dictionaryLoader, Logger* logger,
                      ThreadingUtils* threadingUtils, DBManager* dbManager, 
                      const std::string& threadingStrategy);

    void execute();
    void setPatternVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    DictionaryLoader* dictionaryLoader;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> patternVerificationCallback;

    std::string threadingStrategy;
    std::mutex mtx;
    std::queue<std::string> patternQueue;
    std::atomic<bool> stopFlag = false;

    // Основные методы
    void patternWorker();
    std::vector<std::string> generatePatterns(size_t length);
    void applyMachineLearningModel(std::vector<std::string>& patterns);
    void applyRulesToPatterns(std::vector<std::string>& patterns);
    std::future<void> loadDictionariesAsync();
    void loadPatternsFromDictionaries();
    void loadPatternsFromDatabase();
    void logAttackDetails(const std::string& pattern);

    void evaluateModel();
    void analyzeErrors();
    void manageResources();

    bool checkIfStop();

    void connectToDatabase();
    void disconnectFromDatabase();
};

#endif // PATTERN_BASED_ATTACK_H





