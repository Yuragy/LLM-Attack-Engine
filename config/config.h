#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <functional>
#include "logging/logger.h" 

class Config {
public:
    using ConfigValue = std::variant<std::string, int, double, bool>;
    using ConfigChangeCallback = std::function<void(const std::string&, const ConfigValue&, const ConfigValue&)>;
    bool load(const std::string& filePath);
    bool save(const std::string& filePath);
    std::optional<ConfigValue> get(const std::string& key) const;
    void set(const std::string& key, const ConfigValue& value);
    std::string getDBConnectionString() const;
    std::vector<std::string> getDictionaryPaths() const;
    std::string getRulesPath() const;
    std::map<std::string, ConfigValue> getAttackConfig() const;
    std::map<std::string, ConfigValue> getMonitoringConfig() const;
    std::map<std::string, ConfigValue> getSchedulerConfig() const;
    std::map<std::string, ConfigValue> getWebAppConfig() const;
    std::map<std::string, ConfigValue> getCloudConfig() const;
    std::map<std::string, ConfigValue> getUserManagementConfig() const;
    std::map<std::string, ConfigValue> getNotificationConfig() const;
    std::map<std::string, ConfigValue> getAnalyticsConfig() const;
    std::map<std::string, ConfigValue> getRuleConfig() const;
    std::map<std::string, ConfigValue> getGPUConfig() const;
    std::map<std::string, ConfigValue> getMLConfig() const;
    std::map<std::string, ConfigValue> getLogConfig() const;
    std::map<std::string, ConfigValue> getDataUtilsConfig() const;
    std::map<std::string, ConfigValue> getThreadingConfig() const;
    std::map<std::string, ConfigValue> getDBManagerConfig() const;
    std::map<std::string, ConfigValue> getAuthConfig() const;
    std::map<std::string, ConfigValue> getAPIConfig() const;
    std::map<std::string, ConfigValue> getSocialEngineeringConfig() const;
    std::map<std::string, ConfigValue> getMachineLearningConfig() const;
    std::map<std::string, ConfigValue> getAdaptiveAttackConfig() const;
    std::map<std::string, ConfigValue> getRecoveryConfig() const;
    std::map<std::string, ConfigValue> getUtilsConfig() const;
    std::map<std::string, ConfigValue> getPolicyConfig() const;
    std::map<std::string, ConfigValue> getNotificationsConfig() const;
    std::map<std::string, ConfigValue> getTargetsConfig() const;
    bool reloadConfig(const std::string& filePath);
    void setRuleConfig(const std::map<std::string, ConfigValue>& ruleConfig);
    bool validate() const;
    std::string toString() const;
    std::map<std::string, ConfigValue> getSection(const std::string& section) const;
    void setSection(const std::string& section, const std::map<std::string, ConfigValue>& sectionConfig);
    std::map<std::string, std::map<std::string, ConfigValue>> getNestedConfig() const;
    void logChanges(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue) const;
    void overrideFromEnvironment();
    void enableCaching(bool enable);
    std::optional<ConfigValue> getCached(const std::string& key) const;
    void registerChangeCallback(const ConfigChangeCallback& callback);
    void setSystemParameter(const std::string& key, const ConfigValue& value);
    std::optional<ConfigValue> getSystemParameter(const std::string& key) const;
    std::string getSystemStatus() const;
    void logSystemParameterChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue) const;
    void logSystemStatus(const std::string& status) const;

private:
    std::map<std::string, ConfigValue> configMap;
    std::map<std::string, ConfigValue> cache;
    std::vector<ConfigChangeCallback> changeCallbacks;
    bool loadFromJson(const std::string& filePath);
    bool saveToJson(const std::string& filePath);
    bool loadFromYaml(const std::string& filePath);
    bool saveToYaml(const std::string& filePath);
    std::optional<std::string> getEnv(const std::string& varName) const;
    void applyEnvironmentOverrides();
    void notifyChange(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue);
};

#endif // CONFIG_H




