#include "markov_attack.h"
#include <iostream>
#include <fstream>
#include <future>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem;

MarkovAttack::MarkovAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, Logger* logger, 
                           ThreadingUtils* threadingUtils, DBManager* dbManager, const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), logger(logger), 
      threadingUtils(threadingUtils), dbManager(dbManager), threadingStrategy(threadingStrategy), stopFlag(false) {
    logger->info("MarkovAttack initialized.", {"MarkovAttack", "Initialization"});
}

void MarkovAttack::setMarkovVerificationCallback(std::function<bool(const std::string&)> callback) {
    markovVerificationCallback = callback;
}

std::future<void> MarkovAttack::loadMarkovChainsAsync() {
    logger->info("Loading Markov chains asynchronously...", {"MarkovAttack", "ChainsLoading"});
    fs::path chainsPath = fs::current_path() / "data/markov_chains.txt";
    return std::async(std::launch::async, [this, chainsPath]() {
        auto markovChains = loadMarkovChains(chainsPath.string());
        if (!markovChains.empty()) {
            logger->info("Markov chains loaded successfully from " + chainsPath.string(), {"MarkovAttack", "ChainsLoading"});
        } else {
            logger->error("Failed to load Markov chains from " + chainsPath.string(), {"MarkovAttack", "ChainsLoading"});
            stopFlag = true;
        }
    });
}

std::unordered_map<std::string, std::unordered_map<char, double>> MarkovAttack::loadMarkovChains(const std::string& chainsPath) {
    std::unordered_map<std::string, std::unordered_map<char, double>> markovChains;
    std::ifstream file(chainsPath);
    std::string key;
    char value;
    double probability;
    while (file >> key >> value >> probability) {
        markovChains[key][value] = probability;
    }
    return markovChains;
}

void MarkovAttack::applyMachineLearningModel(std::unordered_map<std::string, std::unordered_map<char, double>>& markovChains) {
    logger->info("Applying machine learning model to Markov chains.", {"MarkovAttack", "ML"});
    
    for (auto& chain : markovChains) {
        arma::mat inputData(chain.second.size(), 1);
        size_t index = 0;
        for (const auto& nextChar : chain.second) {
            inputData(index++, 0) = nextChar.second;
        }

        arma::Row<size_t> predictions = mlPredictor->predict(inputData);
        for (auto& nextChar : chain.second) {
            nextChar.second *= predictions[0];
        }
    }
}

void MarkovAttack::applyRulesToGeneratedStrings(std::vector<std::string>& generatedStrings) {
    logger->info("Applying rules to generated strings.", {"MarkovAttack", "Rules"});
    
    std::vector<std::string> transformedStrings;
    for (const auto& str : generatedStrings) {
        auto transformed = ruleEngine->applyRules(str);
        transformedStrings.insert(transformedStrings.end(), transformed.begin(), transformed.end());
    }
    generatedStrings = std::move(transformedStrings);
}

void MarkovAttack::logMarkovAttackDetails(const std::string& generatedString) {
    logger->trace("Attempting Markov generated string: " + generatedString, {"MarkovAttack", "Details"});
}

void MarkovAttack::markovWorker() {
    while (!stopFlag) {
        std::string generatedString;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (markovQueue.empty()) {
                continue;
            }
            generatedString = markovQueue.front();
            markovQueue.pop();
        }

        logMarkovAttackDetails(generatedString);
        
        if (markovVerificationCallback && markovVerificationCallback(generatedString)) {
            stopFlag = true;
        }

        if (checkIfStop()) break;
    }
}

void MarkovAttack::execute() {
    logger->info("Starting Markov attack.", {"MarkovAttack", "Execution"});

    auto future = loadMarkovChainsAsync();
    future.wait();
    if (stopFlag) return;

    auto markovChains = loadMarkovChains("data/markov_chains.txt");

    std::vector<std::string> generatedStrings;
    for (const auto& chain : markovChains) {
        for (const auto& nextChar : chain.second) {
            std::string generatedString = chain.first + nextChar.first;
            generatedStrings.push_back(generatedString);
        }
    }

    applyMachineLearningModel(markovChains);
    applyRulesToGeneratedStrings(generatedStrings);

    for (const auto& str : generatedStrings) {
        std::unique_lock<std::mutex> lock(mtx);
        markovQueue.push(str);
    }

    threadingUtils->enableMonitoring();

    std::vector<std::function<void()>> tasks;
    tasks.push_back([this]() { markovWorker(); });

    threadingUtils->runInParallel(tasks, threadingStrategy);

    stopFlag = true;
    threadingUtils->stopThreads();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Markov Attack completed.", {"MarkovAttack", "Execution"});
}

bool MarkovAttack::checkIfStop() {
    return stopFlag;
}

void MarkovAttack::evaluateModel() {
    logger->info("Evaluating model after Markov attack.", {"MarkovAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"MarkovAttack", "Evaluation"});
}

void MarkovAttack::analyzeErrors() {
    logger->info("Analyzing errors after Markov attack.", {"MarkovAttack", "ErrorAnalysis"});
    std::string testDataPath = "data/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}

void MarkovAttack::manageResources() {
    logger->info("Managing resources after Markov attack.", {"MarkovAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}





