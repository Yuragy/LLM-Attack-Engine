#ifndef REVERSE_ATTACK_H
#define REVERSE_ATTACK_H

#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include <string>
#include <vector>
#include <future>
#include <functional>
#include <tbb/tbb.h> 
#include <fstream>   
#include <bcrypt.h>  

class ReverseAttack {
public:
    ReverseAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                  Logger* logger, ThreadingUtils* threadingUtils);

    void execute();
    void setHashComparisonCallback(std::function<void(const std::string&, const std::string&)> callback);

    void setHashType(std::string hashType);

private:
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;

    std::string hashType; 
    std::function<void(const std::string&, const std::string&)> hashComparisonCallback; 
    std::string generateCandidate(size_t index, const std::string& charset);
    void applyMachineLearningModel(std::string& candidate);
    void applyRulesToCandidate(std::string& candidate);
    void logAttackDetails(const std::string& candidate, const std::string& hash);
    void generateAndProcessCandidates(const std::string& knownHash, size_t totalCandidates);
    std::string loadKnownHashFromFile(const std::string& filePath);
    bool isValidHash(const std::string& hash);
    std::string bcryptHash(const std::string& candidate);
    void evaluateModel();
    void analyzeErrors();
    void manageResources(); 
};

#endif // REVERSE_ATTACK_H




