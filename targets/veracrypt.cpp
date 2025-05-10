#include "veracrypt.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <json/json.h>

VeraCryptAttack::VeraCryptAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                 Logger* logger, ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), curl(nullptr) {}

void VeraCryptAttack::loadConfiguration(const std::string& configPath) {
    logger->info("Loading configuration from " + configPath, {"VeraCryptAttack", "Configuration"});

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        logger->error("Failed to open configuration file: " + configPath, {"VeraCryptAttack", "Configuration"});
        return;
    }

    Json::Value config;
    configFile >> config;

    veraCryptContainerPath = config.get("veraCryptContainerPath", "/path/to/container").asString();
    veraCryptPassword = config.get("veraCryptPassword", "defaultpassword").asString();

    logger->info("Configuration loaded successfully.", {"VeraCryptAttack", "Configuration"});
}

void VeraCryptAttack::connectToVeraCryptContainer() {
    logger->info("Connecting to VeraCrypt container...", {"VeraCryptAttack", "Connection"});
    curl = curl_easy_init();
    if (!curl) {
        logger->error("Failed to initialize CURL", {"VeraCryptAttack", "Connection"});
        throw std::runtime_error("CURL initialization failed");
    }
}

void VeraCryptAttack::disconnectFromVeraCryptContainer() {
    logger->info("Disconnecting from VeraCrypt container...", {"VeraCryptAttack", "Connection"});
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

bool VeraCryptAttack::sendRpcRequest(const std::string& method, const Json::Value& params, Json::Value& result) {
    if (!curl) {
        logger->error("No active CURL session", {"VeraCryptAttack", "RPCRequest"});
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

    curl_easy_setopt(curl, CURLOPT_URL, veraCryptContainerPath.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, (veraCryptPassword + ":" + veraCryptPassword).c_str());
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
        logger->error("CURL error: " + std::string(curl_easy_strerror(res)), {"VeraCryptAttack", "RPCRequest"});
        return false;
    }

    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, responseString, &result, &errs)) {
        logger->error("Failed to parse JSON response: " + errs, {"VeraCryptAttack", "RPCRequest"});
        return false;
    }

    return true;
}

bool VeraCryptAttack::checkPasswordInVeraCryptContainer(const std::string& password) {
    logger->info("Checking password: " + password, {"VeraCryptAttack", "PasswordCheck"});

    Json::Value params(Json::arrayValue);
    params.append(password);

    Json::Value result;
    if (!sendRpcRequest("validatepassword", params, result)) {
        logger->error("Failed to send RPC request", {"VeraCryptAttack", "PasswordCheck"});
        return false;
    }

    if (result.isMember("result") && result["result"].asBool()) {
        logger->info("Password is valid: " + password, {"VeraCryptAttack", "PasswordCheck"});
        return true;
    } else {
        logger->info("Password is invalid: " + password, {"VeraCryptAttack", "PasswordCheck"});
        return false;
    }
}

void VeraCryptAttack::setAttackCallback(std::function<std::string()> attackCallback) {
    this->attackCallback = attackCallback;
}

std::vector<std::string> VeraCryptAttack::generatePasswords(size_t length, const std::string& charset) {
    std::vector<std::string> passwords;
    std::string password(length, charset[0]);
    while (true) {
        passwords.push_back(password);
        size_t pos = 0;
        while (pos < length && ++password[pos] == charset.size()) {
            password[pos] = charset[0];
            ++pos;
        }
        if (pos == length) break;
    }
    return passwords;
}

void VeraCryptAttack::applyMachineLearningModel(std::vector<std::string>& passwords) {
    arma::mat inputData;
    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < passwords.size(); ++i) {
        passwords[i] += "_" + std::to_string(predictions[i]);
    }
}

void VeraCryptAttack::applyRulesToPasswords(std::vector<std::string>& passwords) {
    std::vector<std::string> transformedPasswords;
    for (const auto& password : passwords) {
        auto transformed = ruleEngine->applyRules(password);
        transformedPasswords.insert(transformedPasswords.end(), transformed.begin(), transformed.end());
    }
    passwords = std::move(transformedPasswords);
}

void VeraCryptAttack::execute() {
    logger->info("Starting VeraCrypt attack.", {"VeraCryptAttack", "Execution"});

    connectToVeraCryptContainer();

    std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t max_length = 16;

    for (size_t length = 1; length <= max_length; ++length) {
        auto passwords = generatePasswords(length, characters);
        applyMachineLearningModel(passwords);
        applyRulesToPasswords(passwords);

        tbb::parallel_for_each(passwords.begin(), passwords.end(), [&](const std::string& password) {
            gpuManager->executeTask([password]() {
                executeAttack(password, "--gpu-loops=1024", "--gpu-accel=128");
            });
        });

        if (stopFlag) {
            break;
        }
    }

    disconnectFromVeraCryptContainer();

    logger->info("VeraCrypt attack completed.", {"VeraCryptAttack", "Execution"});
}
