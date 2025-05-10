#include "rule_based_attack.h"
#include <iostream>
#include <tbb/tbb.h>

RuleBasedAttack::RuleBasedAttack(RuleEngine* ruleEngine, MLPredictor* mlPredictor, 
                                 DictionaryLoader* dictionaryLoader, Logger* logger, 
                                 ThreadingUtils* threadingUtils)
    : ruleEngine(ruleEngine), mlPredictor(mlPredictor), 
      dictionaryLoader(dictionaryLoader), logger(logger), 
      threadingUtils(threadingUtils), passwordVerificationCallback(nullptr) {}

void RuleBasedAttack::setPasswordVerificationCallback(std::function<void(const std::string&)> callback) {
    passwordVerificationCallback = callback;
}

std::future<void> RuleBasedAttack::loadDictionariesAsync() {
    std::vector<std::string> dictionaryPaths = {"path/to/dictionary.txt"};
    return dictionaryLoader->loadMultipleAsync(dictionaryPaths).then([this](bool result) {
        if (result) {
            logger->info("Dictionaries loaded successfully.", {"RuleBased", "Dictionary"});
        } else {
            logger->error("Failed to load one or more dictionaries.", {"RuleBased", "Dictionary"});
        }
    });
}

std::vector<std::string> RuleBasedAttack::applyRules(const std::vector<std::string>& words) {
    std::vector<std::string> transformedWords;
    for (const auto& word : words) {
        auto transformations = ruleEngine->applyRules(word);
        transformedWords.insert(transformedWords.end(), transformations.begin(), transformations.end());
    }
    logger->info("Rules applied to dictionary.", {"RuleBased", "Rules"});
    return transformedWords;
}

void RuleBasedAttack::applyMachineLearningModel(std::vector<std::string>& words) {
    arma::mat inputData(words.size(), 1);
    for (size_t i = 0; i < words.size(); ++i) {
        inputData(i, 0) = words[i].length();
    }

    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < words.size(); ++i) {
        words[i] += "_" + std::to_string(predictions[i]);
    }
    logger->info("Machine learning model applied.", {"RuleBased", "ML"});
}

void RuleBasedAttack::logAttackDetails(const std::string& word) {
    logger->trace("Attempting word: " + word, {"RuleBased", "AttackDetails"});
}

void RuleBasedAttack::execute() {
    logger->info("Starting rule-based attack.", {"RuleBased", "Execution"});

    auto future = loadDictionariesAsync();
    future.wait();

    auto words = dictionaryLoader->getLoadedWords();
    if (words.empty()) {
        logger->error("No words loaded from dictionary.", {"RuleBased", "Execution"});
        return;
    }

    auto transformedWords = applyRules(words);
    applyMachineLearningModel(transformedWords);

    tbb::parallel_for_each(transformedWords.begin(), transformedWords.end(), [&](const std::string& word) {
        logAttackDetails(word);
        if (passwordVerificationCallback) {
            passwordVerificationCallback(word);
        } else {
            logger->error("Password verification callback is not set!", {"RuleBased", "Execution"});
        }
    });

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Rule-based attack completed.", {"RuleBased", "Execution"});
}

void RuleBasedAttack::evaluateModel() {
    logger->info("Evaluating model after rule-based attack.", {"RuleBased", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"RuleBased", "Evaluation"});
}

void RuleBasedAttack::analyzeErrors() {
    logger->info("Analyzing errors after rule-based attack.", {"RuleBased", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}

void RuleBasedAttack::manageResources() {
    logger->info("Managing resources after rule-based attack.", {"RuleBased", "ResourceManagement"});
    mlPredictor->manageResources();
}






