#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <json/json.h>
#include <yaml-cpp/yaml.h>

bool Config::load(const std::string& filePath) {
    bool success = false;
    if (filePath.ends_with(".json")) {
        success = loadFromJson(filePath);
    } else if (filePath.ends_with(".yaml") || filePath.ends_with(".yml")) {
        success = loadFromYaml(filePath);
    } else {
        Logger::error("Unknown config file format: " + filePath);
        return false;
    }

    if (success) {
        Logger::info("Configuration loaded successfully from: " + filePath);
    }

    return success;
}

bool Config::save(const std::string& filePath) {
    bool success = false;
    if (filePath.ends_with(".json")) {
        success = saveToJson(filePath);
    } else if (filePath.ends_with(".yaml") || filePath.ends_with(".yml")) {
        success = saveToYaml(filePath);
    } else {
        Logger::error("Unknown config file format: " + filePath);
        return false;
    }

    if (success) {
        Logger::info("Configuration saved successfully to: " + filePath);
    }

    return success;
}

std::optional<Config::ConfigValue> Config::get(const std::string& key) const {
    if (auto cachedValue = getCached(key); cachedValue.has_value()) {
        return cachedValue;
    }
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Config::set(const std::string& key, const Config::ConfigValue& value) {
    auto oldValue = get(key);
    configMap[key] = value;
    cache[key] = value; 
    if (oldValue) {
        notifyChange(key, *oldValue, value);
        logSystemParameterChange(key, *oldValue, value); 
    }
}

std::string Config::getDBConnectionString() const {
    auto value = get("db_connection_string");
    if (value) {
        return std::get<std::string>(*value);
    }
    return "";
}

std::vector<std::string> Config::getDictionaryPaths() const {
    std::vector<std::string> paths;
    for (const auto& pair : configMap) {
        if (pair.first.find("dictionary_path") == 0) {
            paths.push_back(std::get<std::string>(pair.second));
        }
    }
    return paths;
}

std::string Config::getRulesPath() const {
    auto value = get("rules_path");
    if (value) {
        return std::get<std::string>(*value);
    }
    return "";
}

std::map<std::string, Config::ConfigValue> Config::getAttackConfig() const {
    return getSection("attack");
}

std::map<std::string, Config::ConfigValue> Config::getMonitoringConfig() const {
    return getSection("monitoring");
}

std::map<std::string, Config::ConfigValue> Config::getSchedulerConfig() const {
    return getSection("scheduler");
}

std::map<std::string, Config::ConfigValue> Config::getWebAppConfig() const {
    return getSection("webapp");
}

std::map<std::string, Config::ConfigValue> Config::getCloudConfig() const {
    return getSection("cloud");
}

std::map<std::string, Config::ConfigValue> Config::getUserManagementConfig() const {
    return getSection("user_management");
}

std::map<std::string, Config::ConfigValue> Config::getNotificationConfig() const {
    return getSection("notification");
}

std::map<std::string, Config::ConfigValue> Config::getAnalyticsConfig() const {
    return getSection("analytics");
}

std::map<std::string, Config::ConfigValue> Config::getRuleConfig() const {
    return getSection("rule");
}

std::map<std::string, Config::ConfigValue> Config::getGPUConfig() const {
    return getSection("gpu");
}

std::map<std::string, Config::ConfigValue> Config::getMLConfig() const {
    return getSection("ml");
}

std::map<std::string, Config::ConfigValue> Config::getLogConfig() const {
    return getSection("log");
}

std::map<std::string, Config::ConfigValue> Config::getDataUtilsConfig() const {
    return getSection("data_utils");
}

std::map<std::string, Config::ConfigValue> Config::getThreadingConfig() const {
    return getSection("threading");
}

std::map<std::string, Config::ConfigValue> Config::getDBManagerConfig() const {
    return getSection("db_manager");
}

std::map<std::string, Config::ConfigValue> Config::getAuthConfig() const {
    return getSection("auth");
}

std::map<std::string, Config::ConfigValue> Config::getAPIConfig() const {
    return getSection("api");
}

std::map<std::string, Config::ConfigValue> Config::getSocialEngineeringConfig() const {
    return getSection("social_engineering");
}

std::map<std::string, Config::ConfigValue> Config::getMachineLearningConfig() const {
    return getSection("machine_learning");
}

std::map<std::string, Config::ConfigValue> Config::getAdaptiveAttackConfig() const {
    return getSection("adaptive_attack");
}

std::map<std::string, Config::ConfigValue> Config::getRecoveryConfig() const {
    return getSection("recovery");
}

std::map<std::string, Config::ConfigValue> Config::getUtilsConfig() const {
    return getSection("utils");
}

std::map<std::string, Config::ConfigValue> Config::getPolicyConfig() const {
    return getSection("policy");
}

std::map<std::string, Config::ConfigValue> Config::getNotificationsConfig() const {
    return getSection("notifications");
}

std::map<std::string, Config::ConfigValue> Config::getTargetsConfig() const {
    return getSection("targets");
}

void Config::setRuleConfig(const std::map<std::string, Config::ConfigValue>& ruleConfig) {
    setSection("rule", ruleConfig);
}

bool Config::validate() const {
    const std::vector<std::string> requiredKeys = {
        "db_connection_string",
        "dictionary_path",
        "rules_path",
        "attack.some_required_param",
        "monitoring.some_required_param",
        "scheduler.some_required_param",
        "webapp.some_required_param",
        "cloud.some_required_param",
        "user_management.some_required_param",
        "notification.some_required_param",
        "analytics.some_required_param",
        "rule.some_required_param",
        "gpu.some_required_param",
        "ml.some_required_param",
        "log.some_required_param"
    };

    for (const auto& key : requiredKeys) {
        if (!get(key).has_value()) {
            Logger::error("Missing required configuration key: " + key);
            return false;
        }
    }

    auto validateString = [](const std::string& value) {
        return !value.empty();
    };

    auto validateInt = [](int value, int min, int max) {
        return value >= min && value <= max;
    };

    auto validateDouble = [](double value, double min, double max) {
        return value >= min && value <= max.
    };

    auto validateBool = [](bool value) {
        return value == true || value == false;
    };

    const std::map<std::string, std::function<bool(ConfigValue)>> formatValidators = {
        {"db_connection_string", [](ConfigValue value) { return validateString(std::get<std::string>(value)); }},
        {"rules_path", [](ConfigValue value) { return validateString(std::get<std::string>(value)); }},
        {"attack.thread_count", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 64); }},
        {"monitoring.frequency", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 60); }},
        {"scheduler.max_tasks", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 100); }},
        {"webapp.port", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 65535); }},
        {"cloud.timeout", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 600); }},
        {"user_management.max_users", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 1000); }},
        {"notification.enabled", [](ConfigValue value) { return validateBool(std::get<bool>(value)); }},
        {"analytics.reporting_interval", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 1440); }},
        {"rule.max_rules", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 1000); }},
        {"gpu.max_memory_usage", [](ConfigValue value) { return validateDouble(std::get<double>(value), 0.0, 1.0); }},
        {"gpu.monitoring_enabled", [](ConfigValue value) { return validateBool(std::get<bool>(value)); }},
        {"ml.training_iterations", [](ConfigValue value) { return validateInt(std::get<int>(value), 1, 10000); }},
        {"log.level", [](ConfigValue value) { return validateString(std::get<std::string>(value)); }}
    };

    for (const auto& [key, validator] : formatValidators) {
        auto value = get(key);
        if (value && !validator(*value)) {
            Logger::error("Invalid format for configuration key: " + key);
            return false;
        }
    }

    return true;
}

