#include "rainbow_table_attack.h"
#include <fstream>
#include <iostream>
#include <future>

RainbowTableAttack::RainbowTableAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                       Logger* logger, ThreadingUtils* threadingUtils)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), hashVerificationCallback(nullptr) {}

void RainbowTableAttack::setHashVerificationCallback(std::function<void(const std::string&, const std::string&)> callback) {
    hashVerificationCallback = callback;
}

std::unordered_map<std::string, std::string> RainbowTableAttack::loadRainbowTables() {
    std::unordered_map<std::string, std::string> rainbowTable;
    std::ifstream file("path/to/rainbow_table.txt");
    std::string hash, password;
    while (file >> hash >> password) {
        rainbowTable[hash] = password;
    }
    logger->info("Loaded rainbow tables from file.", {"RainbowTableAttack", "DictionaryLoad"});
    return rainbowTable;
}

void RainbowTableAttack::applyMachineLearningModel(std::unordered_map<std::string, std::string>& rainbowTable) {
    arma::mat inputData(rainbowTable.size(), 1);  
    size_t i = 0;
    for (const auto& pair : rainbowTable) {
        inputData(i, 0) = pair.second.length();  
        ++i;
    }

    arma::Row<size_t> predictions = mlPredictor->predict(inputData);  
    i = 0;
    for (auto& pair : rainbowTable) {
        pair.second += "_ml" + std::to_string(predictions[i]); 
        ++i;
    }

    logger->info("Machine learning model applied to rainbow table.", {"RainbowTableAttack", "ML"});
}

void RainbowTableAttack::applyRulesToRainbowTable(std::unordered_map<std::string, std::string>& rainbowTable) {
    for (auto& pair : rainbowTable) {
        auto transformed = ruleEngine->applyRules(pair.second);
        if (!transformed.empty()) {
            pair.second = transformed[0]; 
        }
    }
    logger->info("Transformation rules applied to rainbow table.", {"RainbowTableAttack", "Rules"});
}

void RainbowTableAttack::logAttackDetails(const std::string& hash, const std::string& password) {
    logger->trace("Attempting hash: " + hash + " with password: " + password, {"RainbowTableAttack", "AttackDetails"});
}
void RainbowTableAttack::execute() {
    logger->info("Starting rainbow table attack.", {"RainbowTableAttack", "Execution"});

    auto rainbowTable = loadRainbowTables();

    applyMachineLearningModel(rainbowTable);
    applyRulesToRainbowTable(rainbowTable);

    for (const auto& pair : rainbowTable) {
        logAttackDetails(pair.first, pair.second);
        if (hashVerificationCallback) {
            hashVerificationCallback(pair.first, pair.second);  
        } else {
            logger->error("Hash verification callback is not set!", {"RainbowTableAttack", "Execution"});
        }
    }

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Rainbow table attack completed.", {"RainbowTableAttack", "Execution"});
}

void RainbowTableAttack::evaluateModel() {
    logger->info("Evaluating model after rainbow table attack.", {"RainbowTableAttack", "Evaluation"});
    arma::mat inputData;  
    arma::Row<size_t> trueLabels;  
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"RainbowTableAttack", "Evaluation"});
}

void RainbowTableAttack::analyzeErrors() {
    logger->info("Analyzing errors after rainbow table attack.", {"RainbowTableAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt"; 
    mlPredictor->analyzeErrors(testDataPath);
}

void RainbowTableAttack::manageResources() {
    logger->info("Managing resources after rainbow table attack.", {"RainbowTableAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}




