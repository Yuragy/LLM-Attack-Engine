#ifndef RULE_BASED_ATTACK_H
#define RULE_BASED_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../dictionary/dictionary_loader.h"
#include "../utils/threading_utils.h"
#include <vector>
#include <string>
#include <future>
#include <functional>

class RuleBasedAttack {
public:
    RuleBasedAttack(RuleEngine* ruleEngine, MLPredictor* mlPredictor, 
                    DictionaryLoader* dictionaryLoader, Logger* logger, 
                    ThreadingUtils* threadingUtils);

    void execute();
    void setPasswordVerificationCallback(std::function<void(const std::string&)> callback);

private:
    RuleEngine* ruleEngine;
    MLPredictor* mlPredictor;
    DictionaryLoader* dictionaryLoader;
    Logger* logger;
    ThreadingUtils* threadingUtils;

    std::function<void(const std::string&)> passwordVerificationCallback;

    std::future<void> loadDictionariesAsync();
    std::vector<std::string> applyRules(const std::vector<std::string>& words);
    void applyMachineLearningModel(std::vector<std::string>& words);
    void logAttackDetails(const std::string& password);
    void evaluateModel();
    void analyzeErrors();
    void manageResources();
};

#endif // RULE_BASED_ATTACK_H


