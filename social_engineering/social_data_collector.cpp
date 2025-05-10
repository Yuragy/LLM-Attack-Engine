#include "social_data_collector.h"
#include <thread>
#include <future>
#include <unordered_set>
#include <iostream>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out) {
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

SocialDataCollector::SocialDataCollector(std::shared_ptr<Logger> logger, std::shared_ptr<DBManager> dbManager, std::shared_ptr<NotificationManager> notificationManager)
    : logger(logger), dbManager(dbManager), notificationManager(notificationManager) {
    dictionaryGenerator = std::make_shared<PersonalizedDictionaryGenerator>(logger, dbManager);
}

SocialDataCollector::~SocialDataCollector() {
}

void SocialDataCollector::startDataCollection(const std::string& target) {
    logDataCollectionStart(target);

    std::vector<std::string> collectedData;

    try {
        auto socialMediaFuture = std::async(std::launch::async, &SocialDataCollector::collectFromSocialMedia, this, target);
        auto publicRecordsFuture = std::async(std::launch::async, &SocialDataCollector::collectFromPublicRecords, this, target);
        auto newsArticlesFuture = std::async(std::launch::async, &SocialDataCollector::collectFromNewsArticles, this, target);
        collectedData = socialMediaFuture.get();
        auto publicRecordsData = publicRecordsFuture.get();
        collectedData.insert(collectedData.end(), publicRecordsData.begin(), publicRecordsData.end());
        auto newsArticlesData = newsArticlesFuture.get();
        collectedData.insert(collectedData.end(), newsArticlesData.begin(), newsArticlesData.end());
    } catch (const std::exception& e) {
        handleError("Error during data collection: " + std::string(e.what()), "Data Collection");
    }
    std::unordered_set<std::string> uniqueData(collectedData.begin(), collectedData.end());
    collectedData.assign(uniqueData.begin(), uniqueData.end());

    logger->log("Saving collected data to the database.", LogLevel::INFO);
    try {
        dbManager->beginTransaction();
        for (const auto& data : collectedData) {
            if (!data.empty()) {
                std::string query = "INSERT INTO social_data (target, data) VALUES (?, ?)";
                dbManager->executeParameterizedQuery(query, {target, data});
            }
        }
        dbManager->commitTransaction();
    } catch (const std::exception& e) {
        dbManager->rollbackTransaction();
        handleError("Failed to save data to the database: " + std::string(e.what()), "Database Save");
    }

    if (!collectedData.empty()) {
        logger->log("Data collection for target: " + target + " completed successfully.", LogLevel::INFO);
        notificationManager->sendNotification("Data collection completed", "Data collection for " + target + " has been completed.");
    } else {
        logger->log("Data collection for target: " + target + " failed. No data was collected.", LogLevel::ERROR);
        notificationManager->sendNotification("Data collection failed", "Data collection for " + target + " has failed.");
    }

    logDataCollectionEnd(target);
}

std::vector<std::string> SocialDataCollector::generatePersonalizedDictionary() {
    logDictionaryGenerationStart();

    std::vector<std::string> dictionary;

    try {
        dictionary = dictionaryGenerator->generateDictionary();
        dbManager->saveDictionary(dictionary);
    } catch (const std::exception& e) {
        handleError("Failed to generate personalized dictionary: " + std::string(e.what()), "Dictionary Generation");
    }

    logDictionaryGenerationEnd(dictionary);
    return dictionary;
}

std::vector<std::string> SocialDataCollector::collectFromSocialMedia(const std::string& target) {
    if (dataCache.find("social_" + target) != dataCache.end()) {
        return dataCache["social_" + target];
    }

    logger->log("Collecting data from social media for target: " + target, LogLevel::INFO);
    std::vector<std::string> socialMediaData;

    try {
        std::string url = "https://api."media".com/user/" + target + "/posts";
        std::string authHeader = "Bearer your_token_here";
        std::string response = performHttpRequest(url, authHeader);

        json jsonData = json::parse(response);
        for (const auto& post : jsonData["posts"]) {
            socialMediaData.push_back(post.get<std::string>());
        }
    } catch (const std::exception& e) {
        handleError("Error collecting data from social media: " + std::string(e.what()), "Social Media Collection");
    }

    dataCache["social_" + target] = socialMediaData;
    return socialMediaData;
}

