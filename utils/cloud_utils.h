#ifndef CLOUD_UTILS_H
#define CLOUD_UTILS_H

#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>
#include "../config/config.h"
#include "../logging/logger.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class CloudUtils {
public:
    CloudUtils(const std::shared_ptr<Config>& config, const std::shared_ptr<Logger>& logger);
    bool uploadFile(const std::string& filePath, const std::string& cloudPath);
    bool downloadFile(const std::string& cloudPath, const std::string& localPath);
    bool deleteFile(const std::string& cloudPath);
    std::unordered_map<std::string, std::string> listFiles(const std::string& directory);

private:
    bool authenticate();
    std::string getAuthToken();
    void logEvent(const std::string& message, const std::string& level = "INFO");
    bool sendRequest(const std::string& method, const std::string& url, const std::string& body, std::string* response = nullptr);
    bool retryRequest(const std::string& method, const std::string& url, const std::string& body, std::string* response = nullptr, int retryCount = 3);
    void handleCurlError(CURLcode res);
    static size_t writeCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);
    static size_t readCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);

    bool uploadFileStream(const std::string& filePath, const std::string& cloudPath);
    bool downloadFileStream(const std::string& cloudPath, const std::string& localPath);

    std::shared_ptr<Config> config;
    std::shared_ptr<Logger> logger;
    std::string authToken;
    std::chrono::time_point<std::chrono::system_clock> tokenExpiryTime;
};

#endif // CLOUD_UTILS_H


