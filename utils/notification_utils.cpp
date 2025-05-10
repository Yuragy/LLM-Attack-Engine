#include "notification_utils.h"
#include <iostream>
#include <curl/curl.h>
#include <regex>
#include <chrono>
#include <thread>

NotificationUtils::NotificationUtils(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger)
    : config(config), logger(logger) {}

bool NotificationUtils::sendEmail(const std::string& to, const std::string& subject, const std::string& message) {
    if (!validateEmail(to)) {
        logError("Email", "Invalid email address: " + to);
        return false;
    }
    bool result = send("email", to, subject, applyTemplate("email", message));
    logNotification("Email", to, message);
    return result;
}
bool NotificationUtils::sendSMS(const std::string& to, const std::string& message) {
    if (!validatePhoneNumber(to)) {
        logError("SMS", "Invalid phone number: " + to);
        return false;
    }
    bool result = send("sms", to, "", applyTemplate("sms", message));
    logNotification("SMS", to, message);
    return result;
}
bool NotificationUtils::sendPushNotification(const std::string& to, const std::string& message) {
    bool result = send("push", to, "", applyTemplate("push", message));
    logNotification("Push", to, message);
    return result;
}
bool NotificationUtils::sendMessengerNotification(const std::string& to, const std::string& message) {
    bool result = send("messenger", to, "", applyTemplate("messenger", message));
    logNotification("Messenger", to, message);
    return result;
}
std::future<bool> NotificationUtils::sendEmailAsync(const std::string& to, const std::string& subject, const std::string& message) {
    return std::async(std::launch::async, &NotificationUtils::sendEmail, this, to, subject, message);
}
std::future<bool> NotificationUtils::sendSMSAsync(const std::string& to, const std::string& message) {
    return std::async(std::launch::async, &NotificationUtils::sendSMS, this, to, message);
}
std::future<bool> NotificationUtils::sendPushNotificationAsync(const std::string& to, const std::string& message) {
    return std::async(std::launch::async, &NotificationUtils::sendPushNotification, this, to, message);
}
std::future<bool> NotificationUtils::sendMessengerNotificationAsync(const std::string& to, const std::string& message) {
    return std::async(std::launch::async, &NotificationUtils::sendMessengerNotification, this, to, message);
}
void NotificationUtils::setTemplate(const std::string& type, const std::string& templateContent) {
    templates[type] = templateContent;
}
void NotificationUtils::logNotification(const std::string& type, const std::string& to, const std::string& message) {
    logger->log("Sending " + type + " to " + to + ": " + message);
}
bool NotificationUtils::send(const std::string& type, const std::string& to, const std::string& subject, const std::string& message) {
    int attempt = 0;
    int delay = 1;
    while (attempt < RETRY_LIMIT) {
        try {
            if (type == "email") {
                std::string emailServer = getEmailServer();
                CURL *curl;
                CURLcode res = CURLE_OK;
                curl = curl_easy_init();
                if (curl) {
                    curl_easy_setopt(curl, CURLOPT_USERNAME, config->get("email_user").c_str());
                    curl_easy_setopt(curl, CURLOPT_PASSWORD, config->get("email_password").c_str());
                    curl_easy_setopt(curl, CURLOPT_URL, emailServer.c_str());
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);

                    struct curl_slist *recipients = NULL;
                    recipients = curl_slist_append(recipients, to.c_str());
                    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

                    std::string payload = "To: " + to + "\r\n" +
                                          "Subject: " + subject + "\r\n" +
                                          "\r\n" + message;

                    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
                    curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
                    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

                    res = curl_easy_perform(curl);

                    if (res != CURLE_OK) {
                        logError("Email", curl_easy_strerror(res));
                        curl_slist_free_all(recipients);
                        curl_easy_cleanup(curl);
                        attempt++;
                        std::this_thread::sleep_for(std::chrono::seconds(delay));
                        delay *= 2;
                        continue;
                    }

                    curl_slist_free_all(recipients);
                    curl_easy_cleanup(curl);
                }
                logSuccess("Email", "Email sent to " + to);
                return true;
            } else if (type == "sms") {
                std::string smsServer = getSMSServer();
                CURL *curl;
                CURLcode res = CURLE_OK;
                curl = curl_easy_init();
                if (curl) {
                    std::string url = smsServer + "?apikey=" + config->get("sms_api_key") + "&to=" + to + "&message=" + message;
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);

                    res = curl_easy_perform(curl);

                    if (res != CURLE_OK) {
                        logError("SMS", curl_easy_strerror(res));
                        curl_easy_cleanup(curl);
                        attempt++;
                        std::this_thread::sleep_for(std::chrono::seconds(delay));
                        delay *= 2;
                        continue;
                    }

                    curl_easy_cleanup(curl);
                }
                logSuccess("SMS", "SMS sent to " + to);
                return true;
            } else if (type == "push") {
                std::string pushServer = getPushServer();
                CURL *curl;
                CURLcode res = CURLE_OK;
                curl = curl_easy_init();
                if (curl) {
                    std::string url = pushServer + "/send";
                    std::string payload = "{\"to\":\"" + to + "\",\"message\":\"" + message + "\",\"token\":\"" + config->get("push_token") + "\"}";
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);

                    res = curl_easy_perform(curl);

                    if (res != CURLE_OK) {
                        logError("Push", curl_easy_strerror(res));
                        curl_easy_cleanup(curl);
                        attempt++;
                        std::this_thread::sleep_for(std::chrono::seconds(delay));
                        delay *= 2;
                        continue;
                    }

                    curl_easy_cleanup(curl);
                }
                logSuccess("Push", "Push notification sent to " + to);
                return true;
            } else if (type == "messenger") {
                std::string messengerServer = getMessengerServer();
                CURL *curl;
                CURLcode res = CURLE_OK;
                curl = curl_easy_init();
                if (curl) {
                    std::string url = messengerServer + "/send";
                    std::string payload = "{\"to\":\"" + to + "\",\"message\":\"" + message + "\",\"token\":\"" + config->get("messenger_token") + "\"}";
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);

                    res = curl_easy_perform(curl);

                    if (res != CURLE_OK) {
                        logError("Messenger", curl_easy_strerror(res));
                        curl_easy_cleanup(curl);
                        attempt++;
                        std::this_thread::sleep_for(std::chrono::seconds(delay));
                        delay *= 2;
                        continue;
                    }

                    curl_easy_cleanup(curl);
                }
                logSuccess("Messenger", "Messenger notification sent to " + to);
                return true;
            }
        } catch (const std::exception& e) {
            logError(type, e.what());
            attempt++;
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            delay *= 2;
        }
    }

    return false;
}

