#include "rule_engine.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <curl/curl.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

RuleEngine::RuleEngine()
    : logger(), config(),
      dbManager(std::make_shared<DBManager>(config.getDBConnectionString(), std::make_shared<DataUtils>(), std::make_shared<Logger>(), std::make_shared<NotificationManager>(), std::make_shared<ThreadingUtils>(), std::make_shared<Config>(config))),
      monitor(std::make_shared<Monitor>(std::make_shared<Config>(config), std::make_shared<Logger>(), dbManager)),
      cloudIntegration(std::make_shared<CloudIntegration>("service_name", "api_key", std::make_shared<Config>(config), std::make_shared<ThreadingUtils>(std::make_shared<Logger>(), std::make_shared<Config>(config)), std::make_shared<NotificationManager>())), 
      threadingUtils(std::make_shared<ThreadingUtils>(std::make_shared<Logger>(), std::make_shared<Config>(config))) {
    Logger::initialize(config.get("logger_config_path").value_or("config/logger_config.json").get<std::string>());
    if (!config.validate()) {
        Logger::critical("Конфигурация не прошла валидацию.");
        throw std::runtime_error("Invalid configuration.");
    }
    config.load(config.getRulesPath());
    Logger::info("Инициализация RuleEngine завершена");
    monitor->initialize(config.getMonitoringConfig().at("config_file").get<std::string>());
    monitor->startMonitoring();
    if (!dbManager->connect()) {
        Logger::error("Ошибка подключения к базе данных");
    }
    threadingUtils->enableMonitoring();
    config.registerChangeCallback([this](const std::string& key, const Config::ConfigValue& oldValue, const Config::ConfigValue& newValue) {
        Logger::info("Изменение конфигурации: " + key);
        if (key == "rules_path") {
            loadRules(std::get<std::string>(newValue));
        }
    });
}

bool RuleEngine::loadRules(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        Logger::error("Не удалось открыть файл правил: " + filePath);
        monitor->monitorRuleApplication("loadRules", filePath, false);
        dbManager->logDBError("Не удалось открыть файл правил: " + filePath);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    parseRules(buffer.str(), customRules);
    file.close();

    Logger::info("Правила загружены из файла: " + filePath);
    monitor->monitorRuleApplication("loadRules", filePath, true);
    dbManager->logDBOperation("Загрузка правил", "Успешно загружены правила из файла: " + filePath);
    return true;
}

bool RuleEngine::loadBaseAndCustomRules(const std::string& baseFilePath, const std::string& customFilePath) {
    std::ifstream baseFile(baseFilePath);
    if (!baseFile.is_open()) {
        Logger::error("Не удалось открыть базовый файл правил: " + baseFilePath);
        monitor->monitorRuleApplication("loadBaseAndCustomRules", baseFilePath, false);
        dbManager->logDBError("Не удалось открыть базовый файл правил: " + baseFilePath);
        return false;
    }

    std::stringstream baseBuffer;
    baseBuffer << baseFile.rdbuf();
    parseRules(baseBuffer.str(), baseRules);
    baseFile.close();

    std::ifstream customFile(customFilePath);
    if (!customFile.is_open()) {
        Logger::error("Не удалось открыть пользовательский файл правил: " + customFilePath);
        monitor->monitorRuleApplication("loadBaseAndCustomRules", customFilePath, false);
        dbManager->logDBError("Не удалось открыть пользовательский файл правил: " + customFilePath);
        return false;
    }

    std::stringstream customBuffer;
    customBuffer << customFile.rdbuf();
    parseRules(customBuffer.str(), customRules);
    customFile.close();

    Logger::info("Базовые и пользовательские правила загружены из файлов: " + baseFilePath + ", " + customFilePath);
    monitor->monitorRuleApplication("loadBaseAndCustomRules", baseFilePath + " и " + customFilePath, true);
    dbManager->logDBOperation("Загрузка базовых и пользовательских правил", "Успешно загружены правила из файлов: " + baseFilePath + ", " + customFilePath);
    return true;
}

