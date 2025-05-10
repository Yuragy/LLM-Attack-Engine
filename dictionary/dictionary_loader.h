#ifndef DICTIONARY_LOADER_H
#define DICTIONARY_LOADER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <future>
#include <mutex>
#include "rule_engine.h"
#include "logger.h"
#include "db_manager.h"
#include "monitor.h"
#include "threading_utils.h"
#include "cloud_integration.h"
#include "config.h"

class DictionaryLoader {
public:
    DictionaryLoader(Config& config, std::shared_ptr<CloudIntegration> cloudIntegration, 
                     std::shared_ptr<ThreadingUtils> threadingUtils, std::shared_ptr<Monitor> monitor,
                     std::shared_ptr<DBManager> dbManager, std::shared_ptr<Logger> logger, 
                     std::shared_ptr<RuleEngine> ruleEngine);

    bool load(const std::string& filePath);
    std::future<bool> loadAsync(const std::string& filePath);
    bool loadMultiple(const std::vector<std::string>& filePaths);
    std::future<bool> loadMultipleAsync(const std::vector<std::string>& filePaths);
    const std::unordered_set<std::string>& getWords() const;
    void addWords(const std::vector<std::string>& words);
    std::future<void> addWordsAsync(const std::vector<std::string>& words);
    void removeWords(const std::vector<std::string>& words);
    std::future<void> removeWordsAsync(const std::vector<std::string>& words);
    void clear();
    std::future<void> clearAsync();
    std::unordered_map<std::string, size_t> getStatistics() const;
    std::unordered_map<std::string, size_t> getDetailedStatistics() const;
    bool contains(const std::string& word) const;
    std::optional<std::string> getWordByIndex(size_t index) const;
    bool save(const std::string& filePath) const;
    std::future<bool> saveAsync(const std::string& filePath) const;
    bool loadFromDatabase(const std::string& connectionString);
    std::future<bool> loadFromDatabaseAsync(const std::string& connectionString);
    bool loadFromAPI(const std::string& apiEndpoint);
    std::future<bool> loadFromAPIAsync(const std::string& apiEndpoint);
    bool loadFromCSV(const std::string& filePath);
    bool loadFromXML(const std::string& filePath);
    bool uploadDictionaryToCloud(const std::string& cloudPath);
    bool downloadDictionaryFromCloud(const std::string& cloudPath);
    std::vector<std::string> getAllWords() const;
    void sendPasswordsToRuleEngine();

private:
    std::unordered_set<std::string> words;
    std::vector<std::string> wordsVector;
    mutable std::mutex mutex_;
    std::shared_ptr<RuleEngine> ruleEngine_;  
    Config& config_;
    std::shared_ptr<CloudIntegration> cloudIntegration_;
    std::shared_ptr<ThreadingUtils> threadingUtils_;
    std::shared_ptr<Monitor> monitor_;  
    std::shared_ptr<DBManager> dbManager_;  
    std::shared_ptr<Logger> logger_;  
    void cacheFrequentlyUsedWords();
    bool isValidWord(const std::string& word) const;
    bool loadFile(const std::string& filePath);
    bool saveFile(const std::string& filePath) const;
    void checkCompatibilityWithNewAttacks();
};

#endif // DICTIONARY_LOADER_H