std::string Config::toString() const {
    std::ostringstream oss;
    for (const auto& [key, value] : configMap) {
        oss << key << ": ";
        std::visit([&oss](auto&& arg) {
            oss << arg;
        }, value);
        oss << "\n";
    }
    return oss.str();
}

std::map<std::string, Config::ConfigValue> Config::getSection(const std::string& section) const {
    std::map<std::string, ConfigValue> sectionConfig;
    for (const auto& [key, value] : configMap) {
        if (key.find(section) == 0) {
            sectionConfig[key] = value;
        }
    }
    return sectionConfig;
}

void Config::setSection(const std::string& section, const std::map<std::string, Config::ConfigValue>& sectionConfig) {
    for (const auto& [key, value] : sectionConfig) {
        set(section + "." + key, value);
    }
}

std::map<std::string, std::map<std::string, Config::ConfigValue>> Config::getNestedConfig() const {
    std::map<std::string, std::map<std::string, Config::ConfigValue>> nestedConfig;
    for (const auto& [key, value] : configMap) {
        auto dotPos = key.find('.');
        if (dotPos != std::string::npos) {
            std::string section = key.substr(0, dotPos);
            std::string subKey = key.substr(dotPos + 1);
            nestedConfig[section][subKey] = value;
        }
    }
    return nestedConfig;
}

void Config::logChanges(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue) const {
    std::string oldValueStr = std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>)
            return arg;
        else
            return std::to_string(arg);
    }, oldValue);

    std::string newValueStr = std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>)
            return arg;
        else
            return std::to_string(arg);
    }, newValue);

    Logger::info("Config key '" + key + "' changed from " + oldValueStr + " to " + newValueStr);
}

void Config::logSystemParameterChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue) const {
    logChanges(key, oldValue, newValue);
}

