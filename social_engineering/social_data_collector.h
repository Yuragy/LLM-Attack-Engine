#ifndef SOCIAL_DATA_COLLECTOR_H
#define SOCIAL_DATA_COLLECTOR_H

#include <string>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include <unordered_map>
#include "../logging/logger.h"
#include "../database/db_manager.h"
#include "../notifications/notification_manager.h"
#include "personalized_dictionary_generator.h"
#include "json.hpp"

using json = nlohmann::json;

class SocialDataCollector {
public:
    SocialDataCollector(std::shared_ptr<Logger> logger, std::shared_ptr<DBManager> dbManager, std::shared_ptr<NotificationManager> notificationManager);
    ~SocialDataCollector();
    void startDataCollection(const std::string& target);
    std::vector<std::string> generatePersonalizedDictionary();

private:
    std::vector<std::string> collectFromSocialMedia(const std::string& target);
    std::vector<std::string> collectFromPublicRecords(const std::string& target);
    std::vector<std::string> collectFromNewsArticles(const std::string& target);
    void logDataCollectionStart(const std::string& target);
    void logDataCollectionEnd(const std::string& target);
    void logDictionaryGenerationStart();
    void logDictionaryGenerationEnd(const std::vector<std::string>& dictionary);
    void handleError(const std::string& message, const std::string& context);
    std::string performHttpRequest(const std::string& url, const std::string& authHeader = "");
    std::shared_ptr<Logger> logger;
    std::shared_ptr<DBManager> dbManager;
    std::shared_ptr<NotificationManager> notificationManager;
    std::shared_ptr<PersonalizedDictionaryGenerator> dictionaryGenerator;
    std::unordered_map<std::string, std::vector<std::string>> dataCache;
};

#endif // SOCIAL_DATA_COLLECTOR_H
