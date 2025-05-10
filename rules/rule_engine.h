#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <regex>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <memory>
#include "logging/logger.h"
#include "config/config.h"
#include "database/db_manager.h"
#include "monitoring/monitor.h"
#include "cloud/cloud_integration.h"
#include "utils/threading_utils.h"

class RuleEngine {
public:
    RuleEngine();
    bool loadRules(const std::string& filePath);
    bool loadBaseAndCustomRules(const std::string& baseFilePath, const std::string& customFilePath);
    bool loadRulesFromString(const std::string& rulesContent);
    bool loadRulesFromURL(const std::string& url);
    std::vector<std::string> applyRules(const std::string& word) const;
    bool addRule(const std::string& rule);
    bool removeRule(const std::string& rule);
    bool containsRule(const std::string& rule) const;
    std::optional<std::string> getRuleByIndex(size_t index) const;
    void clearRules();
    std::unordered_map<std::string, size_t> getStatistics() const;
    std::vector<std::string> getRules() const;
    bool validateRule(const std::string& rule) const;
    bool saveRules(const std::string& filePath) const;
    bool saveBaseAndCustomRules(const std::string& baseFilePath, const std::string& customFilePath) const;
    bool saveRulesToCloud(const std::string& cloudPath) const;
    void evaluatePerformance(const std::string& word) const;
    void setUsageCounterCallback(const std::function<void(const std::string&)>& callback);

private:
    std::vector<std::string> baseRules;
    std::vector<std::string> customRules;
    std::unordered_map<std::string, size_t> ruleUsageCount;
    mutable std::unordered_map<std::string, std::vector<std::string>> cache;
    mutable std::mutex cacheMutex;
    mutable std::shared_mutex rulesMutex;
    std::function<void(const std::string&)> usageCounterCallback;
    void parseRules(const std::string& rulesContent, std::vector<std::string>& rules);
    std::string applyRule(const std::string& word, const std::string& rule) const;
    std::string transformToUpper(const std::string& word) const;
    std::string transformToLower(const std::string& word) const;
    std::string reverseWord(const std::string& word) const;
    std::string insertCharacters(const std::string& word, const std::string& characters, size_t position) const;
    std::string replaceCharacters(const std::string& word, const std::string& pattern, const std::string& replacement) const;
    std::string regexReplaceCharacters(const std::string& word, const std::string& pattern, const std::string& replacement) const;
    std::string duplicateCharacters(const std::string& word, size_t times) const;
    std::string removeCharacters(const std::string& word, const std::string& characters) const;
    std::string caesarCipher(const std::string& word, int shift) const;
    std::string randomCase(const std::string& word) const;
    std::string complexReplace(const std::string& word, const std::unordered_map<char, char>& replacements) const;

    Logger logger;
    Config config;
    std::shared_ptr<DBManager> dbManager;
    std::shared_ptr<Monitor> monitor;
    std::shared_ptr<CloudIntegration> cloudIntegration;
    std::shared_ptr<ThreadingUtils> threadingUtils;
};

#endif // RULE_ENGINE_H






