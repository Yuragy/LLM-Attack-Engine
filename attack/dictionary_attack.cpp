#include "dictionary_attack.h"
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem;

DictionaryAttack::DictionaryAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                   DictionaryLoader* dictionaryLoader, Logger* logger, 
                                   ThreadingUtils* threadingUtils, DBManager* dbManager, 
                                   const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), dictionaryLoader(dictionaryLoader), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), 
      threadingStrategy(threadingStrategy), stopFlag(false) {
    logger->info("DictionaryAttack initialized.", {"DictionaryAttack", "Initialization"});
}

void DictionaryAttack::setDictionaryVerificationCallback(std::function<bool(const std::string&)> callback) {
    dictionaryVerificationCallback = callback;
}

std::future<void> DictionaryAttack::loadDictionariesAsync() {
    logger->info("Loading dictionaries asynchronously...", {"DictionaryAttack", "Dictionaries"});
    fs::path currentPath = fs::current_path();
    std::vector<std::string> dictionaryPaths = {
        (currentPath / "dictionaries/dictionary1.txt").string()
    };

    return dictionaryLoader->loadMultipleAsync(dictionaryPaths).then([this](bool result) {
        if (!result) {
            logger->error("Failed to load one or more dictionaries.", {"DictionaryAttack", "Dictionary"});
            stopFlag = true;
        } else {
            logger->info("Dictionaries loaded successfully.", {"DictionaryAttack", "Dictionary"});
        }
    });
}

void DictionaryAttack::loadWordsFromDictionaries() {
    logger->info("Loading words from dictionaries...", {"DictionaryAttack", "Dictionaries"});
    std::vector<std::string> words = dictionaryLoader->getAllWords();
    applyRulesToWords(words);

    for (const auto& word : words) {
        std::unique_lock<std::mutex> lock(mtx);
        wordQueue.push(word);
        lock.unlock();
    }
    logger->info("Words loaded from dictionaries and queued.", {"DictionaryAttack", "Dictionaries"});
}

void DictionaryAttack::applyMachineLearningModel(std::vector<std::string>& words) {
    logger->info("Applying machine learning model to words.", {"DictionaryAttack", "MLModel"});
    
    arma::mat inputData(words.size(), 1);
    for (size_t i = 0; i < words.size(); ++i) {
        inputData(i, 0) = words[i].length();
    }

    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < words.size(); ++i) {
        words[i] += "_" + std::to_string(predictions[i]);
    }

    logger->info("Words processed by machine learning model.", {"DictionaryAttack", "MLModel"});
}

void DictionaryAttack::applyRulesToWords(std::vector<std::string>& words) {
    logger->info("Applying rules to words...", {"DictionaryAttack", "Rules"});

    std::vector<std::string> transformedWords;
    for (const auto& word : words) {
        auto transformed = ruleEngine->applyRules(word);
        transformedWords.insert(transformedWords.end(), transformed.begin(), transformed.end());
    }
    words = std::move(transformedWords);
    logger->info("Transformation rules applied to words.", {"DictionaryAttack", "Rules"});
}

void DictionaryAttack::dictionaryWorker() {
    while (!stopFlag) {
        std::string word;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (wordQueue.empty()) {
                continue;
            }
            word = wordQueue.front();
            wordQueue.pop();
        }

        logDictionaryAttackDetails(word);

        if (dictionaryVerificationCallback && dictionaryVerificationCallback(word)) {
            stopFlag = true;
        }

        if (checkIfStop()) break;
    }
}

void DictionaryAttack::execute() {
    logger->info("Starting dictionary attack.", {"DictionaryAttack", "Execution"});

    auto future = loadDictionariesAsync();
    future.wait();
    if (stopFlag) return;

    loadWordsFromDictionaries();

    threadingUtils->enableMonitoring();

    std::vector<std::function<void()>> tasks;
    tasks.push_back([this]() { dictionaryWorker(); });

    threadingUtils->runInParallel(tasks, threadingStrategy);

    stopFlag = true;

    threadingUtils->stopThreads();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Dictionary Attack completed.", {"DictionaryAttack", "Execution"});
}

void DictionaryAttack::logDictionaryAttackDetails(const std::string& word) {
    logger->trace("Attempting word: " + word, {"DictionaryAttack", "AttackDetails"});
}

void DictionaryAttack::evaluateModel() {
    logger->info("Evaluating model after dictionary attack.", {"DictionaryAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"DictionaryAttack", "Evaluation"});
}

void DictionaryAttack::analyzeErrors() {
    logger->info("Analyzing errors after dictionary attack.", {"DictionaryAttack", "ErrorAnalysis"});
    std::string testDataPath = (fs::current_path() / "data/test_data.txt").string();
    mlPredictor->analyzeErrors(testDataPath);
}

void DictionaryAttack::manageResources() {
    logger->info("Managing resources after dictionary attack.", {"DictionaryAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool DictionaryAttack::checkIfStop() {
    return stopFlag;
}