bool RuleEngine::loadRulesFromString(const std::string& rulesContent) {
    parseRules(rulesContent, customRules);
    Logger::info("Правила загружены из строки");
    monitor->monitorRuleApplication("loadRulesFromString", "string_content", true);
    dbManager->logDBOperation("Загрузка правил из строки", "Успешно загружены правила из строки");
    return true;
}

bool RuleEngine::loadRulesFromURL(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            Logger::error("Не удалось загрузить правила из URL: " + url);
            monitor->monitorRuleApplication("loadRulesFromURL", url, false);
            dbManager->logDBError("Не удалось загрузить правила из URL: " + url);
            return false;
        }

        parseRules(readBuffer, customRules);
        Logger::info("Правила загружены из URL: " + url);
        monitor->monitorRuleApplication("loadRulesFromURL", url, true);
        dbManager->logDBOperation("Загрузка правил из URL", "Успешно загружены правила из URL: " + url);
        return true;
    }

    Logger::error("Не удалось инициализировать CURL для URL: " + url);
    monitor->monitorRuleApplication("loadRulesFromURL", url, false);
    dbManager->logDBError("Не удалось инициализировать CURL для URL: " + url);
    return false;
}

void RuleEngine::parseRules(const std::string& rulesContent, std::vector<std::string>& rules) {
    std::unique_lock<std::shared_mutex> lock(rulesMutex);
    std::stringstream ss(rulesContent);
    std::string rule;
    while (std::getline(ss, rule)) {
        if (validateRule(rule)) {
            rules.push_back(rule);
            ruleUsageCount[rule] = 0;
            Logger::info("Загружено правило: " + rule);
            monitor->monitorRuleApplication(rule, "parseRules", true);
            dbManager->logDBOperation("Загрузка правила", "Загружено правило: " + rule);
        } else {
            Logger::warning("Некорректное правило: " + rule);
            monitor->monitorRuleApplication(rule, "parseRules", false);
            dbManager->logDBError("Некорректное правило: " + rule);
        }
    }
}

std::vector<std::string> RuleEngine::applyRules(const std::string& word) const {
    {
        std::shared_lock<std::mutex> lock(cacheMutex);
        auto it = cache.find(word);
        if (it != cache.end()) {
            return it->second;
        }
    }

    std::vector<std::string> transformedWords(baseRules.size() + customRules.size());
    std::vector<std::function<void()>> tasks;

    auto applyRuleTask = [&](const std::string& rule, size_t index) {
        transformedWords[index] = applyRule(word, rule);
    };

    size_t index = 0;
    for (const auto& rule : baseRules) {
        tasks.push_back([&, index]() { applyRuleTask(rule, index); });
        index++;
    }
    for (const auto& rule : customRules) {
        tasks.push_back([&, index]() { applyRuleTask(rule, index); });
        index++;
    }
    threadingUtils->runInParallel(tasks);

    {
        std::unique_lock<std::mutex> lock(cacheMutex);
        cache[word] = transformedWords;
    }

    monitor->monitorRuleApplication("applyRules", word, true);
    dbManager->logDBOperation("Применение правил", "Применены правила к слову: " + word);
    Logger::info("Применены правила к слову: " + word);
    return transformedWords;
}

