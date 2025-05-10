#ifndef MARKOV_ATTACK_H
#define MARKOV_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "../database/db_manager.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <future>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

class MarkovAttack {
public:
    MarkovAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, Logger* logger, 
                 ThreadingUtils* threadingUtils, DBManager* dbManager, const std::string& threadingStrategy);

    void execute();
    void setMarkovVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> markovVerificationCallback;

    std::string threadingStrategy;
    std::mutex mtx;
    std::queue<std::string> markovQueue;
    std::atomic<bool> stopFlag = false;

    void markovWorker();
    std::future<void> loadMarkovChainsAsync();
    std::unordered_map<std::string, std::unordered_map<char, double>> loadMarkovChains(const std::string& chainsPath);
    void applyMachineLearningModel(std::unordered_map<std::string, std::unordered_map<char, double>>& markovChains);
    void applyRulesToGeneratedStrings(std::vector<std::string>& generatedStrings);
    void logMarkovAttackDetails(const std::string& generatedString);

    void evaluateModel();
    void analyzeErrors();
    void manageResources();
    bool checkIfStop();
};

#endif // MARKOV_ATTACK_H




