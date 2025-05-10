#ifndef PHANTOM_WALLET_H
#define PHANTOM_WALLET_H

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

class PhantomWalletAttack {
public:
    PhantomWalletAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
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

    std::string phantomWalletNodeUrl;
    std::string rpcUser;
    std::string rpcPassword;

    CURL* curl;

    void connectToPhantomWalletNode();
    void disconnectFromPhantomWalletNode();

    bool checkSeedInPhantomWalletNode(const std::string& seed);

    bool sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result);

    std::vector<std::string> generateSeeds(size_t length, const std::string& charset);
    void applyMachineLearningModel(std::vector<std::string>& seeds);
    void applyRulesToSeeds(std::vector<std::string>& seeds);
};

#endif // PHANTOM_WALLET_H
