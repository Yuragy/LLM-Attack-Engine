#include "attack_engine.h"
#include "logging/logger.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cmath>
#include <random>
#include <unordered_set>
#include <iostream>
#include <cstring>

AttackEngine::AttackEngine(DictionaryLoader* dictLoader, RuleEngine* ruleEngine, GPUManager* gpuManager)
    : dictLoader(dictLoader), ruleEngine(ruleEngine), gpuManager(gpuManager), 
      cloudManager(), notificationManager(), dbManager(),
      isAttacking(false), isPaused(false), stopThreads(false),
      dictionaryAttackObj(new DictionaryAttack(gpuManager, dictLoader)),
      bruteForceAttackObj(new BruteForceAttack(gpuManager)),
      maskAttackObj(new MaskAttack(gpuManager)),
      ruleBasedAttackObj(new RuleBasedAttack(gpuManager, ruleEngine, dictLoader)),
      hybridAttackObj(new HybridAttack(gpuManager, dictLoader, ruleEngine)),
      rainbowTableAttackObj(new RainbowTableAttack(gpuManager)),
      markovAttackObj(new MarkovAttack(gpuManager)),
      combinationAttackObj(new CombinationAttack(gpuManager, dictLoader)),
      permutedDictionaryAttackObj(new PermutedDictionaryAttack(gpuManager, dictLoader)),
      fingerprintAttackObj(new FingerprintAttack(gpuManager)),
      statisticalAttackObj(new StatisticalAttack(gpuManager)),
      reverseAttackObj(new ReverseAttack(gpuManager)),
      patternBasedAttackObj(new PatternBasedAttack(gpuManager)),
      socialEngineeringAttackObj(new SocialEngineeringAttack(gpuManager)),
      phishingAttackObj(new PhishingAttack(gpuManager)),
      credentialStuffingAttackObj(new CredentialStuffingAttack(gpuManager)),
      passTheHashAttackObj(new PassTheHashAttack(gpuManager)),
      timingAttackObj(new TimingAttack(gpuManager)) {}

bool AttackEngine::setup(const std::map<std::string, std::string>& config) {
    Logger::info("Настройка движка атак");
    size_t threadCount = std::stoul(config.at("thread_count"));
    startThreadPool(threadCount);
    return true;
}

bool AttackEngine::startDictionaryAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeDictionaryAttack, gpuManager, dictLoader->getWords());
    Logger::info("Словарная атака запущена");
    logAttackState("Dictionary Attack Started");
    notifyAttackState("Dictionary Attack Started");
    return true;
}

bool AttackEngine::startBruteForceAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeBruteForceAttack, gpuManager);
    Logger::info("Брутфорс атака запущена");
    logAttackState("Brute Force Attack Started");
    notifyAttackState("Brute Force Attack Started");
    return true;
}

bool AttackEngine::startMaskAttack(const std::string& mask) {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeMaskAttack, gpuManager, mask);
    Logger::info("Атака по маске запущена");
    logAttackState("Mask Attack Started");
    notifyAttackState("Mask Attack Started");
    return true;
}

bool AttackEngine::startRuleBasedAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeRuleBasedAttack, gpuManager, dictLoader->getWords());
    Logger::info("Атака на основе правил запущена");
    logAttackState("Rule-Based Attack Started");
    notifyAttackState("Rule-Based Attack Started");
    return true;
}

bool AttackEngine::startHybridAttack(const std::string& mask) {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeHybridAttack, gpuManager, dictLoader->getWords(), mask);
    Logger::info("Гибридная атака запущена");
    logAttackState("Hybrid Attack Started");
    notifyAttackState("Hybrid Attack Started");
    return true;
}

bool AttackEngine::startRainbowTableAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeRainbowTableAttack, gpuManager);
    Logger::info("Атака с использованием радужных таблиц запущена");
    logAttackState("Rainbow Table Attack Started");
    notifyAttackState("Rainbow Table Attack Started");
    return true;
}

bool AttackEngine::startMarkovAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeMarkovAttack, gpuManager);
    Logger::info("Атака Маркова запущена");
    logAttackState("Markov Attack Started");
    notifyAttackState("Markov Attack Started");
    return true;
}

bool AttackEngine::startCombinationAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeCombinationAttack, gpuManager);
    Logger::info("Комбинированная атака запущена");
    logAttackState("Combination Attack Started");
    notifyAttackState("Combination Attack Started");
    return true;
}

bool AttackEngine::startPermutedDictionaryAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executePermutedDictionaryAttack, gpuManager);
    Logger::info("Атака с перестановкой словаря запущена");
    logAttackState("Permuted Dictionary Attack Started");
    notifyAttackState("Permuted Dictionary Attack Started");
    return true;
}

bool AttackEngine::startFingerprintAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeFingerprintAttack, gpuManager);
    Logger::info("Атака с использованием отпечатков пальцев запущена");
    logAttackState("Fingerprint Attack Started");
    notifyAttackState("Fingerprint Attack Started");
    return true;
}

