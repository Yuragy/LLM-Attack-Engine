#include "permuted_dictionary_attack.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <tbb/tbb.h>
#include "../machine_learning/ml_predictor.h"
#include "../rules/rule_engine.h"
#include "../logging/logger.h"
#include "../utils/threading_utils.h"

PermutedDictionaryAttack::PermutedDictionaryAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, Logger* logger, ThreadingUtils* threadingUtils)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), logger(logger), threadingUtils(threadingUtils), wordVerificationCallback(nullptr) {}

void PermutedDictionaryAttack::setWordVerificationCallback(std::function<void(const std::string&)> callback) {
    wordVerificationCallback = callback;
}
std::future<void> PermutedDictionaryAttack::loadDictionaryAsync() {
    std::string dictionaryPath = "path/to/dictionary.txt";
    return std::async(std::launch::async, [this, dictionaryPath]() {
        auto words = loadDictionary(dictionaryPath);
        if (!words.empty()) {
            logger->info("Dictionary loaded successfully from " + dictionaryPath, {"PermutedDictionaryAttack", "DictionaryLoading"});
        } else {
            logger->error("Failed to load dictionary from " + dictionaryPath, {"PermutedDictionaryAttack", "DictionaryLoading"});
        }
    });
}
std::vector<std::string> PermutedDictionaryAttack::loadDictionary(const std::string& dictionaryPath) {
    std::vector<std::string> words;
    std::ifstream file(dictionaryPath);
    std::string word;
    while (std::getline(file, word)) {
        words.push_back(word);
    }
    return words;
}
void PermutedDictionaryAttack::applyMachineLearningModel(std::vector<std::string>& words) {
    arma::mat inputData(words.size(), 1);
    for (size_t i = 0; i < words.size(); ++i) {
        inputData(i, 0) = words[i].length(); 
    }
    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < words.size(); ++i) {
        words[i] += "_" + std::to_string(predictions[i]); 
    }
    logger->info("Machine learning model applied to words.", {"PermutedDictionaryAttack", "ML"});
}
void PermutedDictionaryAttack::applyRulesToWords(std::vector<std::string>& words) {
    std::vector<std::string> transformedWords;
    for (const auto& word : words) {
        auto transformed = ruleEngine->applyRules(word);
        transformedWords.insert(transformedWords.end(), transformed.begin(), transformed.end());
    }
    words = std::move(transformedWords);
    logger->info("Transformation rules applied to words.", {"PermutedDictionaryAttack", "Rules"});
}
void PermutedDictionaryAttack::logPermutedAttackDetails(const std::string& word) {
    logger->trace("Attempting permuted dictionary attack with word: " + word, {"PermutedDictionaryAttack", "AttackDetails"});
}
void PermutedDictionaryAttack::execute() {
    logger->info("Starting permuted dictionary attack.", {"PermutedDictionaryAttack", "Execution"});

    auto future = loadDictionaryAsync();
    future.wait(); 

    auto words = loadDictionary("path/to/dictionary.txt");

    applyMachineLearningModel(words);
    applyRulesToWords(words);

    tbb::parallel_for_each(words.begin(), words.end(), [&](const std::string& word) {
        std::string permutedWord = word;
        std::sort(permutedWord.begin(), permutedWord.end());
        do {
            logPermutedAttackDetails(permutedWord);
            if (wordVerificationCallback) {
                wordVerificationCallback(permutedWord);
            } else {
                logger->error("Word verification callback is not set!", {"PermutedDictionaryAttack", "Execution"});
            }
            gpuManager->executeTask([permutedWord, this]() {
                std::string hashed_permutedWord = hashFunction(permutedWord);
                log("Permuted dictionary attack on word: " + permutedWord + " hash: " + hashed_permutedWord);
                gpuManager->executeAttack(permutedWord, "--gpu-loops=1024", "--gpu-accel=128");
            });
        } while (std::next_permutation(permutedWord.begin(), permutedWord.end()));
    });

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Permuted dictionary attack completed.", {"PermutedDictionaryAttack", "Execution"});
}
void PermutedDictionaryAttack::evaluateModel() {
    logger->info("Evaluating model after permuted dictionary attack.", {"PermutedDictionaryAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"PermutedDictionaryAttack", "Evaluation"});
}
void PermutedDictionaryAttack::analyzeErrors() {
    logger->info("Analyzing errors after permuted dictionary attack.", {"PermutedDictionaryAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}
void PermutedDictionaryAttack::manageResources() {
    logger->info("Managing resources after permuted dictionary attack.", {"PermutedDictionaryAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}




