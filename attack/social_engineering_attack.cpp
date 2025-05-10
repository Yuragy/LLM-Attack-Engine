#include "social_engineering_attack.h"
#include <iostream>
#include <vector>
#include <string>
#include <armadillo>
#include <tbb/tbb.h>
#include <future>
#include <filesystem>

namespace fs = std::filesystem;

SocialEngineeringAttack::SocialEngineeringAttack(GPUManager* gpuManager, MLPredictor* mlPredictor, 
                                                 RuleEngine* ruleEngine, Logger* logger, 
                                                 ThreadingUtils* threadingUtils, DBManager* dbManager)
    : gpuManager(gpuManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager) {}

void SocialEngineeringAttack::setPersonalInfoSubmissionCallback(std::function<void(const std::string&)> callback) {
    personalInfoSubmissionCallback = callback;
}

std::vector<std::string> SocialEngineeringAttack::collectPersonalInfo() {
    logger->info("Collecting personal information.", {"SocialEngineeringAttack", "CollectInfo"});
    return {"birthday", "name", "favorite_color"};
}

void SocialEngineeringAttack::applyMachineLearningModel(std::vector<std::string>& personalInfo) {
    logger->info("Applying machine learning model to personal information.", {"SocialEngineeringAttack", "MLModel"});
    
    arma::mat inputData(personalInfo.size(), 1);
    arma::Row<size_t> predictions = mlPredictor->predict(inputData);

    for (size_t i = 0; i < personalInfo.size(); ++i) {
        personalInfo[i] += "_ml";  
    }
}

void SocialEngineeringAttack::applyRulesToPersonalInfo(std::vector<std::string>& personalInfo) {
    logger->info("Applying rules to personal information.", {"SocialEngineeringAttack", "Rules"});
    std::vector<std::string> transformedInfo;

    for (const auto& info : personalInfo) {
        auto transformed = ruleEngine->applyRules(info);
        transformedInfo.insert(transformedInfo.end(), transformed.begin(), transformed.end());
    }

    personalInfo = std::move(transformedInfo);
}

void SocialEngineeringAttack::logAttackDetails(const std::string& personalInfo) {
    logger->trace("Attempting social engineering attack on: " + personalInfo, {"SocialEngineeringAttack", "AttackDetails"});
}

void SocialEngineeringAttack::personalInfoWorker() {
    std::vector<std::string> personalInfo;

    while (!stopFlag) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (personalInfoQueue.empty()) continue;
            personalInfo.push_back(personalInfoQueue.front());
            personalInfoQueue.pop();
        }

        applyMachineLearningModel(personalInfo);
        applyRulesToPersonalInfo(personalInfo);

        for (const auto& info : personalInfo) {
            logAttackDetails(info);
            if (personalInfoSubmissionCallback) {
                personalInfoSubmissionCallback(info);
            } else {
                logger->error("Personal info submission callback is not set!", {"SocialEngineeringAttack", "Execution"});
            }
        }
        personalInfo.clear();
    }
}

void SocialEngineeringAttack::execute() {
    logger->info("Starting social engineering attack.", {"SocialEngineeringAttack", "Execution"});

    auto personalInfo = collectPersonalInfo();

    for (const auto& info : personalInfo) {
        std::unique_lock<std::mutex> lock(mtx);
        personalInfoQueue.push(info);
    }

    auto future = threadingUtils->runInThread([this]() { personalInfoWorker(); });
    future.wait();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Social Engineering Attack completed.", {"SocialEngineeringAttack", "Execution"});
}

void SocialEngineeringAttack::evaluateModel() {
    logger->info("Evaluating model after social engineering attack.", {"SocialEngineeringAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"SocialEngineeringAttack", "Evaluation"});
}

void SocialEngineeringAttack::analyzeErrors() {
    logger->info("Analyzing errors after social engineering attack.", {"SocialEngineeringAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}

void SocialEngineeringAttack::manageResources() {
    logger->info("Managing resources after social engineering attack.", {"SocialEngineeringAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool SocialEngineeringAttack::checkIfStop() {
    return stopFlag;
}

void SocialEngineeringAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database", {"SocialEngineeringAttack", "DB"});
        stopFlag = true;
    }
}

void SocialEngineeringAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database", {"SocialEngineeringAttack", "DB"});
}




