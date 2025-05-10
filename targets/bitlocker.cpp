#include "bitlocker.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <json/json.h>

BitLockerAttack::BitLockerAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                 Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), curl(nullptr) {}

void BitLockerAttack::loadConfiguration(const std::string& configPath) {
    logger->info("Loading configuration from " + configPath, {"BitLockerAttack", "Configuration"});

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        logger->error("Failed to open configuration file: " + configPath, {"BitLockerAttack", "Configuration"});
        return;
    }

    Json::Value config;
    configFile >> config;

    bitLockerUrl = config.get("bitLockerUrl", "http://127.0.0.1:8332").asString();
    rpcUser = config.get("rpcUser", "user").asString();
    rpcPassword = config.get("rpcPassword", "password").asString();

    logger->info("Configuration loaded successfully.", {"BitLockerAttack", "Configuration"});
}

void BitLockerAttack::connectToBitLocker() {
    logger->info("Connecting to BitLocker...", {"BitLockerAttack", "Connection"});
    curl = curl_easy_init();
    if (!curl) {
        logger->error("Failed to initialize CURL", {"BitLockerAttack", "Connection"});
        throw std::runtime_error("CURL initialization failed");
    }
}

void BitLockerAttack::disconnectFromBitLocker() {
    logger->info("Disconnecting from BitLocker...", {"BitLockerAttack", "Connection"});
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

bool BitLockerAttack::sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result) {
    if (!curl) {
        logger->error("No active CURL session", {"BitLockerAttack", "RPCRequest"});
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

    curl_easy_setopt(curl, CURLOPT_URL, bitLockerUrl.c_str());
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
        logger->error("CURL error: " + std::string(curl_easy_strerror(res)), {"BitLockerAttack", "RPCRequest"});
        return false;
    }

    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, responseString, &result, &errs)) {
        logger->error("Failed to parse JSON response: " + errs, {"BitLockerAttack", "RPCRequest"});
        return false;
    }

    return true;
}

bool BitLockerAttack::checkPasswordInBitLocker(const std::string& password) {
    logger->info("Checking password: " + password, {"BitLockerAttack", "PasswordCheck"});

    Json::Value params(Json::arrayValue);
    params.append(password);

    Json::Value result;
    if (!sendRpcRequest("validatepassword", params, result)) {
        logger->error("Failed to send RPC request", {"BitLockerAttack", "PasswordCheck"});
        return false;
    }

    if (result.isMember("result") && result["result"].asBool()) {
        logger->info("Password is valid: " + password, {"BitLockerAttack", "PasswordCheck"});
        return true;
    } else {
        logger->info("Password is invalid: " + password, {"BitLockerAttack", "PasswordCheck"});
        return false;
    }
}

void BitLockerAttack::setAttackCallback(std::function<std::string()> attackCallback) {
    this->attackCallback = attackCallback;
}

void BitLockerAttack::execute() {
    logger->info("Starting BitLocker attack.", {"BitLockerAttack", "Execution"});

    connectToBitLocker();

    while (!stopFlag) {
        if (attackCallback) {
            std::string password = attackCallback(); 
            if (password.empty()) {
                logger->info("No more passwords to check. Stopping attack.", {"BitLockerAttack", "Execution"});
                break;
            }

            if (checkPasswordInBitLocker(password)) {
                logger->info("Successful password found: " + password, {"BitLockerAttack", "Execution"});
                break;
            }
        } else {
            logger->error("Attack callback is not set!", {"BitLockerAttack", "Execution"});
            break;
        }
    }

    disconnectFromBitLocker();

    logger->info("BitLocker attack completed.", {"BitLockerAttack", "Execution"});
}
