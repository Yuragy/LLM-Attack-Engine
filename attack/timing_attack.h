#ifndef TIMING_ATTACK_H
#define TIMING_ATTACK_H

#include "gpu_manager.h"
#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"  
#include "../utils/threading_utils.h" 
#include "../database/db_manager.h" 
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <queue>
#include <atomic>

class TimingAttack {
public:
    TimingAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                 Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager);

    void execute();
    void setAttemptSubmissionCallback(std::function<void(const std::string&, double)> callback);
private:
    GPUManager* gpuManager;
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;
    std::function<void(const std::string&, double)> attemptSubmissionCallback;
    std::mutex mtx;
    std::queue<std::pair<std::string, double>> attemptQueue;
    std::atomic<bool> stopFlag = false;
    std::vector<std::string> generateAttempts();
    void measureResponseTime(const std::string& attempt);
    void applyMachineLearningModel(std::vector<std::string>& attempts);
    void applyRulesToAttempts(std::vector<std::string>& attempts);
    void logAttackDetails(const std::string& attempt, double duration);
    void attemptWorker();  
    void evaluateModel(); 
    void analyzeErrors(); 
    void manageResources(); 
    bool checkIfStop();
    void connectToDatabase();
    void disconnectFromDatabase();
};

#endif // TIMING_ATTACK_H


