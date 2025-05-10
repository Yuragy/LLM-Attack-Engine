#include "cloud_utils.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

CloudUtils::CloudUtils(const std::shared_ptr<Config>& config, const std::shared_ptr<Logger>& logger)
    : config(config), logger(logger) {
    if (!authenticate()) {
        logEvent("Failed to authenticate with cloud service", "ERROR");
        throw std::runtime_error("Authentication failed");
    }
}

bool CloudUtils::uploadFile(const std::string& filePath, const std::string& cloudPath) {
    return uploadFileStream(filePath, cloudPath);
}

bool CloudUtils::downloadFile(const std::string& cloudPath, const std::string& localPath) {
    return downloadFileStream(cloudPath, localPath);
}

bool CloudUtils::deleteFile(const std::string& cloudPath) {
    std::string url = config->getCloudApiUrl() + "/delete";
    std::string body = "cloudPath=" + cloudPath;

    bool success = retryRequest("POST", url, body);
    logEvent("Deleting file: " + cloudPath, success ? "INFO" : "ERROR");
    return success;
}

std::unordered_map<std::string, std::string> CloudUtils::listFiles(const std::string& directory) {
    std::string url = config->getCloudApiUrl() + "/list";
    std::string body = "directory=" + directory;
    std::string response;

    bool success = retryRequest("POST", url, body, &response);
    logEvent("Listing files in directory: " + directory, success ? "INFO" : "ERROR");

    std::unordered_map<std::string, std::string> files;
    if (success) {
        try {
            auto jsonResponse = nlohmann::json::parse(response);
            for (const auto& item : jsonResponse.items()) {
                files[item.key()] = item.value().get<std::string>();
            }
        } catch (const std::exception& e) {
            logEvent("Failed to parse file list response: " + std::string(e.what()), "ERROR");
        }
    } else {
        logEvent("Failed to list files in directory: " + directory, "ERROR");
    }
    return files;
}

bool CloudUtils::authenticate() {
    std::string url = config->getCloudApiUrl() + "/auth";
    std::string body = "username=" + config->getCloudUsername() + "&password=" + config->getCloudPassword();

    std::string response;
    bool success = sendRequest("POST", url, body, &response);
    if (success) {
        try {
            auto jsonResponse = nlohmann::json::parse(response);
            if (jsonResponse.contains("token")) {
                authToken = jsonResponse.at("token").get<std::string>();
                if (jsonResponse.contains("expires_in")) {
                    int expiresIn = jsonResponse.at("expires_in").get<int>();
                    tokenExpiryTime = std::chrono::system_clock::now() + std::chrono::seconds(expiresIn);
                } else {
                    tokenExpiryTime = std::chrono::system_clock::now() + std::chrono::hours(1); // Default expiry time
                }
                logEvent("Authenticated successfully with cloud service");
            } else {
                logEvent("Authentication failed: token not found in response", "ERROR");
                return false;
            }
        } catch (const std::exception& e) {
            logEvent("Failed to parse authentication response: " + std::string(e.what()), "ERROR");
            return false;
        }
    } else {
        logEvent("Authentication request failed", "ERROR");
    }
    return success;
}

std::string CloudUtils::getAuthToken() {
    if (authToken.empty() || std::chrono::system_clock::now() >= tokenExpiryTime) {
        if (!authenticate()) {
            throw std::runtime_error("Failed to re-authenticate with cloud service");
        }
    }
    return authToken;
}

void CloudUtils::logEvent(const std::string& message, const std::string& level) {
    logger->log("[" + level + "] " + message);
}

size_t CloudUtils::writeCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

size_t CloudUtils::readCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fread(ptr, size, nmemb, stream);
}

bool CloudUtils::uploadFileStream(const std::string& filePath, const std::string& cloudPath) {
    std::string url = config->getCloudApiUrl() + "/upload_stream";
    
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        logEvent("Failed to open file for uploading: " + filePath, "ERROR");
        return false;
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + getAuthToken()).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, file);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            handleCurlError(res);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            fclose(file);
            return false;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    } else {
        logEvent("Failed to initialize cURL for file upload", "ERROR");
        fclose(file);
        return false;
    }

    fclose(file);
    logEvent("File uploaded successfully: " + cloudPath);
    return true;
}

bool CloudUtils::downloadFileStream(const std::string& cloudPath, const std::string& localPath) {
    std::string url = config->getCloudApiUrl() + "/download_stream";
    
    FILE* file = fopen(localPath.c_str(), "wb");
    if (!file) {
        logEvent("Failed to open file for downloading: " + localPath, "ERROR");
        return false;
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + getAuthToken()).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            handleCurlError(res);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            fclose(file);
            return false;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    } else {
        logEvent("Failed to initialize cURL for file download", "ERROR");
        fclose(file);
        return false;
    }

    fclose(file);
    logEvent("File downloaded successfully: " + cloudPath);
    return true;
}

bool CloudUtils::sendRequest(const std::string& method, const std::string& url, const std::string& body, std::string* response) {
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + getAuthToken()).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
        }
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

        if (response) {
            std::ostringstream responseStream;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                std::ostringstream* stream = static_cast<std::ostringstream*>(userdata);
                size_t totalSize = size * nmemb;
                stream->write(static_cast<char*>(ptr), totalSize);
                return totalSize;
            });
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStream);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                *response = responseStream.str();
            }
        } else {
            res = curl_easy_perform(curl);
        }

        if (res != CURLE_OK) {
            handleCurlError(res);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return true;
}

bool CloudUtils::retryRequest(const std::string& method, const std::string& url, const std::string& body, std::string* response, int retryCount) {
    int attempt = 0;
    int delay = 2;
    while (attempt < retryCount) {
        if (sendRequest(method, url, body, response)) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        delay *= 2; 
        attempt++;
    }
    return false;
}

void CloudUtils::handleCurlError(CURLcode res) {
    std::string errorMessage = "CURL error: " + std::string(curl_easy_strerror(res));
    logEvent(errorMessage, "ERROR");
}