std::string NotificationUtils::getEmailServer() const {
    return config->get("email_server");
}
std::string NotificationUtils::getSMSServer() const {
    return config->get("sms_gateway");
}
std::string NotificationUtils::getPushServer() const {
    return config->get("push_server");
}
std::string NotificationUtils::getMessengerServer() const {
    return config->get("messenger_server");
}
void NotificationUtils::logError(const std::string& type, const std::string& message) const {
    logger->log(type + " send failed: " + message, LogLevel::ERROR);
}
void NotificationUtils::logSuccess(const std::string& type, const std::string& message) const {
    logger->log(message, LogLevel::INFO);
}
std::string NotificationUtils::applyTemplate(const std::string& type, const std::string& message) const {
    auto it = templates.find(type);
    if (it != templates.end()) {
        std::string result = it->second;
        size_t pos = result.find("{message}");
        if (pos != std::string::npos) {
            result.replace(pos, 9, message);
        }
        return result;
    }
    return message;
}
bool NotificationUtils::validateEmail(const std::string& email) const {
    const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
    return std::regex_match(email, pattern);
}
bool NotificationUtils::validatePhoneNumber(const std::string& phoneNumber) const {
    const std::regex pattern("\\+?[0-9]{7,15}");
    return std::regex_match(phoneNumber, pattern);
}