std::string RuleEngine::applyRule(const std::string& word, const std::string& rule) const {
    Logger::info("Применение правила: " + rule + " к слову: " + word);
    ruleUsageCount[rule]++;
    if (usageCounterCallback) {
        usageCounterCallback(rule);
    }

    if (rule == "upper") {
        return transformToUpper(word);
    } else if (rule == "lower") {
        return transformToLower(word);
    } else if (rule == "reverse") {
        return reverseWord(word);
    } else if (rule.find("insert_") == 0) {
        size_t pos = rule.find('_', rule.find('_') + 1);
        std::string characters = rule.substr(7, pos - 7);
        size_t position = std::stoul(rule.substr(pos + 1));
        return insertCharacters(word, characters, position);
    } else if (rule.find("replace_") == 0) {
        size_t pos = rule.find('_', rule.find('_') + 1);
        std::string pattern = rule.substr(8, pos - 8);
        std::string replacement = rule.substr(pos + 1);
        return replaceCharacters(word, pattern, replacement);
    } else if (rule.find("regex_replace_") == 0) {
        size_t pos = rule.find('_', rule.find('_') + 1);
        std::string pattern = rule.substr(14, pos - 14);
        std::string replacement = rule.substr(pos + 1);
        return regexReplaceCharacters(word, pattern, replacement);
    } else if (rule.find("duplicate_") == 0) {
        size_t times = std::stoul(rule.substr(10));
        return duplicateCharacters(word, times);
    } else if (rule.find("remove_") == 0) {
        std::string characters = rule.substr(7);
        return removeCharacters(word, characters);
    } else if (rule.find("caesar_") == 0) {
        int shift = std::stoi(rule.substr(7));
        return caesarCipher(word, shift);
    } else if (rule == "random_case") {
        return randomCase(word);
    } else if (rule.find("complex_replace") == 0) {
        std::unordered_map<char, char> replacements = {{'a', '@'}, {'e', '3'}, {'i', '1'}, {'o', '0'}, {'s', '$'}};
        return complexReplace(word, replacements);
    }
    Logger::warning("Неизвестное правило: " + rule);
    monitor->monitorRuleApplication(rule, word, false);
    dbManager->logDBError("Неизвестное правило: " + rule);
    return word;
}

std::unordered_map<std::string, size_t> RuleEngine::getStatistics() const {
    std::shared_lock<std::shared_mutex> lock(rulesMutex);
    return ruleUsageCount;
}

bool RuleEngine::saveRules(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        Logger::error("Не удалось сохранить файл правил: " + filePath);
        monitor->monitorRuleApplication("saveRules", filePath, false);
        dbManager->logDBError("Не удалось сохранить файл правил: " + filePath);
        return false;
    }

    std::shared_lock<std::shared_mutex> lock(rulesMutex);
    for (const auto& rule : customRules) {
        file << rule << '\n';
    }

    file.close();
    Logger::info("Правила сохранены в файл: " + filePath);
    monitor->monitorRuleApplication("saveRules", filePath, true);
    dbManager->logDBOperation("Сохранение правил", "Правила сохранены в файл: " + filePath);
    return true;
}

bool RuleEngine::saveBaseAndCustomRules(const std::string& baseFilePath, const std::string& customFilePath) const {
    std::ofstream baseFile(baseFilePath);
    if (!baseFile.is_open()) {
        Logger::error("Не удалось сохранить базовый файл правил: " + baseFilePath);
        monitor->monitorRuleApplication("saveBaseAndCustomRules", baseFilePath, false);
        dbManager->logDBError("Не удалось сохранить базовый файл правил: " + baseFilePath);
        return false;
    }

    std::shared_lock<std::shared_mutex> lock(rulesMutex);
    for (const auto& rule : baseRules) {
        baseFile << rule << '\n';
    }
    baseFile.close();

    std::ofstream customFile(customFilePath);
    if (!customFile.is_open()) {
        Logger::error("Не удалось сохранить пользовательский файл правил: " + customFilePath);
        monitor->monitorRuleApplication("saveBaseAndCustomRules", customFilePath, false);
        dbManager->logDBError("Не удалось сохранить пользовательский файл правил: " + customFilePath);
        return false;
    }

    for (const auto& rule : customRules) {
        customFile << rule << '\n';
    }
    customFile.close();

    Logger::info("Базовые и пользовательские правила сохранены в файлы: " + baseFilePath + ", " + customFilePath);
    monitor->monitorRuleApplication("saveBaseAndCustomRules", baseFilePath + " и " + customFilePath, true);
    dbManager->logDBOperation("Сохранение базовых и пользовательских правил", "Правила сохранены в файлы: " + baseFilePath + ", " + customFilePath);
    return true;
}

