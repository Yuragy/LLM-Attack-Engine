#ifndef PERMUTED_DICTIONARY_ATTACK_H
#define PERMUTED_DICTIONARY_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include <vector>
#include <string>
#include <functional>
#include <future>

class PermutedDictionaryAttack {
public:
    PermutedDictionaryAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, Logger* logger, ThreadingUtils* threadingUtils);

    void execute();

    void setWordVerificationCallback(std::function<void(const std::string&)> callback);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    
    std::function<void(const std::string&)> wordVerificationCallback; 
    std::future<void> loadDictionaryAsync();
    std::vector<std::string> loadDictionary(const std::string& dictionaryPath);
    void applyMachineLearningModel(std::vector<std::string>& words);
    void applyRulesToWords(std::vector<std::string>& words);
    void logPermutedAttackDetails(const std::string& word);
    void evaluateModel();
    void analyzeErrors();
    void manageResources();
};

#endif // PERMUTED_DICTIONARY_ATTACK_H

