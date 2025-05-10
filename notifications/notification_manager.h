#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unordered_set>

#include "config/config.h"
#include "logging/logger.h"
#include "utils/notification_utils.h"

class NotificationManager {
public:
    NotificationManager(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger);
    ~NotificationManager();

    void init(const std::string &configFile);

    bool sendEmail(const std::string &recipient, const std::string &subject, const std::string &body);
    bool sendSMS(const std::string &phoneNumber, const std::string &message);
    bool sendPushNotification(const std::string &deviceToken, const std::string &message);
    bool sendMessengerNotification(const std::string &recipient, const std::string &message);

    bool createTemplate(const std::string &templateName, const std::string &templateContent);
    bool editTemplate(const std::string &templateName, const std::string &newContent);
    bool deleteTemplate(const std::string &templateName);
    std::string getTemplate(const std::string &templateName);

    void addTrigger(const std::string &event, const std::string &templateName);
    void removeTrigger(const std::string &event);

    void logNotification(const std::string &recipient, const std::string &message, bool success);

    void createNotification(const std::string &notificationName, const std::string &content);
    void deleteNotification(const std::string &notificationName);
    std::vector<std::string> listNotifications() const;

private:
    std::shared_ptr<Config> config;
    std::shared_ptr<Logger> logger;
    std::unique_ptr<NotificationUtils> notificationUtils;

    std::map<std::string, std::string> templates;
    std::map<std::string, std::string> triggers;
    std::map<std::string, std::string> notifications;

    void loadConfig(const std::string &configFile);
    void logAction(const std::string &action, const std::string &details);

    bool isEmailValid(const std::string &email);
    bool isPhoneNumberValid(const std::string &phoneNumber);
    bool isTriggerValid(const std::string &event);

    void processTriggers(const std::string &event);
};

#endif // NOTIFICATION_MANAGER_H