bool RuleEngine::saveRulesToCloud(const std::string& cloudPath) const {
    std::shared_lock<std::shared_mutex> lock(rulesMutex);

    std::string rulesContent;
    for (const auto& rule : customRules) {
        rulesContent += rule + '\n';
    }

    std::ofstream tempFile("temp_rules.txt");
    if (!tempFile.is_open()) {
        Logger::error("Не удалось создать временный файл для сохранения правил.");
        monitor->monitorRuleApplication("saveRulesToCloud", cloudPath, false);
        dbManager->logDBError("Не удалось создать временный файл для сохранения правил.");
        return false;
    }
    tempFile << rulesContent;
    tempFile.close();

    bool uploadSuccess = cloudIntegration->uploadData("temp_rules.txt", cloudPath);
    if (uploadSuccess) {
        Logger::info("Правила успешно загружены в облако: " + cloudPath);
        monitor->monitorRuleApplication("saveRulesToCloud", cloudPath, true);
        dbManager->logDBOperation("Сохранение правил в облако", "Правила успешно загружены в облако: " + cloudPath);
    } else {
        Logger::error("Не удалось загрузить правила в облако: " + cloudPath);
        monitor->monitorRuleApplication("saveRulesToCloud", cloudPath, false);
        dbManager->logDBError("Не удалось загрузить правила в облако: " + cloudPath);
    }

    std::remove("temp_rules.txt");
    return uploadSuccess;
}

bool RuleEngine::addRule(const std::string& rule) {
    std::unique_lock<std::shared_mutex> lock(rulesMutex);
    if (validateRule(rule) && !containsRule(rule)) {
        customRules.push_back(rule);
        ruleUsageCount[rule] = 0;
        Logger::info("Добавлено новое правило: " + rule);
        monitor->monitorRuleApplication("addRule", rule, true);
        dbManager->logDBOperation("Добавление правила", "Добавлено новое правило: " + rule);
        return true;
    }
    Logger::warning("Не удалось добавить правило: " + rule);
    monitor->monitorRuleApplication("addRule", rule, false);
    dbManager->logDBError("Не удалось добавить правило: " + rule);
    return false;
}

bool RuleEngine::removeRule(const std::string& rule) {
    std::unique_lock<std::shared_mutex> lock(rulesMutex);
    auto it = std::find(customRules.begin(), customRules.end(), rule);
    if (it != customRules.end()) {
        customRules.erase(it);
        ruleUsageCount.erase(rule);
        Logger::info("Удалено правило: " + rule);
        monitor->monitorRuleApplication("removeRule", rule, true);
        dbManager->logDBOperation("Удаление правила", "Удалено правило: " + rule);
        return true;
    }
    Logger::warning("Не удалось удалить правило: " + rule);
    monitor->monitorRuleApplication("removeRule", rule, false);
    dbManager->logDBError("Не удалось удалить правило: " + rule);
    return false;
}

bool RuleEngine::containsRule(const std::string& rule) const {
    std::shared_lock<std::shared_mutex> lock(rulesMutex);
    return std::find(customRules.begin(), customRules.end(), rule) != customRules.end();
}

std::optional<std::string> RuleEngine::getRuleByIndex(size_t index) const {
    std::shared_lock<std::shared_mutex> lock(rulesMutex);
    if (index < baseRules.size()) {
        return baseRules[index];
    } else if (index < baseRules.size() + customRules.size()) {
        return customRules[index - baseRules.size()];
    }
    return std::nullopt;
}

