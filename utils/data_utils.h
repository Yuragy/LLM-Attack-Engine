#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <stdexcept>

#include "../logging/logger.h"
#include "../config/config.h"

class DataUtils {
public:
    DataUtils(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config);
    nlohmann::json parseJSON(const std::string& jsonString);
    std::string serializeJSON(const nlohmann::json& jsonObject);
    bool validateJSON(const std::string& jsonString, const std::string& schemaString);
    std::vector<std::unordered_map<std::string, std::string>> parseCSV(const std::string& csvString);
    std::string serializeCSV(const std::vector<std::unordered_map<std::string, std::string>>& data);
    bool validateCSV(const std::string& csvString, const std::vector<std::string>& headers);
    nlohmann::json parseXML(const std::string& xmlString);
    std::string serializeXML(const nlohmann::json& jsonObject);
    bool validateXML(const std::string& xmlString, const std::string& schemaString);
    YAML::Node parseYAML(const std::string& yamlString);
    std::string serializeYAML(const YAML::Node& yamlObject);
    bool validateYAML(const std::string& yamlString, const std::string& schemaString);

private:
    nlohmann::json processXMLNode(const pugi::xml_node& node);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);

    std::shared_ptr<Logger> logger;
    std::shared_ptr<Config> config;
};

#endif // DATA_UTILS_H


