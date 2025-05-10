#include "dictionary_loader.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <pugixml.hpp>

using json = nlohmann::json;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

DictionaryLoader::DictionaryLoader(Config& config, std::shared_ptr<CloudIntegration> cloudIntegration, 
                                   std::shared_ptr<ThreadingUtils> threadingUtils, 
                                   std::shared_ptr<Monitor> monitor, std::shared_ptr<DBManager> dbManager,
                                   std::shared_ptr<Logger> logger, 
                                   std::shared_ptr<RuleEngine> ruleEngine)
    : config_(config), cloudIntegration_(cloudIntegration), threadingUtils_(threadingUtils), 
      monitor_(monitor), dbManager_(dbManager), logger_(logger), 
      ruleEngine_(ruleEngine) {
}

bool DictionaryLoader::load(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info("Loading dictionary from file: " + filePath);

    bool result = loadFile(filePath);
    if (result) {
        checkCompatibilityWithNewAttacks();
        monitor_->monitorDictionaryUsage(filePath, true);
        dbManager_->logDBOperation("Load dictionary from file", "Success");
    } else {
        monitor_->monitorDictionaryUsage(filePath, false);
        dbManager_->logDBOperation("Load dictionary from file", "Failed");
    }
    return result;
}

std::future<bool> DictionaryLoader::loadAsync(const std::string& filePath) {
    logger_->info("Asynchronous loading of dictionary from file: " + filePath);
    return threadingUtils_->asyncTask([this, filePath]() {
        return load(filePath);
    });
}

bool DictionaryLoader::loadMultiple(const std::vector<std::string>& filePaths) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& filePath : filePaths) {
        if (!loadFile(filePath)) {
            monitor_->monitorDictionaryUsage(filePath, false);
            dbManager_->logDBOperation("Load multiple dictionaries", "Failed");
            logger_->error("Failed to load dictionary from file: " + filePath);
            return false;
        }
        monitor_->monitorDictionaryUsage(filePath, true);
    }
    checkCompatibilityWithNewAttacks();
    dbManager_->logDBOperation("Load multiple dictionaries", "Success");
    return true;
}

std::future<bool> DictionaryLoader::loadMultipleAsync(const std::vector<std::string>& filePaths) {
    logger_->info("Asynchronous loading of multiple dictionaries from files");
    return threadingUtils_->asyncTask([this, filePaths]() {
        return loadMultiple(filePaths);
    });
}

bool DictionaryLoader::uploadDictionaryToCloud(const std::string& cloudPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info("Uploading dictionary to cloud: " + cloudPath);
    bool result = cloudIntegration_->uploadData("local_dictionary_file.txt", cloudPath);
    monitor_->monitorCloudResources(cloudPath, result ? "Uploaded" : "Upload failed");
    dbManager_->logDBOperation("Upload dictionary to cloud", result ? "Success" : "Failed");
    return result;
}

bool DictionaryLoader::downloadDictionaryFromCloud(const std::string& cloudPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info("Downloading dictionary from cloud: " + cloudPath);
    bool result = cloudIntegration_->downloadData(cloudPath, "local_dictionary_file.txt");
    monitor_->monitorCloudResources(cloudPath, result ? "Downloaded" : "Download failed");
    dbManager_->logDBOperation("Download dictionary from cloud", result ? "Success" : "Failed");
    return result;
}

bool DictionaryLoader::loadFromDatabase(const std::string& connectionString) {
    if (!dbManager_->connect()) {
        logger_->error("Failed to connect to database");
        monitor_->monitorDictionaryUsage("Database", false);
        return false;
    }

    std::string query = "SELECT password FROM passwords";
    std::string dbWords = dbManager_->fetchData(query);
    if (dbWords.empty()) {
        dbManager_->logDBOperation("Load dictionary from database", "No data found");
        logger_->error("No data found in database for query: " + query);
        return false;
    }

    std::istringstream iss(dbWords);
    std::string word;
    while (iss >> word) {
        if (isValidWord(word)) {
            words.insert(word);
            logger_->info("Loaded word from database: " + word);
        } else {
            logger_->error("Invalid word from database: " + word);
        }
    }

    dbManager_->logDBOperation("Load dictionary from database", "Success");
    monitor_->monitorDictionaryUsage("Database", true);
    return true;
}

