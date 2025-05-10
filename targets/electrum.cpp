#include "electrum.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <json/json.h>

ElectrumAttack::ElectrumAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                               Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), curl(nullptr) {}

void ElectrumAttack::loadConfiguration(const std::string& configPath) {
    logger->info("Loading configuration from " + configPath, {"ElectrumAttack", "Configuration"});

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        logger->error("Failed to open configuration file: " + configPath, {"ElectrumAttack", "Configuration"});
        return;
    }

    Json::Value config;
    configFile >> config;

    electrumNodeUrl = config.get("electrumNodeUrl", "http://127.0.0.1:8332").asString();
    rpcUser = config.get("rpcUser", "user").asString();
    rpcPassword = config.get("rpcPassword", "password").asString();

    logger->info("Configuration loaded successfully.", {"ElectrumAttack", "Configuration"});
}

void ElectrumAttack::connectToElectrumNode() {
    logger->info("Connecting to Electrum node...", {"ElectrumAttack", "Connection"});
    curl = curl_easy_init();
    if (!curl) {
        logger->error("Failed to initialize CURL", {"ElectrumAttack", "Connection"});
        throw std::runtime_error("CURL initialization failed");
    }
}

void ElectrumAttack::disconnectFromElectrumNode() {
    logger->info("Disconnecting from Electrum node...", {"ElectrumAttack", "Connection"});
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

bool ElectrumAttack::sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result) {
    if (!curl) {
        logger->error("No active CURL session", {"ElectrumAttack", "RPCRequest"});
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

    curl_easy_setopt(curl, CURLOPT_URL, electrumNodeUrl.c_str());
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
        logger->error("CURL error: " + std::string(curl_easy_strerror(res)), {"ElectrumAttack", "RPCRequest"});
        return false;
    }

    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, responseString, &result, &errs)) {
        logger->error("Failed to parse JSON response: " + errs, {"ElectrumAttack", "RPCRequest"});
        return false;
    }

    return true;
}

bool ElectrumAttack::checkSeedInElectrumNode(const std::string& seed) {
    logger->info("Checking seed: " + seed, {"ElectrumAttack", "SeedCheck"});

    Json::Value params(Json::arrayValue);
    params.append(seed);

    Json::Value result;
    if (!sendRpcRequest("validateseed", params, result)) {
        logger->error("Failed to send RPC request", {"ElectrumAttack", "SeedCheck"});
        return false;
    }

    if (result.isMember("result") && result["result"].asBool()) {
        logger->info("Seed is valid: " + seed, {"ElectrumAttack", "SeedCheck"});
        return true;
    } else {
        logger->info("Seed is invalid: " + seed, {"ElectrumAttack", "SeedCheck"});
        return false;
    }
}

void ElectrumAttack::setAttackCallback(std::function<std::string()> attackCallback) {
    this->attackCallback = attackCallback;
}

std::vector<std::string> ElectrumAttack::generateSeeds(size_t length, const std::string& charset) {
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

void ElectrumAttack::applyMachineLearningModel(std::vector<std::string>& seeds) {
    arma::mat inputData;
    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < seeds.size(); ++i) {
        seeds[i] += "_" + std::to_string(predictions[i]);
    }
}

void ElectrumAttack::applyRulesToSeeds(std::vector<std::string>& seeds) {
    std::vector<std::string> transformedSeeds;
    for (const auto& seed : seeds) {
        auto transformed = ruleEngine->applyRules(seed);
        transformedSeeds.insert(transformedSeeds.end(), transformed.begin(), transformed.end());
    }
    seeds = std::move(transformedSeeds);
}

void ElectrumAttack::execute() {
    logger->info("Starting Electrum attack.", {"ElectrumAttack", "Execution"});

    connectToElectrumNode();

    while (!stopFlag) {
        if (attackCallback) {
            std::string seed = attackCallback();  
            if (seed.empty()) {
                logger->info("No more seeds to check. Stopping attack.", {"ElectrumAttack", "Execution"});
                break;
            }

            if (checkSeedInElectrumNode(seed)) {
                logger->info("Successful seed found: " + seed, {"ElectrumAttack", "Execution"});
                break;
            }
        } else {
            logger->error("Attack callback is not set!", {"ElectrumAttack", "Execution"});
            break;
        }
    }

    disconnectFromElectrumNode();

    logger->info("Electrum attack completed.", {"ElectrumAttack", "Execution"});
}
