#ifndef VERACRYPT_H
#define VERACRYPT_H

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
#include <atomic>
#include <json/json.h>

class VeraCryptAttack {
public:
    VeraCryptAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                    Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager);

    void loadConfiguration(const std::string& configPath);
    void execute();

    void setAttackCallback(std::function<std::string()> attackCallback);

private:
    GPUManager* gpuManager;
    MLPredictor* mlPredictor;
    RuleEngine* ruleEngine;
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<std::string()> attackCallback;
    std::mutex mtx;
    std::atomic<bool> stopFlag = false;

    std::string veraCryptContainerPath;
    std::string veraCryptPassword;

    CURL* curl;

    void connectToVeraCryptContainer();
    void disconnectFromVeraCryptContainer();

    bool checkPasswordInVeraCryptContainer(const std::string& password);

    bool sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result);

    std::vector<std::string> generatePasswords(size_t length, const std::string& charset);
    void applyMachineLearningModel(std::vector<std::string>& passwords);
    void applyRulesToPasswords(std::vector<std::string>& passwords);
};

#endif // VERACRYPT_H
