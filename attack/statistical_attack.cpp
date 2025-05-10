#include "statistical_attack.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <armadillo>
#include <tbb/tbb.h>
#include <future>
#include <filesystem>

namespace fs = std::filesystem;

StatisticalAttack::StatisticalAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, 
                                     RuleEngine* ruleEngine, Logger* logger, 
                                     ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager) {}

void StatisticalAttack::setStatisticalDataCallback(std::function<void(const std::string&, double)> callback) {
    statisticalDataCallback = callback;
}

std::unordered_map<std::string, double> StatisticalAttack::loadStatisticalData() {
    logger->info("Loading statistical data from file.", {"StatisticalAttack", "DataLoading"});
    std::unordered_map<std::string, double> statisticalData;
    std::ifstream file("path/to/statistical_data.txt");

    if (!file.is_open()) {
        logger->error("Failed to open statistical data file.", {"StatisticalAttack", "DataLoading"});
        stopFlag = true;
        return statisticalData;
    }

    std::string password;
    double frequency;
    while (file >> password >> frequency) {
        statisticalData[password] = frequency;
    }

    if (statisticalData.empty()) {
        logger->error("No statistical data found.", {"StatisticalAttack", "DataLoading"});
        stopFlag = true;
    } else {
        logger->info("Statistical data loaded successfully.", {"StatisticalAttack", "DataLoading"});
    }

    return statisticalData;
}

void StatisticalAttack::applyMachineLearningModel(std::unordered_map<std::string, double>& statisticalData) {
    logger->info("Applying machine learning model to statistical data.", {"StatisticalAttack", "MLModel"});
    
    arma::mat inputData(statisticalData.size(), 1);  
    arma::Row<size_t> predictions = mlPredictor->predict(inputData);

    for (auto& pair : statisticalData) {
        pair.second *= 1.1; 
    }
}

void StatisticalAttack::applyRulesToStatisticalData(std::unordered_map<std::string, double>& statisticalData) {
    logger->info("Applying rules to statistical data.", {"StatisticalAttack", "Rules"});
    std::unordered_map<std::string, double> transformedData;

    for (const auto& pair : statisticalData) {
        auto transformed = ruleEngine->applyRules(pair.first);
        for (const auto& new_password : transformed) {
            transformedData[new_password] = pair.second;
        }
    }

    statisticalData = std::move(transformedData);
}

void StatisticalAttack::logAttackDetails(const std::string& password, double frequency) {
    logger->trace("Attempting attack with password: " + password + " frequency: " + std::to_string(frequency), 
                  {"StatisticalAttack", "AttackDetails"});
}

void StatisticalAttack::statisticalDataWorker() {
    while (!stopFlag) {
        std::pair<std::string, double> data;

        {
            std::unique_lock<std::mutex> lock(mtx);
            if (statisticalDataQueue.empty()) continue;
            data = statisticalDataQueue.front();
            statisticalDataQueue.pop();
        }

        logAttackDetails(data.first, data.second);

        if (statisticalDataCallback) {
            statisticalDataCallback(data.first, data.second);
        } else {
            logger->error("Statistical data callback is not set!", {"StatisticalAttack", "Execution"});
        }
    }
}

void StatisticalAttack::execute() {
    logger->info("Starting statistical attack.", {"StatisticalAttack", "Execution"});

    auto statisticalData = loadStatisticalData();
    applyMachineLearningModel(statisticalData);
    applyRulesToStatisticalData(statisticalData);

    for (const auto& pair : statisticalData) {
        std::unique_lock<std::mutex> lock(mtx);
        statisticalDataQueue.push(pair);
    }

    // Многопоточная обработка данных
    auto future = threadingUtils->runInThread([this]() { statisticalDataWorker(); });
    future.wait();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Statistical Attack completed.", {"StatisticalAttack", "Execution"});
}

void StatisticalAttack::evaluateModel() {
    logger->info("Evaluating model after statistical attack.", {"StatisticalAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"StatisticalAttack", "Evaluation"});
}

void StatisticalAttack::analyzeErrors() {
    logger->info("Analyzing errors after statistical attack.", {"StatisticalAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}

void StatisticalAttack::manageResources() {
    logger->info("Managing resources after statistical attack.", {"StatisticalAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool StatisticalAttack::checkIfStop() {
    return stopFlag;
}

void StatisticalAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database.", {"StatisticalAttack", "DB"});
        stopFlag = true;
    }
}

void StatisticalAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database.", {"StatisticalAttack", "DB"});
}




