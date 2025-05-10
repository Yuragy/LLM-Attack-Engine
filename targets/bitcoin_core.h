#ifndef BITCOIN_CORE_H
#define BITCOIN_CORE_H

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

class BitcoinCoreAttack {
public:
    BitcoinCoreAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
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
    std::string bitcoinCoreUrl;
    std::string rpcUser;
    std::string rpcPassword;

    CURL* curl;
    void connectToBitcoinCore();
    void disconnectFromBitcoinCore();
    bool checkPasswordInBitcoinCore(const std::string& password);
    bool sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result);
};

#endif // BITCOIN_CORE_H
