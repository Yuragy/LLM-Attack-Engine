#include "timing_attack.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <armadillo>
#include <tbb/tbb.h>
#include <future>
#include <filesystem>

namespace fs = std::filesystem;

TimingAttack::TimingAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, 
                           RuleEngine* ruleEngine, Logger* logger, 
                           ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager) {}

void TimingAttack::setAttemptSubmissionCallback(std::function<void(const std::string&, double)> callback) {
    attemptSubmissionCallback = callback;
}
std::vector<std::string> TimingAttack::generateAttempts() {
    logger->info("Generating attempts.", {"TimingAttack", "GenerateAttempts"});
    return {"attempt1", "attempt2", "attempt3"};
}
void TimingAttack::measureResponseTime(const std::string& attempt) {
    auto start = std::chrono::high_resolution_clock::now();
    gpuManager->executeAttack(attempt, "--gpu-loops=1024", "--gpu-accel=128");
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    logAttackDetails(attempt, duration.count());

    if (attemptSubmissionCallback) {
        attemptSubmissionCallback(attempt, duration.count());
    } else {
        logger->error("Attempt submission callback is not set!", {"TimingAttack", "Execution"});
    }
}
void TimingAttack::applyMachineLearningModel(std::vector<std::string>& attempts) {
    logger->info("Applying machine learning model to attempts.", {"TimingAttack", "MLModel"});

    arma::mat inputData(attempts.size(), 1);  
    arma::Row<size_t> predictions = mlPredictor->predict(inputData);

    for (size_t i = 0; i < attempts.size(); ++i) {
        attempts[i] += "_ml";
    }
}
void TimingAttack::applyRulesToAttempts(std::vector<std::string>& attempts) {
    logger->info("Applying rules to attempts.", {"TimingAttack", "Rules"});
    std::vector<std::string> transformedAttempts;

    for (const auto& attempt : attempts) {
        auto transformed = ruleEngine->applyRules(attempt);
        transformedAttempts.insert(transformedAttempts.end(), transformed.begin(), transformed.end());
    }

    attempts = std::move(transformedAttempts);
}
void TimingAttack::logAttackDetails(const std::string& attempt, double duration) {
    logger->trace("Attempt: " + attempt + " took " + std::to_string(duration) + " seconds", 
                  {"TimingAttack", "AttackDetails"});
}
void TimingAttack::attemptWorker() {
    std::pair<std::string, double> attemptData;

    while (!stopFlag) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (attemptQueue.empty()) continue;
            attemptData = attemptQueue.front();
            attemptQueue.pop();
        }

        measureResponseTime(attemptData.first);
    }
}
void TimingAttack::execute() {
    logger->info("Starting timing attack.", {"TimingAttack", "Execution"});

    auto attempts = generateAttempts();
    applyMachineLearningModel(attempts);
    applyRulesToAttempts(attempts);

    for (const auto& attempt : attempts) {
        std::unique_lock<std::mutex> lock(mtx);
        attemptQueue.push({attempt, 0.0});
    }

    auto future = threadingUtils->runInThread([this]() { attemptWorker(); });
    future.wait();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Timing Attack completed.", {"TimingAttack", "Execution"});
}
void TimingAttack::evaluateModel() {
    logger->info("Evaluating model after timing attack.", {"TimingAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"TimingAttack", "Evaluation"});
}
void TimingAttack::analyzeErrors() {
    logger->info("Analyzing errors after timing attack.", {"TimingAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}
void TimingAttack::manageResources() {
    logger->info("Managing resources after timing attack.", {"TimingAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}
bool TimingAttack::checkIfStop() {
    return stopFlag;
}
void TimingAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database.", {"TimingAttack", "DB"});
        stopFlag = true;
    }
}
void TimingAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database.", {"TimingAttack", "DB"});
}




