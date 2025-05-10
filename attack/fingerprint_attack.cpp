#include "fingerprint_attack.h"
#include <iostream>
#include <fstream>
#include <future>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem;

FingerprintAttack::FingerprintAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                     Logger* logger, ThreadingUtils* threadingUtils, 
                                     DBManager* dbManager, const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), logger(logger), 
      threadingUtils(threadingUtils), dbManager(dbManager), 
      threadingStrategy(threadingStrategy), stopFlag(false) {
    logger->info("FingerprintAttack initialized.", {"FingerprintAttack", "Initialization"});
}

void FingerprintAttack::setFingerprintVerificationCallback(std::function<bool(const std::string&)> callback) {
    fingerprintVerificationCallback = callback;
}

std::future<void> FingerprintAttack::loadFingerprintDataAsync() {
    logger->info("Loading fingerprint data asynchronously...", {"FingerprintAttack", "DataLoading"});
    fs::path currentPath = fs::current_path();
    std::string dataPath = (currentPath / "data/fingerprint_data.txt").string();

    return std::async(std::launch::async, [this, dataPath]() {
        std::ifstream file(dataPath);
        std::string user, data;
        std::unordered_map<std::string, std::string> fingerprintData;

        while (file >> user >> data) {
            fingerprintData[user] = data;
        }

        if (!fingerprintData.empty()) {
            logger->info("Fingerprint data loaded successfully.", {"FingerprintAttack", "DataLoading"});
        } else {
            logger->error("Failed to load fingerprint data.", {"FingerprintAttack", "DataLoading"});
            stopFlag = true;
        }

        applyMachineLearningModel(fingerprintData);
        applyRulesToFingerprintData(fingerprintData);

        for (const auto& pair : fingerprintData) {
            std::unique_lock<std::mutex> lock(mtx);
            fingerprintQueue.push(pair.second);
        }
    });
}

void FingerprintAttack::loadFingerprintData() {
    logger->info("Loading fingerprint data...", {"FingerprintAttack", "DataLoading"});
    fs::path currentPath = fs::current_path();
    std::string dataPath = (currentPath / "data/fingerprint_data.txt").string();

    std::unordered_map<std::string, std::string> fingerprintData;
    std::ifstream file(dataPath);
    std::string user, data;

    while (file >> user >> data) {
        fingerprintData[user] = data;
    }

    applyMachineLearningModel(fingerprintData);
    applyRulesToFingerprintData(fingerprintData);

    for (const auto& pair : fingerprintData) {
        std::unique_lock<std::mutex> lock(mtx);
        fingerprintQueue.push(pair.second);
    }

    logger->info("Fingerprint data loaded and queued.", {"FingerprintAttack", "DataLoading"});
}

void FingerprintAttack::applyMachineLearningModel(std::unordered_map<std::string, std::string>& fingerprintData) {
    logger->info("Applying machine learning model to fingerprint data.", {"FingerprintAttack", "MLModel"});

    for (auto& pair : fingerprintData) {
        arma::mat inputData(pair.second.length(), 1);
        for (size_t i = 0; i < pair.second.length(); ++i) {
            inputData(i, 0) = pair.second[i];
        }

        arma::Row<size_t> predictions = mlPredictor->predict(inputData);
        pair.second += "_" + std::to_string(predictions[0]);
    }

    logger->info("Machine learning model applied.", {"FingerprintAttack", "MLModel"});
}

void FingerprintAttack::applyRulesToFingerprintData(std::unordered_map<std::string, std::string>& fingerprintData) {
    logger->info("Applying rules to fingerprint data...", {"FingerprintAttack", "Rules"});

    for (auto& pair : fingerprintData) {
        auto transformed = ruleEngine->applyRules(pair.second);
        pair.second = transformed.empty() ? pair.second : transformed[0];
    }

    logger->info("Rules applied to fingerprint data.", {"FingerprintAttack", "Rules"});
}

void FingerprintAttack::fingerprintWorker() {
    while (!stopFlag) {
        std::string fingerprintData;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (fingerprintQueue.empty()) {
                continue;
            }
            fingerprintData = fingerprintQueue.front();
            fingerprintQueue.pop();
        }

        logFingerprintAttackDetails(fingerprintData);

        if (fingerprintVerificationCallback && fingerprintVerificationCallback(fingerprintData)) {
            stopFlag = true;
        }

        if (checkIfStop()) break;
    }
}

void FingerprintAttack::execute() {
    logger->info("Starting fingerprint attack.", {"FingerprintAttack", "Execution"});

    auto future = loadFingerprintDataAsync();
    future.wait();
    if (stopFlag) return;

    threadingUtils->enableMonitoring();

    std::vector<std::function<void()>> tasks;
    tasks.push_back([this]() { fingerprintWorker(); });

    threadingUtils->runInParallel(tasks, threadingStrategy);

    stopFlag = true;
    threadingUtils->stopThreads();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Fingerprint Attack completed.", {"FingerprintAttack", "Execution"});
}

void FingerprintAttack::logFingerprintAttackDetails(const std::string& data) {
    logger->trace("Attempting fingerprint data: " + data, {"FingerprintAttack", "AttackDetails"});
}

void FingerprintAttack::evaluateModel() {
    logger->info("Evaluating model after fingerprint attack.", {"FingerprintAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"FingerprintAttack", "Evaluation"});
}

void FingerprintAttack::analyzeErrors() {
    logger->info("Analyzing errors after fingerprint attack.", {"FingerprintAttack", "ErrorAnalysis"});
    std::string testDataPath = (fs::current_path() / "data/test_data.txt").string();
    mlPredictor->analyzeErrors(testDataPath);
}

void FingerprintAttack::manageResources() {
    logger->info("Managing resources after fingerprint attack.", {"FingerprintAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool FingerprintAttack::checkIfStop() {
    return stopFlag;
}





