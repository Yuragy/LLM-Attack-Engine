#ifndef RAINBOW_TABLE_ATTACK_H
#define RAINBOW_TABLE_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "gpu_manager.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <future>
#include <functional>

class RainbowTableAttack {
public:
    RainbowTableAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                       Logger* logger, ThreadingUtils* threadingUtils);

    void execute();
    void setHashVerificationCallback(std::function<void(const std::string&, const std::string&)> callback);

private:
    GPUManager* gpuManager;
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;

    std::function<void(const std::string&, const std::string&)> hashVerificationCallback; 

    std::unordered_map<std::string, std::string> loadRainbowTables();
    void applyMachineLearningModel(std::unordered_map<std::string, std::string>& rainbowTable);
    void applyRulesToRainbowTable(std::unordered_map<std::string, std::string>& rainbowTable);
    void logAttackDetails(const std::string& hash, const std::string& password);
    void evaluateModel();
    void analyzeErrors();
    void manageResources();
};

#endif // RAINBOW_TABLE_ATTACK_H