std::vector<std::string> SocialDataCollector::collectFromPublicRecords(const std::string& target) {
    if (dataCache.find("public_" + target) != dataCache.end()) {
        return dataCache["public_" + target];
    }

    logger->log("Collecting data from public records for target: " + target, LogLevel::INFO);
    std::vector<std::string> publicRecordsData;

    try {
        std::string url = "https://api.publicrecords.com/search?query=" + target;
        std::string response = performHttpRequest(url);

        json jsonData = json::parse(response);
        for (const auto& record : jsonData["records"]) {
            publicRecordsData.push_back(record.get<std::string>());
        }
    } catch (const std::exception& e) {
        handleError("Error collecting data from public records: " + std::string(e.what()), "Public Records Collection");
    }

    dataCache["public_" + target] = publicRecordsData;
    return publicRecordsData;
}

std::vector<std::string> SocialDataCollector::collectFromNewsArticles(const std::string& target) {
    if (dataCache.find("news_" + target) != dataCache.end()) {
        return dataCache["news_" + target];
    }

    logger->log("Collecting data from news articles for target: " + target, LogLevel::INFO);
    std::vector<std::string> newsArticlesData;

    try {
        std::string url = "https://newsapi.org/v2/everything?q=" + target + "&apiKey=API_KEY";
        std::string response = performHttpRequest(url);

        json jsonData = json::parse(response);
        for (const auto& article : jsonData["articles"]) {
            newsArticlesData.push_back(article["title"].get<std::string>());
        }
    } catch (const std::exception& e) {
        handleError("Error collecting data from news articles: " + std::string(e.what()), "News Articles Collection");
    }

    dataCache["news_" + target] = newsArticlesData;
    return newsArticlesData;
}

void SocialDataCollector::logDataCollectionStart(const std::string& target) {
    logger->log("Starting data collection for target: " + target, LogLevel::INFO);
    notificationManager->sendNotification("Data collection started", "Data collection for " + target + " has started.");
}

void SocialDataCollector::logDataCollectionEnd(const std::string& target) {
    logger->log("Finished data collection for target: " + target, LogLevel::INFO);
    notificationManager->sendNotification("Data collection finished", "Data collection for " + target + " has finished.");
}

void SocialDataCollector::logDictionaryGenerationStart() {
    logger->log("Starting personalized dictionary generation.", LogLevel::INFO);
    notificationManager->sendNotification("Dictionary generation started", "Personalized dictionary generation has started.");
}

void SocialDataCollector::logDictionaryGenerationEnd(const std::vector<std::string>& dictionary) {
    logger->log("Finished personalized dictionary generation. Dictionary size: " + std::to_string(dictionary.size()), LogLevel::INFO);
    notificationManager->sendNotification("Dictionary generation finished", "Personalized dictionary generation has finished. Dictionary size: " + std::to_string(dictionary.size()));
}

void SocialDataCollector::handleError(const std::string& message, const std::string& context) {
    logger->log("Error in context: " + context + " - " + message, LogLevel::ERROR);
    notificationManager->sendNotification("Error", "An error occurred in " + context + ": " + message);
}

std::string SocialDataCollector::performHttpRequest(const std::string& url, const std::string& authHeader) {
    CURL* curl = curl_easy_init();
    std::string response;
    int maxRetries = 3;
    int retryCount = 0;
    int backoff = 1000;

    while (retryCount < maxRetries) {
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            
            if (!authHeader.empty()) {
                struct curl_slist* headers = nullptr;
                headers = curl_slist_append(headers, authHeader.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            }

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                curl_easy_cleanup(curl);
                return response;
            } else {
                handleError("CURL error: " + std::string(curl_easy_strerror(res)), "HTTP Request");
            }
        } else {
            handleError("Failed to initialize CURL", "HTTP Request");
        }
        
        retryCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(backoff));
        backoff *= 2;
    }

    curl_easy_cleanup(curl);
    return "";
}