bool AttackEngine::startStatisticalAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeStatisticalAttack, gpuManager);
    Logger::info("Статистическая атака запущена");
    logAttackState("Statistical Attack Started");
    notifyAttackState("Statistical Attack Started");
    return true;
}

bool AttackEngine::startReverseAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeReverseAttack, gpuManager);
    Logger::info("Обратная атака запущена");
    logAttackState("Reverse Attack Started");
    notifyAttackState("Reverse Attack Started");
    return true;
}

bool AttackEngine::startPatternBasedAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executePatternBasedAttack, gpuManager);
    Logger::info("Атака на основе шаблонов запущена");
    logAttackState("Pattern-Based Attack Started");
    notifyAttackState("Pattern-Based Attack Started");
    return true;
}

bool AttackEngine::startSocialEngineeringAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::launch::async, &GPUManager::executeSocialEngineeringAttack, gpuManager);
    Logger::info("Атака социальной инженерии запущена");
    logAttackState("Social Engineering Attack Started");
    notifyAttackState("Social Engineering Attack Started");
    return true;
}

bool AttackEngine::startPhishingAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executePhishingAttack, gpuManager);
    Logger::info("Фишинговая атака запущена");
    logAttackState("Phishing Attack Started");
    notifyAttackState("Phishing Attack Started");
    return true;
}

bool AttackEngine::startCredentialStuffingAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeCredentialStuffingAttack, gpuManager);
    Logger::info("Атака с использованием украденных данных запущена");
    logAttackState("Credential Stuffing Attack Started");
    notifyAttackState("Credential Stuffing Attack Started");
    return true;
}

bool AttackEngine::startPassTheHashAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executePassTheHashAttack, gpuManager);
    Logger::info("Атака с передачей хеша запущена");
    logAttackState("Pass-The-Hash Attack Started");
    notifyAttackState("Pass-The-Hash Attack Started");
    return true;
}

bool AttackEngine::startTimingAttack() {
    if (isAttacking) {
        Logger::warning("Атака уже запущена");
        return false;
    }
    isAttacking = true;
    isPaused = false;
    attackFuture = std::async(std::launch::async, &GPUManager::executeTimingAttack, gpuManager);
    Logger::info("Атака по времени запущена");
    logAttackState("Timing Attack Started");
    notifyAttackState("Timing Attack Started");
    return true;
}

bool AttackEngine::stopAttack() {
    if (!isAttacking) {
        Logger::warning("Атака не запущена");
        return false;
    }
    isAttacking = false;
    isPaused = false;
    if (attackFuture.valid()) {
        attackFuture.wait();
    }
    Logger::info("Атака остановлена");
    logAttackState("Attack Stopped");
    notifyAttackState("Attack Stopped");
    return true;
}

bool AttackEngine::pauseAttack() {
    if (!isAttacking || isPaused) {
        Logger::warning("Атака не запущена или уже приостановлена");
        return false;
    }
    isPaused = true;
    Logger::info("Атака приостановлена");
    logAttackState("Attack Paused");
    notifyAttackState("Attack Paused");
    return true;
}

bool AttackEngine::resumeAttack() {
    if (!isAttacking || !isPaused) {
        Logger::warning("Атака не запущена или не приостановлена");
        return false;
    }
    isPaused = false;
    Logger::info("Атака возобновлена");
    logAttackState("Attack Resumed");
    notifyAttackState("Attack Resumed");
    return true;
}

std::string AttackEngine::getAttackStatus() const {
    if (isAttacking) {
        return isPaused ? "Атака приостановлена" : "Атака запущена";
    } else {
        return "Атака остановлена";
    }
}

std::vector<std::string> AttackEngine::applyRulesToDictionaries() const {
    std::vector<std::string> results;
    for (const auto& word : dictLoader->getWords()) {
        auto transformedWords = ruleEngine->applyRules(word);
        results.insert(results.end(), transformedWords.begin(), transformedWords.end());
    }
    return results;
}

bool AttackEngine::addRule(const std::string& rule) {
    if (ruleEngine->addRule(rule)) {
        log("Правило добавлено: " + rule);
        return true;
    }
    return false;
}

bool AttackEngine::removeRule(const std::string& rule) {
    if (ruleEngine->removeRule(rule)) {
        log("Правило удалено: " + rule);
        return true;
    }
    return false;
}

std::unordered_map<std::string, size_t> AttackEngine::getRuleUsageStatistics() const {
    return ruleEngine->getStatistics();
}

std::string AttackEngine::getProgress() const {
    return "Прогресс атаки: ...";
}

std::vector<std::string> AttackEngine::getLogs() const {
    std::lock_guard<std::mutex> lock(logsMutex);
    return logs;
}

std::map<std::string, std::string> AttackEngine::getCurrentConfig() const {
    return {
        {"dictionary_path", "path/to/dictionary"},
        {"rules_path", "path/to/rules"},
        {"gpu_enabled", "true"},
        {"thread_count", "4"}
    };
}

