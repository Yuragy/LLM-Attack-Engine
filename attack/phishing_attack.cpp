#include "phishing_attack.h"
#include <iostream>
#include <fstream>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem;

PhishingAttack::PhishingAttack(DictionaryLoader* dictionaryLoader, Logger* logger, 
                               MLPredictor* mlPredictor, ThreadingUtils* threadingUtils, 
                               DBManager* dbManager)
    : dictionaryLoader(dictionaryLoader), logger(logger), 
      mlPredictor(mlPredictor), threadingUtils(threadingUtils), 
      dbManager(dbManager), pageSubmissionCallback(nullptr) {}

void PhishingAttack::setPageSubmissionCallback(std::function<void(const std::string&)> callback) {
    pageSubmissionCallback = callback;
}

std::vector<std::string> PhishingAttack::loadPhishingPages() {
    logger->info("Loading phishing pages from file...", {"PhishingAttack", "Pages"});
    std::vector<std::string> pages = dictionaryLoader->load("path/to/phishing_pages.txt");

    if (pages.empty()) {
        logger->error("No phishing pages found.", {"PhishingAttack", "Pages"});
        stopFlag = true;
    } else {
        logger->info("Phishing pages loaded successfully.", {"PhishingAttack", "Pages"});
    }

    return pages;
}

void PhishingAttack::applyMachineLearningModel(std::vector<std::string>& pages) {
    logger->info("Applying machine learning model to phishing pages.", {"PhishingAttack", "MLModel"});
    arma::mat inputData(pages.size(), 1);
    for (size_t i = 0; i < pages.size(); ++i) {
        inputData(i, 0) = pages[i].length();
    }

    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < pages.size(); ++i) {
        pages[i] += "_" + std::to_string(predictions[i]);
    }
}

void PhishingAttack::logAttackDetails(const std::string& pageContent) {
    logger->trace("Attempting phishing page submission: " + pageContent, {"PhishingAttack", "AttackDetails"});
}

void PhishingAttack::phishingWorker() {
    auto pages = loadPhishingPages();
    if (stopFlag) return;

    applyMachineLearningModel(pages);

    for (const auto& page : pages) {
        logAttackDetails(page);
        if (pageSubmissionCallback) {
            pageSubmissionCallback(page);
        } else {
            logger->error("Page submission callback is not set!", {"PhishingAttack", "Execution"});
        }
    }
}

void PhishingAttack::execute() {
    logger->info("Starting phishing attack.", {"PhishingAttack", "Execution"});

    auto future = threadingUtils->runInThread([this]() { phishingWorker(); });
    future.wait();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Phishing Attack completed.", {"PhishingAttack", "Execution"});
}

void PhishingAttack::evaluateModel() {
    logger->info("Evaluating model after phishing attack.", {"PhishingAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"PhishingAttack", "Evaluation"});
}

void PhishingAttack::analyzeErrors() {
    logger->info("Analyzing errors after phishing attack.", {"PhishingAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}

void PhishingAttack::manageResources() {
    logger->info("Managing resources after phishing attack.", {"PhishingAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool PhishingAttack::checkIfStop() {
    return stopFlag;
}

void PhishingAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database", {"PhishingAttack", "DB"});
        stopFlag = true;
    }
}

void PhishingAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database", {"PhishingAttack", "DB"});
}