void RuleEngine::clearRules() {
    std::unique_lock<std::shared_mutex> lock(rulesMutex);
    baseRules.clear();
    customRules.clear();
    ruleUsageCount.clear();
    Logger::info("Все правила очищены");
    monitor->monitorRuleApplication("clearRules", "all_rules", true);
    dbManager->logDBOperation("Очистка правил", "Все правила очищены");
}

void RuleEngine::evaluatePerformance(const std::string& word) const {
    for (const auto& rule : baseRules) {
        auto start = std::chrono::high_resolution_clock::now();
        applyRule(word, rule);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        Logger::info("Время выполнения для правила " + rule + ": " + std::to_string(elapsed.count()) + " секунд");
        dbManager->logQueryPerformance("Применение правила: " + rule, elapsed);
    }
    for (const auto& rule : customRules) {
        auto start = std::chrono::high_resolution_clock::now();
        applyRule(word, rule);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        Logger::info("Время выполнения для правила " + rule + ": " + std::to_string(elapsed.count()) + " секунд");
        dbManager->logQueryPerformance("Применение правила: " + rule, elapsed);
    }
}

std::string RuleEngine::transformToUpper(const std::string& word) const {
    std::string result = word;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string RuleEngine::transformToLower(const std::string& word) const {
    std::string result = word;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string RuleEngine::reverseWord(const std::string& word) const {
    std::string result = word;
    std::reverse(result.begin(), result.end());
    return result;
}

std::string RuleEngine::insertCharacters(const std::string& word, const std::string& characters, size_t position) const {
    std::string result = word;
    if (position <= result.size()) {
        result.insert(position, characters);
    }
    return result;
}

std::string RuleEngine::replaceCharacters(const std::string& word, const std::string& pattern, const std::string& replacement) const {
    std::string result = word;
    size_t pos = 0;
    while ((pos = result.find(pattern, pos)) != std::string::npos) {
        result.replace(pos, pattern.length(), replacement);
        pos += replacement.length();
    }
    return result;
}

std::string RuleEngine::regexReplaceCharacters(const std::string& word, const std::string& pattern, const std::string& replacement) const {
    std::regex re(pattern);
    return std::regex_replace(word, re, replacement);
}

std::string RuleEngine::duplicateCharacters(const std::string& word, size_t times) const {
    std::string result;
    for (char ch : word) {
        result.append(times, ch);
    }
    return result;
}

std::string RuleEngine::removeCharacters(const std::string& word, const std::string& characters) const {
    std::string result = word;
    for (char ch : characters) {
        result.erase(std::remove(result.begin(), result.end(), ch), result.end());
    }
    return result;
}

std::string RuleEngine::caesarCipher(const std::string& word, int shift) const {
    std::string result = word;
    for (char& ch : result) {
        if (std::isalpha(ch)) {
            char base = std::islower(ch) ? 'a' : 'A';
            ch = (ch - base + shift) % 26 + base;
        }
    }
    return result;
}

std::string RuleEngine::randomCase(const std::string& word) const {
    std::string result = word;
    for (char& ch : result) {
        if (std::isalpha(ch)) {
            ch = (rand() % 2) ? std::toupper(ch) : std::tolower(ch);
        }
    }
    return result;
}

std::string RuleEngine::complexReplace(const std::string& word, const std::unordered_map<char, char>& replacements) const {
    std::string result = word;
    for (char& ch : result) {
        auto it = replacements.find(ch);
        if (it != replacements.end()) {
            ch = it->second;
        }
    }
    return result;
}

void RuleEngine::setUsageCounterCallback(const std::function<void(const std::string&)>& callback) {
    usageCounterCallback = callback;
}

std::vector<std::string> RuleEngine::getRules() const {
    std::shared_lock<std::shared_mutex> lock(rulesMutex);
    std::vector<std::string> allRules = baseRules;
    allRules.insert(allRules.end(), customRules.begin(), customRules.end());
    return allRules;
}
















