#ifndef PASS_THE_HASH_ATTACK_H
#define PASS_THE_HASH_ATTACK_H

#include "../logging/logger.h"
#include "../utils/threading_utils.h"
#include "../database/db_manager.h"
#include <string>
#include <functional>
#include <future>
#include <mutex>
#include <atomic>
#include <queue>

class PassTheHashAttack {
public:
    PassTheHashAttack(Logger* logger, ThreadingUtils* threadingUtils, 
                      DBManager* dbManager, const std::string& threadingStrategy);

    void execute(); 
    void setHashVerificationCallback(std::function<bool(const std::string&)> callback);

private:
    Logger* logger;
    ThreadingUtils* threadingUtils;
    DBManager* dbManager;

    std::function<bool(const std::string&)> hashVerificationCallback;

    std::string threadingStrategy;
    std::mutex mtx;
    std::queue<std::string> hashQueue;
    std::atomic<bool> stopFlag = false;

    std::vector<std::string> loadHashesFromFile(const std::string& filePath);

    void hashWorker();
    void logHashAttackDetails(const std::string& hash);

    bool checkIfStop();
    void connectToDatabase();
    void disconnectFromDatabase();
};

#endif // PASS_THE_HASH_ATTACK_H