std::future<bool> DictionaryLoader::loadFromDatabaseAsync(const std::string& connectionString) {
    logger_->info("Asynchronous loading of dictionary from database");
    return threadingUtils_->asyncTask([this, connectionString]() {
        return loadFromDatabase(connectionString);
    });
}

bool DictionaryLoader::loadFromAPI(const std::string& apiEndpoint) {
    try {
        http_client client(U(apiEndpoint));
        uri_builder builder(U("/"));
        pplx::task<http_response> responseTask = client.request(methods::GET, builder.to_string());

        http_response response = responseTask.get();
        if (response.status_code() != status_codes::OK) {
            logger_->error("API request error: " + apiEndpoint);
            monitor_->monitorDictionaryUsage(apiEndpoint, false);
            return false;
        }

        pplx::task<json::value> jsonTask = response.extract_json();
        json::value jsonResponse = jsonTask.get();

        std::vector<std::string> apiWords;
        for (const auto& word : jsonResponse.as_array()) {
            apiWords.push_back(word.as_string());
        }
        addWords(apiWords);
        logger_->info("Dictionary loaded from API: " + apiEndpoint);
        checkCompatibilityWithNewAttacks();
        monitor_->monitorDictionaryUsage(apiEndpoint, true);
        return true;
    } catch (const std::exception& e) {
        logger_->error(std::string("API load error: ") + e.what());
        monitor_->monitorDictionaryUsage(apiEndpoint, false);
        return false;
    }
}

std::future<bool> DictionaryLoader::loadFromAPIAsync(const std::string& apiEndpoint) {
    logger_->info("Asynchronous loading of dictionary from API: " + apiEndpoint);
    return threadingUtils_->asyncTask([this, apiEndpoint]() {
        return loadFromAPI(apiEndpoint);
    });
}

bool DictionaryLoader::loadFromCSV(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logger_->error("Failed to open CSV file: " + filePath);
        monitor_->monitorDictionaryUsage(filePath, false);
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        words.insert(line);
        logger_->info("Loaded word from CSV: " + line);
    }

    file.close();
    wordsVector.assign(words.begin(), words.end());
    cacheFrequentlyUsedWords();
    checkCompatibilityWithNewAttacks();
    monitor_->monitorDictionaryUsage(filePath, true);
    return true;
}

bool DictionaryLoader::loadFromXML(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info("Loading dictionary from XML file: " + filePath);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filePath.c_str());

    if (!result) {
        logger_->error("Failed to load XML file: " + filePath);
        monitor_->monitorDictionaryUsage(filePath, false);
        return false;
    }

    for (pugi::xml_node wordNode : doc.child("Dictionary").children("Word")) {
        std::string word = wordNode.text().as_string();
        if (isValidWord(word)) {
            words.insert(word);
            logger_->info("Loaded word from XML: " + word);
        } else {
            logger_->error("Invalid word in XML: " + word);
        }
    }

    wordsVector.assign(words.begin(), words.end());
    cacheFrequentlyUsedWords();
    checkCompatibilityWithNewAttacks();
    monitor_->monitorDictionaryUsage(filePath, true);
    return true;
}

std::vector<std::string> DictionaryLoader::getAllWords() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<std::string>(words.begin(), words.end());
}

void DictionaryLoader::cacheFrequentlyUsedWords() {
    std::unordered_set<std::string> frequentlyUsedWords = {"password", "123456", "123456789"};
    for (const auto& word : frequentlyUsedWords) {
        if (words.find(word) != words.end()) {
            logger_->info("Word cached: " + word);
        }
    }
}

bool DictionaryLoader::isValidWord(const std::string& word) const {
    if (word.empty() || word.length() > 128) {
        logger_->error("Invalid word: " + word);
        return false;
    }
    return true;
}

