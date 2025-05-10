#ifndef ATTACK_ENGINE_H
#define ATTACK_ENGINE_H

#include <string>
#include <vector>
#include <future>
#include <atomic>
#include <unordered_map>
#include "../dictionary/dictionary_loader.h"
#include "../rules/rule_engine.h"
#include "../gpu/gpu_manager.h"
#include "../targets/target_interface.h"
#include "../targets/target_factory.h"
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "logging/logger.h"
#include "cloud/cloud_integration.h"
#include "utils/threading_utils.h"
#include "notification/notification_manager.h"
#include "database/db_manager.h"
#include "dictionary_attack.h"
#include "brute_force_attack.h"
#include "mask_attack.h"
#include "rule_based_attack.h"
#include "hybrid_attack.h"
#include "rainbow_table_attack.h"
#include "markov_attack.h"
#include "combination_attack.h"
#include "permuted_dictionary_attack.h"
#include "fingerprint_attack.h"
#include "statistical_attack.h"
#include "reverse_attack.h"
#include "pattern_based_attack.h"
#include "social_engineering_attack.h"
#include "phishing_attack.h"
#include "credential_stuffing_attack.h"
#include "pass_the_hash_attack.h"
#include "timing_attack.h"

class AttackEngine {
public:
    AttackEngine(DictionaryLoader* dictLoader, RuleEngine* ruleEngine, GPUManager* gpuManager);
    bool setup(const std::map<std::string, std::string>& config);
    bool startDictionaryAttack();
    bool startBruteForceAttack();
    bool startMaskAttack(const std::string& mask);
    bool startRuleBasedAttack();
    bool startHybridAttack(const std::string& mask);
    bool startRainbowTableAttack();
    bool startMarkovAttack();
    bool startCombinationAttack();
    bool startPermutedDictionaryAttack();
    bool startFingerprintAttack();
    bool startStatisticalAttack();
    bool startReverseAttack();
    bool startPatternBasedAttack();
    bool startSocialEngineeringAttack();
    bool startPhishingAttack();
    bool startCredentialStuffingAttack();
    bool startPassTheHashAttack();
    bool startTimingAttack();

    bool stopAttack();
    bool pauseAttack();
    bool resumeAttack();
    std::string getAttackStatus() const;
    std::vector<std::string> applyRulesToDictionaries() const;
    bool addRule(const std::string& rule);
    bool removeRule(const std::string& rule);
    std::unordered_map<std::string, size_t> getRuleUsageStatistics() const;
    std::string getProgress() const;
    std::vector<std::string> getLogs() const;
    std::map<std::string, std::string> getCurrentConfig() const;
    void saveState();
    void restoreState();
    void monitorAttack();
    void sendNotification(const std::string& message);
    void startAttackCLI(const std::string& attackType, const std::string& parameter = "");
    void stopAttackCLI();
    void pauseAttackCLI();
    void resumeAttackCLI();
    std::string getStatusCLI() const;
    void logAttackState(const std::string& state);
    std::vector<std::string> filterLogs(const std::string& status) const;
    void exportLogs(const std::string& filename) const;
    void scheduleAttack(const std::string& attackType, const std::string& parameter = "");
    void monitorScheduledAttacks();
    bool startAttackAPI(const std::string& attackType, const std::string& parameter = "");
    bool stopAttackAPI();
    bool pauseAttackAPI();
    bool resumeAttackAPI();
    std::string getStatusAPI() const;
    void saveResultsToCloud(const std::string& attackType);
    void saveLogsToCloud();
    void notifyAttackState(const std::string& state);
    void saveResultsToDB(const std::string& attackType);
    void loadAttackDataFromDB();

private:
    DictionaryLoader* dictLoader;
    RuleEngine* ruleEngine;
    GPUManager* gpuManager;
    CloudManager cloudManager;
    NotificationManager notificationManager;
    DBManager dbManager;
    std::atomic<bool> isAttacking;
    std::atomic<bool> isPaused;
    std::future<void> attackFuture;
    std::vector<std::string> logs;
    mutable std::mutex logsMutex;
    std::vector<std::thread> threadPool;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    bool stopThreads;
    DictionaryAttack* dictionaryAttackObj;
    BruteForceAttack* bruteForceAttackObj;
    MaskAttack* maskAttackObj;
    RuleBasedAttack* ruleBasedAttackObj;
    HybridAttack* hybridAttackObj;
    RainbowTableAttack* rainbowTableAttackObj;
    MarkovAttack* markovAttackObj;
    CombinationAttack* combinationAttackObj;
    PermutedDictionaryAttack* permutedDictionaryAttackObj;
    FingerprintAttack* fingerprintAttackObj;
    StatisticalAttack* statisticalAttackObj;
    ReverseAttack* reverseAttackObj;
    PatternBasedAttack* patternBasedAttackObj;
    SocialEngineeringAttack* socialEngineeringAttackObj;
    PhishingAttack* phishingAttackObj;
    CredentialStuffingAttack* credentialStuffingAttackObj;
    PassTheHashAttack* passTheHashAttackObj;
    TimingAttack* timingAttackObj;

    void log(const std::string& message);
    void logPerformanceMetrics();
    void startThreadPool(size_t threadCount);
    void stopThreadPool();
    void threadLoop();
    void handleException(const std::exception& e);
};

#endif // ATTACK_ENGINE_H