void Config::logSystemStatus(const std::string& status) const {
    Logger::info("System status: " + status);
}

void Config::overrideFromEnvironment() {
    applyEnvironmentOverrides();
}

void Config::enableCaching(bool enable) {
    if (!enable) {
        cache.clear();
    }
}

std::optional<Config::ConfigValue> Config::getCached(const std::string& key) const {
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Config::registerChangeCallback(const ConfigChangeCallback& callback) {
    changeCallbacks.push_back(callback);
}

bool Config::loadFromJson(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        Logger::error("Failed to open config file: " + filePath);
        return false;
    }
    
    Json::Value root;
    file >> root;
    
    for (const auto& key : root.getMemberNames()) {
        const auto& value = root[key];
        if (value.isString()) {
            configMap[key] = value.asString();
        } else if (value.isInt()) {
            configMap[key] = value.asInt();
        } else if (value.isDouble()) {
            configMap[key] = value.asDouble();
        } else if (value.isBool()) {
            configMap[key] = value.asBool();
        }
    }
    
    return true;
}

bool Config::saveToJson(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        Logger::error("Failed to open config file: " + filePath);
        return false;
    }
    
    Json::Value root;
    for (const auto& [key, value] : configMap) {
        std::visit([&root, &key](auto&& arg) {
            root[key] = arg;
        }, value);
    }
    
    file << root;
    return true;
}

bool Config::loadFromYaml(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        Logger::error("Failed to open config file: " + filePath);
        return false;
    }
    
    YAML::Node root = YAML::LoadFile(filePath);
    
    for (const auto& key : root) {
        const auto& value = root[key];
        if (value.IsScalar()) {
            configMap[key.as<std::string>()] = value.as<std::string>();
        } else if (value.IsSequence()) {
            for (const auto& item : value) {
                configMap[key.as<std::string>()] = item.as<std::string>();
            }
        } else if (value.IsMap()) {
            for (const auto& subKey : value) {
                configMap[key.as<std::string>() + "." + subKey.first.as<std::string>()] = subKey.second.as<std::string>();
            }
        }
    }
    
    return true;
}

bool Config::saveToYaml(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        Logger::error("Failed to open config file: " + filePath);
        return false;
    }
    
    YAML::Emitter out;
    out << YAML::BeginMap;
    for (const auto& [key, value] : configMap) {
        out << YAML::Key << key;
        std::visit([&out](auto&& arg) {
            out << YAML::Value << arg;
        }, value);
    }
    out << YAML::EndMap;
    
    file << out.c_str();
    return true;
}

std::optional<std::string> Config::getEnv(const std::string& varName) const {
    const char* val = std::getenv(varName.c_str());
    if (val == nullptr) {
        return std::nullopt;
    }
    return std::string(val);
}

void Config::applyEnvironmentOverrides() {
    for (auto& [key, value] : configMap) {
        auto envVar = getEnv(key);
        if (envVar) {
            if (std::holds_alternative<std::string>(value)) {
                set(key, *envVar);
            } else if (std::holds_alternative<int>(value)) {
                set(key, std::stoi(*envVar));
            } else if (std::holds_alternative<double>(value)) {
                set(key, std::stod(*envVar));
            } else if (std::holds_alternative<bool>(value)) {
                set(key, (*envVar == "true") ? true : false);
            }
        }
    }
}

void Config::notifyChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue) {
    for (const auto& callback : changeCallbacks) {
        callback(key, oldValue, newValue);
    }
}

bool Config::reloadConfig(const std::string& filePath) {
    if (load(filePath)) {
        for (const auto& [key, value] : configMap) {
            notifyChange(key, ConfigValue{}, value);
        }
        Logger::info("Configuration reloaded from: " + filePath);
        return true;
    }
    Logger::error("Failed to reload configuration from: " + filePath);
    return false;
}

void Config::setSystemParameter(const std::string& key, const Config::ConfigValue& value) {
    auto oldValue = get(key);
    set(key, value);
    logSystemParameterChange(key, oldValue.value_or("undefined"), value);
}

std::optional<Config::ConfigValue> Config::getSystemParameter(const std::string& key) const {
    return get(key);
}

std::string Config::getSystemStatus() const {
    std::ostringstream status;
    status << "Состояние системы:\n";
    status << "База данных: " << (getDBConnectionString().empty() ? "Не подключено" : "Подключено") << "\n";
    status << "Словари: " << (getDictionaryPaths().empty() ? "Не загружены" : "Загружены") << "\n";
    status << "Правила: " << (getRulesPath().empty() ? "Не загружены" : "Загружены") << "\n";
    std::string statusStr = status.str();
    logSystemStatus(statusStr);
    return statusStr;
}



