bool DictionaryLoader::loadFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logger_->error("Failed to open dictionary file: " + filePath);
        monitor_->monitorDictionaryUsage(filePath, false);
        return false;
    }

    std::string word;
    while (file >> word) {
        if (isValidWord(word)) {
            words.insert(word);
            logger_->info("Loaded word: " + word);
        } else {
            logger_->error("Invalid word: " + word);
        }
    }

    file.close();
    wordsVector.assign(words.begin(), words.end());
    cacheFrequentlyUsedWords();
    monitor_->monitorDictionaryUsage(filePath, true);
    return true;
}

bool DictionaryLoader::saveFile(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        logger_->error("Failed to save dictionary file: " + filePath);
        monitor_->monitorDictionaryUsage(filePath, false);
        return false;
    }

    for (const auto& word : words) {
        file << word << std::endl;
    }

    file.close();
    logger_->info("Dictionary saved to file: " + filePath);
    monitor_->monitorDictionaryUsage(filePath, true);
    return true;
}

void DictionaryLoader::addWords(const std::vector<std::string>& newWords) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& word : newWords) {
        if (isValidWord(word)) {
            words.insert(word);
            logger_->info("Added word: " + word);
        } else {
            logger_->error("Invalid word: " + word);
        }
    }
    wordsVector.assign(words.begin(), words.end());
    cacheFrequentlyUsedWords();
    checkCompatibilityWithNewAttacks();
}

std::future<void> DictionaryLoader::addWordsAsync(const std::vector<std::string>& newWords) {
    logger_->info("Asynchronous adding of words");
    return threadingUtils_->asyncTask([this, newWords]() {
        addWords(newWords);
        checkCompatibilityWithNewAttacks();
    });
}

void DictionaryLoader::removeWords(const std::vector<std::string>& removeWords) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& word : removeWords) {
        words.erase(word);
        logger_->info("Removed word: " + word);
    }
    wordsVector.assign(words.begin(), words.end());
    cacheFrequentlyUsedWords();
    checkCompatibilityWithNewAttacks();
}

std::future<void> DictionaryLoader::removeWordsAsync(const std::vector<std::string>& removeWords) {
    logger_->info("Asynchronous removal of words");
    return threadingUtils_->asyncTask([this, removeWords]() {
        removeWords(removeWords);
        checkCompatibilityWithNewAttacks();
    });
}

void DictionaryLoader::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    words.clear();
    wordsVector.clear();
    logger_->info("Dictionary cleared");
    monitor_->monitorDictionaryUsage("Dictionary cleared", true);
}

std::future<void> DictionaryLoader::clearAsync() {
    logger_->info("Asynchronous clearing of dictionary");
    return threadingUtils_->asyncTask([this]() {
        clear();
    });
}

std::unordered_map<std::string, size_t> DictionaryLoader::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::unordered_map<std::string, size_t> stats;
    stats["total_words"] = words.size();
    logger_->info("Dictionary statistics: total words - " + std::to_string(words.size()));
    return stats;
}

std::unordered_map<std::string, size_t> DictionaryLoader::getDetailedStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::unordered_map<std::string, size_t> stats;
    for (const auto& word : words) {
        stats[word]++;
    }
    return stats;
}

bool DictionaryLoader::contains(const std::string& word) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return words.find(word) != words.end();
}

std::optional<std::string> DictionaryLoader::getWordByIndex(size_t index) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < wordsVector.size()) {
        return wordsVector[index];
    }
    return std::nullopt;
}

bool DictionaryLoader::save(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info("Saving dictionary to file: " + filePath);
    bool result = saveFile(filePath);
    monitor_->monitorDictionaryUsage(filePath, result);
    dbManager_->logDBOperation("Save dictionary to file", result ? "Success" : "Failed");
    return result;
}

std::future<bool> DictionaryLoader::saveAsync(const std::string& filePath) const {
    logger_->info("Asynchronous saving of dictionary to file: " + filePath);
    return threadingUtils_->asyncTask([this, filePath]() {
        return save(filePath);
    });
}

void DictionaryLoader::sendPasswordsToRuleEngine() {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info("Sending passwords to rule engine");
    ruleEngine_->applyRules(wordsVector);
}

void DictionaryLoader::checkCompatibilityWithNewAttacks() {
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info("Checking compatibility with new attack types");
    for (const auto& word : words) {
        ruleEngine_->evaluatePerformance(word);
    }
}















