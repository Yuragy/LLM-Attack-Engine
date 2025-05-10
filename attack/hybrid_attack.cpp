#include "hybrid_attack.h"
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <armadillo>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

HybridAttack::HybridAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                           DictionaryLoader* dictionaryLoader, Logger* logger,
                           ThreadingUtils* threadingUtils, DBManager* dbManager,
                           const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), dictionaryLoader(dictionaryLoader), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), 
      threadingStrategy(threadingStrategy), stopFlag(false) {
    logger->info("HybridAttack initialized.", {"HybridAttack", "Initialization"});
}

void HybridAttack::setHybridVerificationCallback(std::function<bool(const std::string&)> callback) {
    hybridVerificationCallback = callback;
}

std::future<void> HybridAttack::loadDictionariesAsync() {
    logger->info("Loading dictionaries asynchronously...", {"HybridAttack", "Dictionaries"});
    fs::path currentPath = fs::current_path();
    std::vector<std::string> dictionaryPaths = {
        (currentPath / "dictionaries/dictionary1.txt").string()
    };

    return dictionaryLoader->loadMultipleAsync(dictionaryPaths).then([this](bool result) {
        if (!result) {
            logger->error("Failed to load one or more dictionaries.", {"HybridAttack", "Dictionary"});
            stopFlag = true;
        } else {
            logger->info("Dictionaries loaded successfully.", {"HybridAttack", "Dictionary"});
        }
    });
}

void HybridAttack::loadWordsFromDictionaries() {
    logger->info("Loading words from dictionaries...", {"HybridAttack", "Dictionaries"});
    std::vector<std::string> words = dictionaryLoader->getAllWords();
    applyRulesToWords(words);

    for (const auto& word : words) {
        std::unique_lock<std::mutex> lock(mtx);
        hybridQueue.push(word);
        lock.unlock();
    }
    logger->info("Words loaded from dictionaries and queued.", {"HybridAttack", "Dictionaries"});
}

std::vector<std::string> HybridAttack::loadMasksFromFile() {
    logger->info("Loading masks from file...", {"HybridAttack", "Masks"});
    fs::path maskFilePath = fs::current_path() / "masks/mask.txt";
    std::ifstream maskFile(maskFilePath);
    std::vector<std::string> masks;

    if (!maskFile.is_open()) {
        logger->error("Failed to open mask file: " + maskFilePath.string(), {"HybridAttack", "MaskLoading"});
        stopFlag = true;
        return masks;
    }

    std::string mask;
    while (std::getline(maskFile, mask)) {
        if (!mask.empty()) {
            masks.push_back(mask); 
        }
    }

    if (masks.empty()) {
        logger->error("No masks found in file: " + maskFilePath.string(), {"HybridAttack", "MaskLoading"});
        stopFlag = true;
    } else {
        logger->info("Masks loaded successfully.", {"HybridAttack", "MaskLoading"});
    }

    return masks;
}

void HybridAttack::applyMachineLearningModel(std::vector<std::string>& words) {
    logger->info("Applying machine learning model to words.", {"HybridAttack", "MLModel"});

    arma::mat inputData(words.size(), 1);
    for (size_t i = 0; i < words.size(); ++i) {
        inputData(i, 0) = words[i].length();
    }

    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < words.size(); ++i) {
        words[i] += "_" + std::to_string(predictions[i]);
    }

    logger->info("Words processed by machine learning model.", {"HybridAttack", "MLModel"});
}

void HybridAttack::applyRulesToWords(std::vector<std::string>& words) {
    logger->info("Applying rules to words...", {"HybridAttack", "Rules"});

    std::vector<std::string> transformedWords;
    for (const auto& word : words) {
        auto transformed = ruleEngine->applyRules(word);
        transformedWords.insert(transformedWords.end(), transformed.begin(), transformed.end());
    }
    words = std::move(transformedWords);
    logger->info("Transformation rules applied to words.", {"HybridAttack", "Rules"});
}

void HybridAttack::hybridWorker(const std::vector<std::string>& masks) {
    while (!stopFlag) {
        std::string word;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (hybridQueue.empty()) {
                continue;
            }
            word = hybridQueue.front();
            hybridQueue.pop();
        }

        for (const auto& mask : masks) {
            logHybridAttackDetails(word, mask);
            std::string transformedWord = word + mask;

            if (hybridVerificationCallback && hybridVerificationCallback(transformedWord)) {
                stopFlag = true;
                break;
            }
        }

        if (checkIfStop()) break;
    }
}

void HybridAttack::execute() {
    logger->info("Starting hybrid attack.", {"HybridAttack", "Execution"});

    auto masks = loadMasksFromFile();
    if (stopFlag || masks.empty()) return;

    auto future = loadDictionariesAsync();
    future.wait();
    if (stopFlag) return;

    loadWordsFromDictionaries();

    threadingUtils->enableMonitoring();

    std::vector<std::function<void()>> tasks;
    tasks.push_back([this, &masks]() { hybridWorker(masks); });

    threadingUtils->runInParallel(tasks, threadingStrategy);

    stopFlag = true;

    threadingUtils->stopThreads();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Hybrid Attack completed.", {"HybridAttack", "Execution"});
}

void HybridAttack::logHybridAttackDetails(const std::string& word, const std::string& mask) {
    logger->trace("Attempting hybrid attack with word: " + word + " and mask: " + mask, {"HybridAttack", "AttackDetails"});
}

void HybridAttack::evaluateModel() {
    logger->info("Evaluating model after hybrid attack.", {"HybridAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"HybridAttack", "Evaluation"});
}

void HybridAttack::analyzeErrors() {
    logger->info("Analyzing errors after hybrid attack.", {"HybridAttack", "ErrorAnalysis"});
    std::string testDataPath = (fs::current_path() / "data/test_data.txt").string();
    mlPredictor->analyzeErrors(testDataPath);
}

void HybridAttack::manageResources() {
    logger->info("Managing resources after hybrid attack.", {"HybridAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

void HybridAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database", {"HybridAttack", "DB"});
        stopFlag = true;
    }
}

void HybridAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database", {"HybridAttack", "DB"});
}







