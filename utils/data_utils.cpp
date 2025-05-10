#include "data_utils.h"
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json-schema.hpp>
#include <pugixml.hpp>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

DataUtils::DataUtils(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config)
    : logger(logger), config(config) {}

nlohmann::json DataUtils::parseJSON(const std::string& jsonString) {
    try {
        logger->log("Parsing JSON data");
        return nlohmann::json::parse(jsonString);
    } catch (const std::exception& e) {
        logger->error("Failed to parse JSON: " + std::string(e.what()));
        throw;
    }
}

std::string DataUtils::serializeJSON(const nlohmann::json& jsonObject) {
    try {
        logger->log("Serializing JSON data");
        if (jsonObject.is_null()) {
            throw std::runtime_error("Cannot serialize null JSON object");
        }
        return jsonObject.dump();
    } catch (const std::exception& e) {
        logger->error("Failed to serialize JSON: " + std::string(e.what()));
        throw;
    }
}

bool DataUtils::validateJSON(const std::string& jsonString, const std::string& schemaString) {
    try {
        logger->log("Validating JSON data against schema");
        auto json = parseJSON(jsonString);
        auto schema = nlohmann::json::parse(schemaString);
        nlohmann::json_schema::json_validator validator(schema);
        validator.validate(json);
        logger->log("JSON validation successful");
        return true;
    } catch (const std::exception& e) {
        logger->error("JSON validation failed: " + std::string(e.what()));
        return false;
    }
}
std::vector<std::unordered_map<std::string, std::string>> DataUtils::parseCSV(const std::string& csvString) {
    std::vector<std::unordered_map<std::string, std::string>> data;
    std::istringstream sstream(csvString);
    std::string line;
    std::vector<std::string> headers;

    try {
        logger->log("Parsing CSV data");
        if (std::getline(sstream, line)) {
            headers = split(line, ',');
        }

        while (std::getline(sstream, line)) {
            if (line.empty()) continue;
            std::unordered_map<std::string, std::string> row;
            auto values = split(line, ',');
            for (size_t i = 0; i < headers.size(); ++i) {
                row[headers[i]] = values[i];
            }
            data.push_back(row);
        }
    } catch (const std::exception& e) {
        logger->error("Failed to parse CSV: " + std::string(e.what()));
        throw;
    }

    return data;
}

std::string DataUtils::serializeCSV(const std::vector<std::unordered_map<std::string, std::string>>& data) {
    std::ostringstream sstream;

    try {
        logger->log("Serializing CSV data");
        if (!data.empty()) {
            for (const auto& header : data[0]) {
                sstream << header.first << ",";
            }
            sstream.seekp(-1, sstream.cur);
            sstream << "\n";

            for (const auto& row : data) {
                for (const auto& field : row) {
                    sstream << (field.second.empty() ? "\"\"" : field.second) << ",";
                }
                sstream.seekp(-1, sstream.cur);
                sstream << "\n";
            }
        } else {
            logger->log("No data provided for CSV serialization");
        }
    } catch (const std::exception& e) {
        logger->error("Failed to serialize CSV: " + std::string(e.what()));
        throw;
    }

    return sstream.str();
}

bool DataUtils::validateCSV(const std::string& csvString, const std::vector<std::string>& headers) {
    try {
        logger->log("Validating CSV data against headers");
        auto data = parseCSV(csvString);
        if (data.empty()) {
            logger->error("CSV validation failed: No data found");
            return false;
        }

        for (const auto& header : headers) {
            if (data[0].find(header) == data[0].end()) {
                logger->error("CSV validation failed: Missing header " + header);
                return false;
            }
        }
        logger->log("CSV validation successful");
        return true;
    } catch (const std::exception& e) {
        logger->error("CSV validation failed: " + std::string(e.what()));
        return false;
    }
}

nlohmann::json DataUtils::parseXML(const std::string& xmlString) {
    try {
        logger->log("Parsing XML data");
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xmlString.c_str());
        if (!result) {
            throw std::runtime_error("Failed to parse XML: " + std::string(result.description()));
        }
        nlohmann::json jsonObj;
        for (auto& node : doc.children()) {
            jsonObj[node.name()] = processXMLNode(node);
        }
        return jsonObj;
    } catch (const std::exception& e) {
        logger->error("Failed to parse XML: " + std::string(e.what()));
        throw;
    }
}

