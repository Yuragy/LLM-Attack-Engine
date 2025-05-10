#include "brute_force_attack.h"
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <fstream>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem;

BruteForceAttack::BruteForceAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                   Logger* logger, ThreadingUtils* threadingUtils, 
                                   DBManager* dbManager, const std::string& charset, 
                                   size_t maxPasswordLength, const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), 
      dbManager(dbManager), charset(charset), 
      maxPasswordLength(maxPasswordLength), threadingStrategy(threadingStrategy), stopFlag(false) {
    logger->info("BruteForceAttack initialized.", {"BruteForceAttack", "Initialization"});
}

void BruteForceAttack::setPasswordVerificationCallback(std::function<bool(const std::string&)> callback) {
    passwordVerificationCallback = callback;
}

void BruteForceAttack::generatePasswords(size_t length) {
    logger->info("Generating passwords...", {"BruteForceAttack", "PasswordGeneration"});
    std::string password(length, charset[0]);
    while (!stopFlag) {
        std::unique_lock<std::mutex> lock(mtx);
        passwordQueue.push(password);
        lock.unlock();

        size_t pos = 0;
        while (pos < length && ++password[pos] == charset.size()) {
            password[pos] = charset[0];
            ++pos;
        }
        if (pos == length) break;

        if (checkIfStop()) break;
    }
}

std::future<void> BruteForceAttack::loadDictionariesAsync() {
    logger->info("Loading dictionaries asynchronously...", {"BruteForceAttack", "Dictionaries"});

    fs::path currentPath = fs::current_path();
    std::vector<std::string> dictionaryPaths = {
        (currentPath / "dictionaries/dictionary1.txt").string(),
        (currentPath / "dictionaries/dictionary2.txt").string()
    };

    return dictionaryLoader->loadMultipleAsync(dictionaryPaths).then([this](bool result) {
        if (!result) {
            logger->error("Failed to load one or more dictionaries.", {"BruteForceAttack", "DictionaryLoading"});
            stopFlag = true;
        } else {
            logger->info("Dictionaries loaded successfully.", {"BruteForceAttack", "DictionaryLoading"});
        }
    });
}

void BruteForceAttack::loadPasswordsFromDictionaries() {
    logger->info("Loading passwords from dictionaries...", {"BruteForceAttack", "Dictionaries"});
    std::vector<std::string> loadedPasswords = dictionaryLoader->getAllWords();
    applyRulesToPasswords(loadedPasswords);

    for (const auto& password : loadedPasswords) {
        std::unique_lock<std::mutex> lock(mtx);
        passwordQueue.push(password);
    }
    logger->info("Passwords loaded from dictionaries and queued.", {"BruteForceAttack", "Dictionaries"});
}

void BruteForceAttack::applyMachineLearningModel(std::vector<std::string>& passwords) {
    logger->info("Applying machine learning model to passwords.", {"BruteForceAttack", "MLModel"});
    arma::mat inputData(passwords.size(), 1);
    for (size_t i = 0; i < passwords.size(); ++i) {
        inputData(i, 0) = passwords[i].length();
    }

    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < passwords.size(); ++i) {
        passwords[i] += "_" + std::to_string(predictions[i]);
    }
}

void BruteForceAttack::applyRulesToPasswords(std::vector<std::string>& passwords) {
    logger->info("Applying rules to passwords...", {"BruteForceAttack", "Rules"});
    std::vector<std::string> transformedPasswords;
    for (const auto& password : passwords) {
        auto transformed = ruleEngine->applyRules(password);
        transformedPasswords.insert(transformedPasswords.end(), transformed.begin(), transformed.end());
    }
    passwords = std::move(transformedPasswords);
}

void BruteForceAttack::logBruteForceAttackDetails(const std::string& password) {
    logger->trace("Attempting password: " + password, {"BruteForceAttack", "PasswordDetails"});
}

void BruteForceAttack::passwordWorker() {
    std::vector<std::string> passwords;

    while (!stopFlag) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (passwordQueue.empty()) continue;
            passwords.push_back(passwordQueue.front());
            passwordQueue.pop();
        }

        applyMachineLearningModel(passwords);
        applyRulesToPasswords(passwords);

        for (const auto& password : passwords) {
            logBruteForceAttackDetails(password);
            if (passwordVerificationCallback) {
                passwordVerificationCallback(password);
            } else {
                logger->error("Password verification callback is not set!", {"BruteForceAttack", "Execution"});
            }
        }
        passwords.clear();
    }
}

void BruteForceAttack::execute() {
    logger->info("Starting brute force attack.", {"BruteForceAttack", "Execution"});

    auto future = loadDictionariesAsync();
    future.wait();
    if (stopFlag) return;

    loadPasswordsFromDictionaries();

    threadingUtils->enableMonitoring();
    threadingUtils->runInThread([this]() { passwordWorker(); }).wait();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Brute Force Attack completed.", {"BruteForceAttack", "Execution"});
}

void BruteForceAttack::evaluateModel() {
    logger->info("Evaluating model after brute force attack.", {"BruteForceAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"BruteForceAttack", "Evaluation"});
}

void BruteForceAttack::analyzeErrors() {
    logger->info("Analyzing errors after brute force attack.", {"BruteForceAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}

void BruteForceAttack::manageResources() {
    logger->info("Managing resources after brute force attack.", {"BruteForceAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool BruteForceAttack::checkIfStop() {
    return stopFlag;
}

void BruteForceAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database", {"BruteForceAttack", "DB"});
        stopFlag = true;
    }
}

void BruteForceAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database", {"BruteForceAttack", "DB"});
}



























