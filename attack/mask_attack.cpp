#include "mask_attack.h"
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <fstream>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem;

MaskAttack::MaskAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                       Logger* logger, ThreadingUtils* threadingUtils, 
                       DBManager* dbManager, const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), 
      dbManager(dbManager), threadingStrategy(threadingStrategy), stopFlag(false) {
    logger->info("MaskAttack initialized.", {"MaskAttack", "Initialization"});
}

void MaskAttack::setPasswordVerificationCallback(std::function<bool(const std::string&)> callback) {
    passwordVerificationCallback = callback;
}

std::vector<std::string> MaskAttack::loadMasksFromFile() {
    logger->info("Loading masks from file...", {"MaskAttack", "Masks"});
    std::vector<std::string> masks;
    fs::path maskFilePath = fs::current_path() / "masks/mask.txt";
    std::ifstream maskFile(maskFilePath);

    if (!maskFile.is_open()) {
        logger->error("Failed to open mask file: " + maskFilePath.string(), {"MaskAttack", "MaskLoading"});
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
        logger->error("No masks found in file: " + maskFilePath.string(), {"MaskAttack", "MaskLoading"});
        stopFlag = true;
    } else {
        logger->info("Masks loaded successfully.", {"MaskAttack", "MaskLoading"});
    }

    return masks;
}

std::string MaskAttack::generatePassword(const std::string& mask, const std::unordered_map<char, std::string>& charsets, const std::vector<int>& indices) {
    std::string password(mask.size(), ' ');
    for (size_t i = 0; i < mask.size(); ++i) {
        if (mask[i] == '?') {
            password[i] = charsets.at(mask[i])[indices[i]];
        } else {
            password[i] = mask[i];
        }
    }
    return password;
}

void MaskAttack::applyMachineLearningModel(std::vector<std::string>& passwords) {
    logger->info("Applying machine learning model to passwords.", {"MaskAttack", "MLModel"});
    arma::mat inputData(passwords.size(), 1);
    for (size_t i = 0; i < passwords.size(); ++i) {
        inputData(i, 0) = passwords[i].length();
    }

    arma::Row<size_t> predictions = mlPredictor->predict(inputData);
    for (size_t i = 0; i < passwords.size(); ++i) {
        passwords[i] += "_" + std::to_string(predictions[i]);
    }
}

void MaskAttack::applyRulesToPasswords(std::vector<std::string>& passwords) {
    logger->info("Applying rules to passwords...", {"MaskAttack", "Rules"});
    std::vector<std::string> transformedPasswords;
    for (const auto& password : passwords) {
        auto transformed = ruleEngine->applyRules(password);
        transformedPasswords.insert(transformedPasswords.end(), transformed.begin(), transformed.end());
    }
    passwords = std::move(transformedPasswords);
}

void MaskAttack::logMaskAttackDetails(const std::string& password) {
    logger->trace("Attempting password: " + password, {"MaskAttack", "PasswordDetails"});
}

void MaskAttack::maskWorker() {
    std::vector<std::string> masks = loadMasksFromFile();
    if (stopFlag) return;

    std::unordered_map<char, std::string> charsets = {
        {'l', "abcdefghijklmnopqrstuvwxyz"},   // lower case
        {'u', "ABCDEFGHIJKLMNOPQRSTUVWXYZ"},   // upper case
        {'d', "0123456789"},                   // digits
        {'s', "!@#$%^&*()"},                   // special symbols
        {'?', "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"}  // default
    };

    for (const auto& mask : masks) {
        std::vector<int> indices(mask.size(), 0);

        while (!stopFlag) {
            std::vector<std::string> passwords;
            auto attempt = generatePassword(mask, charsets, indices);
            passwords.push_back(attempt);

            applyMachineLearningModel(passwords);
            applyRulesToPasswords(passwords);

            for (const auto& password : passwords) {
                logMaskAttackDetails(password);
                if (passwordVerificationCallback) {
                    passwordVerificationCallback(password);
                } else {
                    logger->error("Password verification callback is not set!", {"MaskAttack", "Execution"});
                }
            }

            size_t pos = 0;
            while (pos < mask.size() && (mask[pos] != '?' || ++indices[pos] == charsets.at('?').size())) {
                if (mask[pos] == '?') indices[pos] = 0;
                ++pos;
            }
            if (pos == mask.size()) break;
        }
    }
}

void MaskAttack::execute() {
    logger->info("Starting mask attack.", {"MaskAttack", "Execution"});

    auto future = threadingUtils->runInThread([this]() { maskWorker(); });
    future.wait();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Mask Attack completed.", {"MaskAttack", "Execution"});
}

void MaskAttack::evaluateModel() {
    logger->info("Evaluating model after mask attack.", {"MaskAttack", "Evaluation"});
    arma::mat inputData;
    arma::Row<size_t> trueLabels;
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"MaskAttack", "Evaluation"});
}

void MaskAttack::analyzeErrors() {
    logger->info("Analyzing errors after mask attack.", {"MaskAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";
    mlPredictor->analyzeErrors(testDataPath);
}

void MaskAttack::manageResources() {
    logger->info("Managing resources after mask attack.", {"MaskAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool MaskAttack::checkIfStop() {
    return stopFlag;
}

void MaskAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database", {"MaskAttack", "DB"});
        stopFlag = true;
    }
}

void MaskAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database", {"MaskAttack", "DB"});
}





