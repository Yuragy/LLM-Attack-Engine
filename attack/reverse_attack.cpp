#include "reverse_attack.h"
#include <iostream>
#include <future>
#include <string>
#include <fstream> 
#include <bcrypt.h>  

const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";

ReverseAttack::ReverseAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                             Logger* logger, ThreadingUtils* threadingUtils)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), 
      logger(logger), threadingUtils(threadingUtils), 
      hashType("bcrypt"), hashComparisonCallback(nullptr) {}

void ReverseAttack::setHashType(std::string type) {
    hashType = type;
}

void ReverseAttack::setHashComparisonCallback(std::function<void(const std::string&, const std::string&)> callback) {
    hashComparisonCallback = callback;
}
std::string ReverseAttack::generateCandidate(size_t index, const std::string& charset) {
    std::string candidate;
    while (index > 0) {
        candidate += charset[index % charset.size()];
        index /= charset.size();
    }
    return candidate;
}

void ReverseAttack::applyMachineLearningModel(std::string& candidate) {
    arma::mat inputData(1, 1); 
    inputData(0, 0) = candidate.length(); 

    arma::Row<size_t> prediction = mlPredictor->predict(inputData);  
    candidate += "_ml" + std::to_string(prediction[0]);  

    logger->info("Machine learning model applied to candidate.", {"ReverseAttack", "ML"});
}

void ReverseAttack::applyRulesToCandidate(std::string& candidate) {
    auto transformed = ruleEngine->applyRules(candidate);
    if (!transformed.empty()) {
        candidate = transformed[0]; 
    }

    logger->info("Transformation rules applied to candidate.", {"ReverseAttack", "Rules"});
}

void ReverseAttack::logAttackDetails(const std::string& candidate, const std::string& hash) {
    logger->trace("Attempting candidate: " + candidate + " with hash: " + hash, {"ReverseAttack", "AttackDetails"});
}

void ReverseAttack::generateAndProcessCandidates(const std::string& knownHash, size_t totalCandidates) {
    logger->info("Starting candidate generation and processing.", {"ReverseAttack", "Execution"});

    tbb::parallel_for(size_t(0), totalCandidates, [&](size_t i) {
        std::string candidate = generateCandidate(i, charset);

        applyMachineLearningModel(candidate);
        applyRulesToCandidate(candidate);

        std::string hash;
        if (hashType == "bcrypt") {
            hash = bcryptHash(candidate);
        } else {
            logger->error("Unsupported hash type: " + hashType, {"ReverseAttack", "Execution"});
            return;
        }

        logAttackDetails(candidate, hash);

        if (hashComparisonCallback) {
            hashComparisonCallback(hash, knownHash);
        } else {
            logger->error("Hash comparison callback is not set!", {"ReverseAttack", "Execution"});
        }
    });

    logger->info("Completed candidate generation and processing.", {"ReverseAttack", "Execution"});
}

std::string ReverseAttack::loadKnownHashFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string knownHash;
    if (file.is_open()) {
        std::getline(file, knownHash); 
        file.close();
        logger->info("Loaded known hash from file: " + knownHash, {"ReverseAttack", "FileLoad"});

        if (!isValidHash(knownHash)) {
            logger->error("Invalid hash format: " + knownHash, {"ReverseAttack", "FileError"});
            return "";
        }
    } else {
        logger->error("Failed to open file: " + filePath, {"ReverseAttack", "FileError"});
    }
    return knownHash;
}

bool ReverseAttack::isValidHash(const std::string& hash) {
    return hash.size() == 60 && hash.substr(0, 4) == "$2b$";
}

std::string ReverseAttack::bcryptHash(const std::string& candidate) {
    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];
    if (bcrypt_gensalt(12, salt) != 0) {
        logger->error("Failed to generate salt", {"ReverseAttack", "Hashing"});
        return "";
    }
    if (bcrypt_hashpw(candidate.c_str(), salt, hash) != 0) {
        logger->error("bcrypt hash failed", {"ReverseAttack", "Hashing"});
        return "";
    }

    return std::string(hash);
}
void ReverseAttack::execute() {
    logger->info("Starting reverse attack.", {"ReverseAttack", "Execution"});
    std::string knownHash = loadKnownHashFromFile("path/to/known_hash.txt"); 
    if (knownHash.empty()) {
        logger->error("No valid hash loaded.", {"ReverseAttack", "Execution"});
        return;
    }
    size_t totalCandidates = 1000000;
    generateAndProcessCandidates(knownHash, totalCandidates);

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Reverse attack completed.", {"ReverseAttack", "Execution"});
}
void ReverseAttack::evaluateModel() {
    logger->info("Evaluating model after reverse attack.", {"ReverseAttack", "Evaluation"});
    arma::mat inputData;  
    arma::Row<size_t> trueLabels; 
    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"ReverseAttack", "Evaluation"});
}
void ReverseAttack::analyzeErrors() {
    logger->info("Analyzing errors after reverse attack.", {"ReverseAttack", "ErrorAnalysis"});
    std::string testDataPath = "path/to/test_data.txt";  
    mlPredictor->analyzeErrors(testDataPath);
}
void ReverseAttack::manageResources() {
    logger->info("Releasing resources after reverse attack.", {"ReverseAttack", "ResourceManagement"});
}