nlohmann::json DataUtils::processXMLNode(const pugi::xml_node& node) {
    nlohmann::json jsonObj;
    
    for (auto& attr : node.attributes()) {
        jsonObj["@attributes"][attr.name()] = attr.value();
    }

    for (auto& child : node.children()) {
        if (child.type() == pugi::node_element) {
            jsonObj[child.name()] = processXMLNode(child);
        } else if (child.type() == pugi::node_pcdata) {
            jsonObj["_text"] = child.value();
        }
    }

    return jsonObj;
}

std::string DataUtils::serializeXML(const nlohmann::json& jsonObject) {
    try {
        logger->log("Serializing JSON to XML");
        pugi::xml_document doc;
        for (auto& item : jsonObject.items()) {
            pugi::xml_node node = doc.append_child(item.key().c_str());
            if (item.value().is_object() && item.value().contains("@attributes")) {
                for (auto& attr : item.value()["@attributes"].items()) {
                    node.append_attribute(attr.key().c_str()) = attr.value().get<std::string>().c_str();
                }
            }
            if (item.value().is_object() && item.value().contains("_text")) {
                node.append_child(pugi::node_pcdata).set_value(item.value()["_text"].get<std::string>().c_str());
            } else if (item.value().is_object()) {
                for (auto& child : item.value().items()) {
                    if (child.key() != "@attributes" && child.key() != "_text") {
                        node.append_child(child.key().c_str()).append_child(pugi::node_pcdata).set_value(child.value().dump().c_str());
                    }
                }
            }
        }
        std::ostringstream sstream;
        doc.save(sstream);
        return sstream.str();
    } catch (const std::exception& e) {
        logger->error("Failed to serialize XML: " + std::string(e.what()));
        throw;
    }
}

bool DataUtils::validateXML(const std::string& xmlString, const std::string& schemaString) {
    try {
        logger->log("Validating XML data against schema");

        xmlSchemaParserCtxtPtr ctxt = xmlSchemaNewMemParserCtxt(schemaString.c_str(), schemaString.size());
        xmlSchemaPtr schema = xmlSchemaParse(ctxt);
        xmlSchemaValidCtxtPtr validCtxt = xmlSchemaNewValidCtxt(schema);

        xmlDocPtr doc = xmlReadMemory(xmlString.c_str(), xmlString.size(), NULL, NULL, 0);
        if (doc == NULL) {
            logger->error("Failed to parse XML");
            return false;
        }
        int ret = xmlSchemaValidateDoc(validCtxt, doc);
        xmlSchemaFreeValidCtxt(validCtxt);
        xmlFreeDoc(doc);
        xmlSchemaFree(schema);
        xmlSchemaCleanupTypes();

        if (ret == 0) {
            logger->log("XML validation successful");
            return true;
        } else {
            logger->error("XML validation failed");
            return false;
        }
    } catch (const std::exception& e) {
        logger->error("XML validation failed: " + std::string(e.what()));
        return false;
    }
}

YAML::Node DataUtils::parseYAML(const std::string& yamlString) {
    try {
        logger->log("Parsing YAML data");
        return YAML::Load(yamlString);
    } catch (const std::exception& e) {
        logger->error("Failed to parse YAML: " + std::string(e.what()));
        throw;
    }
}

std::string DataUtils::serializeYAML(const YAML::Node& yamlObject) {
    try {
        logger->log("Serializing YAML data");
        YAML::Emitter out;
        out << yamlObject;
        return out.c_str();
    } catch (const std::exception& e) {
        logger->error("Failed to serialize YAML: " + std::string(e.what()));
        throw;
    }
}

bool DataUtils::validateYAML(const std::string& yamlString, const std::string& schemaString) {
    try {
        logger->log("Validating YAML data against schema");
        auto yaml = parseYAML(yamlString);
        auto schema = parseYAML(schemaString);

        for (const auto& item : schema) {
            if (!yaml[item.first.as<std::string>()]) {
                logger->error("YAML validation failed: Missing key " + item.first.as<std::string>());
                return false;
            }
        }

        logger->log("YAML validation successful");
        return true;
    } catch (const std::exception& e) {
        logger->error("YAML validation failed: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> DataUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

std::string DataUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}



