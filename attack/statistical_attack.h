#ifndef STATISTICAL_ATTACK_H
#define STATISTICAL_ATTACK_H

#include "gpu_manager.h"
#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"  
#include "../utils/threading_utils.h"  
#include "../database/db_manager.h"  
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <queue>
#include <atomic>

class StatisticalAttack {
public:
    StatisticalAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                      Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager);

    void execute();
    void setStatisticalDataCallback(std::function<void(const std::string&, double)> callback);

private:
    GPUManager* gpuManager;
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<void(const std::string&, double)> statisticalDataCallback;

    std::mutex mtx;
    std::queue<std::pair<std::string, double>> statisticalDataQueue;
    std::atomic<bool> stopFlag = false;

    std::unordered_map<std::string, double> loadStatisticalData();
    void applyMachineLearningModel(std::unordered_map<std::string, double>& statisticalData);
    void applyRulesToStatisticalData(std::unordered_map<std::string, double>& statisticalData);
    void logAttackDetails(const std::string& password, double frequency);

    void statisticalDataWorker();  
    void evaluateModel();  
    void analyzeErrors();  
    void manageResources();  

    bool checkIfStop();
    void connectToDatabase();
    void disconnectFromDatabase();
};

#endif // STATISTICAL_ATTACK_H


