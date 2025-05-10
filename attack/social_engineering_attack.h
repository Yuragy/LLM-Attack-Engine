#ifndef SOCIAL_ENGINEERING_ATTACK_H
#define SOCIAL_ENGINEERING_ATTACK_H

#include "gpu_manager.h"
#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"  
#include "../utils/threading_utils.h"  
#include "../database/db_manager.h"  
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>
#include <atomic>

class SocialEngineeringAttack {
public:
    SocialEngineeringAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                            Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager);

    void execute();
    void setPersonalInfoSubmissionCallback(std::function<void(const std::string&)> callback);

private:
    GPUManager* gpuManager;
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<void(const std::string&)> personalInfoSubmissionCallback;

    std::mutex mtx;
    std::queue<std::string> personalInfoQueue;
    std::atomic<bool> stopFlag = false;

    std::vector<std::string> collectPersonalInfo();
    void applyMachineLearningModel(std::vector<std::string>& personalInfo);
    void applyRulesToPersonalInfo(std::vector<std::string>& personalInfo);
    void logAttackDetails(const std::string& personalInfo);

    void personalInfoWorker();  
    void evaluateModel();  
    void analyzeErrors(); 
    void manageResources();  

    bool checkIfStop();
    void connectToDatabase();
    void disconnectFromDatabase();
};

#endif // SOCIAL_ENGINEERING_ATTACK_H


