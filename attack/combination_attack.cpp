#include "combination_attack.h"
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem; 

CombinationAttack::CombinationAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                     DictionaryLoader* dictionaryLoader, Logger* logger,
                                     ThreadingUtils* threadingUtils, DBManager* dbManager, 
                                     const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      dictionaryLoader(dictionaryLoader), logger(logger), 
      threadingUtils(threadingUtils), dbManager(dbManager),
      threadingStrategy(threadingStrategy), combinationVerificationCallback(nullptr), stopFlag(false) {
    logger->info("CombinationAttack initialized.", {"CombinationAttack", "Initialization"});
}

void CombinationAttack::setCombinationVerificationCallback(std::function<bool(const std::string&, const std::string&)> callback) {
    combinationVerificationCallback = callback;
}

std::future<void> CombinationAttack::loadDictionariesAsync() {
    logger->info("Loading dictionaries asynchronously...", {"CombinationAttack", "Dictionaries"});

    fs::path currentPath = fs::current_path();
    std::vector<std::string> dictionaryPaths = {
        (currentPath / "dictionaries/dictionary1.txt").string(),
        (currentPath / "dictionaries/dictionary2.txt").string()
    };

    return dictionaryLoader->loadMultipleAsync(dictionaryPaths).then([this](bool result) {
        if (!result) {
            logger->error("Failed to load one or more dictionaries.", {"CombinationAttack", "Dictionary"});
            stopFlag = true;
        } else {
            logger->info("Dictionaries loaded successfully.", {"CombinationAttack", "Dictionary"});
        }
    });
}

void CombinationAttack::loadWordsFromDictionaries() {
    logger->info("Loading words from dictionaries...", {"CombinationAttack", "Dictionaries"});
    std::vector<std::string> words1 = dictionaryLoader->getAllWords();
    std::vector<std::string> words2 = dictionaryLoader->getAllWords();
    
    applyRulesToWords(words1);
    applyRulesToWords(words2);

    for (const auto& word1 : words1) {
        for (const auto& word2 : words2) {
            std::unique_lock<std::mutex> lock(mtx);
            combinationQueue.push({word1, word2});
            lock.unlock();
        }
    }
    logger->info("Words loaded and queued for combination.", {"CombinationAttack", "Dictionaries"});
}

void CombinationAttack::applyMachineLearningModel(std::vector<std::string>& words1, std::vector<std::string>& words2) {
    logger->info("Applying machine learning model to word combinations.", {"CombinationAttack", "MLModel"});

    arma::mat inputData1(words1.size(), 1, arma::fill::zeros);
    for (size_t i = 0; i < words1.size(); ++i) {
        inputData1(i, 0) = static_cast<double>(words1[i].length());
    }

    arma::Row<size_t> predictions1 = mlPredictor->predict(inputData1);
    for (size_t i = 0; i < words1.size(); ++i) {
        words1[i] += "_" + std::to_string(predictions1[i]);
    }

    arma::mat inputData2(words2.size(), 1, arma::fill::zeros);
    for (size_t i = 0; i < words2.size(); ++i) {
        inputData2(i, 0) = static_cast<double>(words2[i].length());
    }

    arma::Row<size_t> predictions2 = mlPredictor->predict(inputData2);
    for (size_t i = 0; i < words2.size(); ++i) {
        words2[i] += "_" + std::to_string(predictions2[i]);
    }
    logger->info("Machine learning model applied to word combinations.", {"CombinationAttack", "ML"});
}

void CombinationAttack::applyRulesToWords(std::vector<std::string>& words) {
    logger->info("Applying rules to words...", {"CombinationAttack", "Rules"});

    std::vector<std::string> transformedWords;
    for (const auto& word : words) {
        auto transformed = ruleEngine->applyRules(word);
        transformedWords.insert(transformedWords.end(), transformed.begin(), transformed.end());
    }
    words = std::move(transformedWords);
    logger->info("Rules applied to words.", {"CombinationAttack", "Rules"});
}

void CombinationAttack::combinationWorker() {
    while (!stopFlag) {
        std::pair<std::string, std::string> combination;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (combinationQueue.empty()) {
                continue;
            }
            combination = combinationQueue.front();
            combinationQueue.pop();
        }

        logCombinationAttackDetails(combination.first, combination.second);

        if (combinationVerificationCallback && combinationVerificationCallback(combination.first, combination.second)) {
            stopFlag = true;
        }

        if (checkIfStop()) break;
    }
}

void CombinationAttack::generateCombinations() {
    logger->info("Generating word combinations...", {"CombinationAttack", "Generation"});

    // logic
}

bool CombinationAttack::checkIfStop() {
    return stopFlag;
}

void CombinationAttack::execute() {
    logger->info("Starting combination attack.", {"CombinationAttack", "Execution"});

    auto future = loadDictionariesAsync();
    future.wait();
    if (stopFlag) return;

    loadWordsFromDictionaries();
    
    threadingUtils->enableMonitoring();

    std::vector<std::function<void()>> tasks;
    tasks.push_back([this]() {
        generateCombinations();
    });

    threadingUtils->runInParallel(tasks, threadingStrategy);
    stopFlag = true;

    threadingUtils->stopThreads();
    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Combination Attack completed.", {"CombinationAttack", "Execution"});
}

void CombinationAttack::logCombinationAttackDetails(const std::string& word1, const std::string& word2) {
    logger->trace("Attempting combination: " + word1 + " + " + word2, {"CombinationAttack", "CombinationDetails"});
}

void CombinationAttack::evaluateModel() {
    logger->info("Evaluating model after combination attack.", {"CombinationAttack", "Evaluation"});
    arma::mat inputData; 
    arma::Row<size_t> trueLabels; 

    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"CombinationAttack", "Evaluation"});
}

void CombinationAttack::analyzeErrors() {
    logger->info("Analyzing errors after combination attack.", {"CombinationAttack", "ErrorAnalysis"});

    fs::path currentPath = fs::current_path();
    std::string testDataPath = (currentPath / "data/test_data.txt").string();
    mlPredictor->analyzeErrors(testDataPath);
}

void CombinationAttack::manageResources() {
    logger->info("Managing resources after combination attack.", {"CombinationAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}






