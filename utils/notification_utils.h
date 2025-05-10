#ifndef NOTIFICATION_UTILS_H
#define NOTIFICATION_UTILS_H

#include <string>
#include <memory>
#include <map>
#include <future>
#include "../config/config.h"
#include "../logging/logger.h"

class NotificationUtils {
public:
    NotificationUtils(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger);

    bool sendEmail(const std::string& to, const std::string& subject, const std::string& message);
    bool sendSMS(const std::string& to, const std::string& message);
    bool sendPushNotification(const std::string& to, const std::string& message);
    bool sendMessengerNotification(const std::string& to, const std::string& message);

    void setTemplate(const std::string& type, const std::string& templateContent);
    void logNotification(const std::string& type, const std::string& to, const std::string& message);

    std::future<bool> sendEmailAsync(const std::string& to, const std::string& subject, const std::string& message);
    std::future<bool> sendSMSAsync(const std::string& to, const std::string& message);
    std::future<bool> sendPushNotificationAsync(const std::string& to, const std::string& message);
    std::future<bool> sendMessengerNotificationAsync(const std::string& to, const std::string& message);

private:
    std::shared_ptr<Config> config;
    std::shared_ptr<Logger> logger;
    std::map<std::string, std::string> templates;

    bool send(const std::string& type, const std::string& to, const std::string& subject, const std::string& message);
    std::string getEmailServer() const;
    std::string getSMSServer() const;
    std::string getPushServer() const;
    std::string getMessengerServer() const;
    void logError(const std::string& type, const std::string& message) const;
    void logSuccess(const std::string& type, const std::string& message) const;
    std::string applyTemplate(const std::string& type, const std::string& message) const;

    bool validateEmail(const std::string& email) const;
    bool validatePhoneNumber(const std::string& phoneNumber) const;

    static constexpr int RETRY_LIMIT = 3;
    static constexpr int TIMEOUT = 10;
};

#endif // NOTIFICATION_UTILS_H



