#include "pass_the_hash_attack.h"
#include <iostream>
#include <future>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

PassTheHashAttack::PassTheHashAttack(Logger* logger, ThreadingUtils* threadingUtils, 
                                     DBManager* dbManager, const std::string& threadingStrategy)
    : logger(logger), threadingUtils(threadingUtils), dbManager(dbManager), 
      threadingStrategy(threadingStrategy), stopFlag(false) {
    logger->info("PassTheHashAttack initialized.", {"PassTheHashAttack", "Initialization"});
}

void PassTheHashAttack::setHashVerificationCallback(std::function<bool(const std::string&)> callback) {
    hashVerificationCallback = callback;
}

std::vector<std::string> PassTheHashAttack::loadHashesFromFile(const std::string& filePath) {
    logger->info("Loading hashes from file: " + filePath, {"PassTheHashAttack", "HashLoading"});

    std::vector<std::string> hashes;
    std::ifstream file(filePath);
    std::string hash;

    if (!file.is_open()) {
        logger->error("Failed to open hash file: " + filePath, {"PassTheHashAttack", "HashLoading"});
        return hashes;
    }

    while (std::getline(file, hash)) {
        if (!hash.empty()) {
            hashes.push_back(hash);
        }
    }

    if (hashes.empty()) {
        logger->error("No hashes found in file: " + filePath, {"PassTheHashAttack", "HashLoading"});
    } else {
        logger->info("Hashes loaded successfully.", {"PassTheHashAttack", "HashLoading"});
    }

    return hashes;
}

void PassTheHashAttack::logHashAttackDetails(const std::string& hash) {
    logger->trace("Attempting hash: " + hash, {"PassTheHashAttack", "HashDetails"});
}

void PassTheHashAttack::hashWorker() {
    while (!stopFlag) {
        std::string hash;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (hashQueue.empty()) {
                continue;
            }
            hash = hashQueue.front();
            hashQueue.pop();
        }

        logHashAttackDetails(hash);

        if (hashVerificationCallback && hashVerificationCallback(hash)) {
            stopFlag = true; 
        }

        if (checkIfStop()) break;
    }
}

void PassTheHashAttack::execute() {
    logger->info("Starting pass the hash attack.", {"PassTheHashAttack", "Execution"});

    std::string hashFilePath = (fs::current_path() / "hashes/hash.txt").string();
    auto hashes = loadHashesFromFile(hashFilePath);
    
    if (hashes.empty()) {
        logger->error("No hashes loaded, aborting attack.", {"PassTheHashAttack", "Execution"});
        return;
    }

    // Установка хэшей в очередь
    for (const auto& hash : hashes) {
        std::unique_lock<std::mutex> lock(mtx);
        hashQueue.push(hash);
    }

    threadingUtils->enableMonitoring();
    std::vector<std::function<void()>> tasks;
    tasks.push_back([this]() { hashWorker(); });

    threadingUtils->runInParallel(tasks, threadingStrategy);

    stopFlag = true;
    threadingUtils->stopThreads();

    logger->info("Pass the hash attack completed.", {"PassTheHashAttack", "Execution"});
}

bool PassTheHashAttack::checkIfStop() {
    return stopFlag;
}

void PassTheHashAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database", {"PassTheHashAttack", "DB"});
        stopFlag = true;
    }
}

void PassTheHashAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database", {"PassTheHashAttack", "DB"});
}