void AttackEngine::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logsMutex);
    logs.push_back(message);
    Logger::info(message);
}

void AttackEngine::logAttackState(const std::string& state) {
    log("Attack State: " + state);
    Logger::info("Attack State: " + state);
}

void AttackEngine::logPerformanceMetrics() {
}

void AttackEngine::startThreadPool(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        threadPool.emplace_back(&AttackEngine::threadLoop, this);
    }
}

void AttackEngine::stopThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopThreads = true;
    }
    queueCondition.notify_all();
    for (auto& thread : threadPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void AttackEngine::threadLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return stopThreads || !taskQueue.empty(); });
            if (stopThreads && taskQueue.empty()) {
                return;
            }
            task = std::move(taskQueue.front());
            taskQueue.pop();
        }
        task();
    }
}

void AttackEngine::handleException(const std::exception& e) {
    log(std::string("Ошибка: ") + e.what());
    stopAttack();
}

void AttackEngine::startAttackCLI(const std::string& attackType, const std::string& parameter) {
    if (attackType == "dictionary") {
        startDictionaryAttack();
    } else if (attackType == "brute_force") {
        startBruteForceAttack();
    } else if (attackType == "mask") {
        startMaskAttack(parameter);
    } else if (attackType == "rule_based") {
        startRuleBasedAttack();
    } else if (attackType == "hybrid") {
        startHybridAttack(parameter);
    } else if (attackType == "rainbow_table") {
        startRainbowTableAttack();
    } else if (attackType == "markov") {
        startMarkovAttack();
    } else if (attackType == "combination") {
        startCombinationAttack();
    } else if (attackType == "permuted_dictionary") {
        startPermutedDictionaryAttack();
    } else if (attackType == "fingerprint") {
        startFingerprintAttack();
    } else if (attackType == "statistical") {
        startStatisticalAttack();
    } else if (attackType == "reverse") {
        startReverseAttack();
    } else if (attackType == "pattern_based") {
        startPatternBasedAttack();
    } else if (attackType == "social_engineering") {
        startSocialEngineeringAttack();
    } else if (attackType == "phishing") {
        startPhishingAttack();
    } else if (attackType == "credential_stuffing") {
        startCredentialStuffingAttack();
    } else if (attackType == "pass_the_hash") {
        startPassTheHashAttack();
    } else if (attackType == "timing") {
        startTimingAttack();
    } else {
        Logger::error("Неизвестный тип атаки: " + attackType);
    }
}

void AttackEngine::stopAttackCLI() {
    stopAttack();
}

void AttackEngine::pauseAttackCLI() {
    pauseAttack();
}

void AttackEngine::resumeAttackCLI() {
    resumeAttack();
}

std::string AttackEngine::getStatusCLI() const {
    return getAttackStatus();
}

std::vector<std::string> AttackEngine::filterLogs(const std::string& status) const {
    std::lock_guard<std::mutex> lock(logsMutex);
    std::vector<std::string> filteredLogs;
    for (const auto& log : logs) {
        if (log.find(status) != std::string::npos) {
            filteredLogs.push_back(log);
        }
    }
    return filteredLogs;
}

void AttackEngine::exportLogs(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(logsMutex);
    std::ofstream outFile(filename);
    for (const auto& log : logs) {
        outFile << log << std::endl;
    }
    outFile.close();
}

void AttackEngine::scheduleAttack(const std::string& attackType, const std::string& parameter) {
    std::async(std::launch::async, [this, attackType, parameter]() {
        startAttackCLI(attackType, parameter);
    });
}

void AttackEngine::monitorScheduledAttacks() {
    while (isAttacking) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Logger::info("Мониторинг запланированных атак...");
    }
}

bool AttackEngine::startAttackAPI(const std::string& attackType, const std::string& parameter) {
    return startAttackCLI(attackType, parameter);
}

bool AttackEngine::stopAttackAPI() {
    return stopAttackCLI();
}

bool AttackEngine::pauseAttackAPI() {
    return pauseAttackCLI();
}

bool AttackEngine::resumeAttackAPI() {
    return resumeAttackCLI();
}

std::string AttackEngine::getStatusAPI() const {
    return getStatusCLI();
}

void AttackEngine::saveResultsToCloud(const std::string& attackType) {
    cloudManager.uploadResults(attackType, logs);
}

void AttackEngine::saveLogsToCloud() {
    cloudManager.uploadLogs(logs);
}

void AttackEngine::notifyAttackState(const std::string& state) {
    notificationManager.sendNotification(state);
}

void AttackEngine::saveResultsToDB(const std::string& attackType) {
    dbManager.saveResults(attackType, logs);
}

void AttackEngine::loadAttackDataFromDB() {
    auto data = dbManager.loadAttackData();
    for (const auto& item : data) {
        Logger::info("Loaded attack data: " + item);
    }
}













