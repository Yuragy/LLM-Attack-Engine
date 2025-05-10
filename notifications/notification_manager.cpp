#include "notifications/notification_manager.h"
#include "config/config.h"
#include "logging/logger.h"
#include "utils/notification_utils.h"

#include <iostream>
#include <fstream>
#include <ctime>
#include <regex>

NotificationManager::NotificationManager(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger)
    : config(config), logger(logger) {
    notificationUtils = std::make_unique<NotificationUtils>(config, logger);
}

NotificationManager::~NotificationManager() {
    notificationUtils.reset();
    logger->info("NotificationManager is being destroyed and resources are cleaned up.");
}

void NotificationManager::init(const std::string &configFile) {
    loadConfig(configFile);
    logger->info("NotificationManager initialized with config file: " + configFile);
}

void NotificationManager::loadConfig(const std::string &configFile) {
    if (config->loadFromFile(configFile)) {
        logger->info("Config file loaded successfully: " + configFile);
    } else {
        logger->error("Unable to load config file: " + configFile);
        throw std::runtime_error("Failed to load config file");
    }
}

bool NotificationManager::sendEmail(const std::string &recipient, const std::string &subject, const std::string &body) {
    if (!isEmailValid(recipient)) {
        logger->error("Invalid email address: " + recipient);
        return false;
    }
    bool success = notificationUtils->sendEmail(recipient, subject, body);
    logNotification(recipient, body, success);
    return success;
}

bool NotificationManager::sendSMS(const std::string &phoneNumber, const std::string &message) {
    if (!isPhoneNumberValid(phoneNumber)) {
        logger->error("Invalid phone number: " + phoneNumber);
        return false;
    }
    bool success = notificationUtils->sendSMS(phoneNumber, message);
    logNotification(phoneNumber, message, success);
    return success;
}

bool NotificationManager::sendPushNotification(const std::string &deviceToken, const std::string &message) {
    if (deviceToken.empty() || deviceToken.size() < 10) { 
        logger->error("Invalid device token: " + deviceToken);
        return false;
    }
    bool success = notificationUtils->sendPushNotification(deviceToken, message);
    logNotification(deviceToken, message, success);
    return success;
}

bool NotificationManager::sendMessengerNotification(const std::string &recipient, const std::string &message) {
    bool success = notificationUtils->sendMessengerNotification(recipient, message);
    logNotification(recipient, message, success);
    return success;
}

bool NotificationManager::createTemplate(const std::string &templateName, const std::string &templateContent) {
    if (templates.find(templateName) != templates.end()) {
        logger->error("Template already exists: " + templateName);
        return false;
    }
    templates[templateName] = templateContent;
    notificationUtils->setTemplate(templateName, templateContent);
    logger->info("Template " + templateName + " created.");
    return true;
}

bool NotificationManager::editTemplate(const std::string &templateName, const std::string &newContent) {
    if (templates.find(templateName) == templates.end()) {
        logger->error("Template not found: " + templateName);
        return false;
    }
    templates[templateName] = newContent;
    notificationUtils->setTemplate(templateName, newContent);
    logger->info("Template " + templateName + " edited.");
    return true;
}

bool NotificationManager::deleteTemplate(const std::string &templateName) {
    if (templates.find(templateName) == templates.end()) {
        logger->error("Template not found: " + templateName);
        return false;
    }
    templates.erase(templateName);
    logger->info("Template " + templateName + " deleted.");
    return true;
}

std::string NotificationManager::getTemplate(const std::string &templateName) {
    if (templates.find(templateName) == templates.end()) {
        logger->error("Template not found: " + templateName);
        return "";
    }
    return templates[templateName];
}

void NotificationManager::addTrigger(const std::string &event, const std::string &templateName) {
    if (templates.find(templateName) == templates.end()) {
        logger->error("Template not found: " + templateName);
        return;
    }
    if (!isTriggerValid(event)) {
        logger->error("Invalid event: " + event);
        return;
    }
    triggers[event] = templateName;
    logger->info("Trigger for event " + event + " added with template " + templateName);
}

void NotificationManager::removeTrigger(const std::string &event) {
    if (triggers.find(event) == triggers.end()) {
        logger->error("Trigger not found for event: " + event);
        return;
    }
    triggers.erase(event);
    logger->info("Trigger for event " + event + " removed.");
}

void NotificationManager::logNotification(const std::string &recipient, const std::string &message, bool success) {
    std::string status = success ? "Success" : "Failure";
    logger->log("Notification to " + recipient + ": " + message + " - " + status, success ? LogLevel::INFO : LogLevel::ERROR);
}

void NotificationManager::createNotification(const std::string &notificationName, const std::string &content) {
    if (notifications.find(notificationName) != notifications.end()) {
        logger->error("Notification already exists: " + notificationName);
        return;
    }
    notifications[notificationName] = content;
    logAction("Create Notification", "Name: " + notificationName + ", Content: " + content);
    logger->info("Notification " + notificationName + " created.");
}

void NotificationManager::deleteNotification(const std::string &notificationName) {
    if (notifications.find(notificationName) == notifications.end()) {
        logger->error("Notification not found: " + notificationName);
        return;
    }
    notifications.erase(notificationName);
    logAction("Delete Notification", "Name: " + notificationName);
    logger->info("Notification " + notificationName + " deleted.");
}

std::vector<std::string> NotificationManager::listNotifications() const {
    std::vector<std::string> notificationList;
    for (const auto &pair : notifications) {
        notificationList.push_back(pair.first);
    }
    return notificationList;
}

void NotificationManager::logAction(const std::string &action, const std::string &details) {
    logger->log(action + ": " + details, LogLevel::INFO);
}

bool NotificationManager::isEmailValid(const std::string &email) {
    const std::regex pattern(R"((\w+)(\.{0,1})(\w*)@(\w+)\.(\w+))");
    return std::regex_match(email, pattern);
}

bool NotificationManager::isPhoneNumberValid(const std::string &phoneNumber) {
    const std::regex pattern(R"(\+?\d{10,13})");
    return std::regex_match(phoneNumber, pattern);
}

bool NotificationManager::isTriggerValid(const std::string &event) {
    static const std::unordered_set<std::string> validEvents = {
        "user_login", "user_logout", "system_error", "task_completed", "user_registration", "password_reset"
    };
    return validEvents.find(event) != validEvents.end();
}

void NotificationManager::processTriggers(const std::string &event) {
    auto it = triggers.find(event);
    if (it != triggers.end()) {
        std::string templateName = it->second;
        std::string message = getTemplate(templateName);
        
        if (event == "user_login" || event == "user_logout") {
            sendEmail("admin", "Event Triggered: " + event, message);
        } else if (event == "system_error") {
            sendSMS("", "Critical system error occurred: " + message);
        } else if (event == "task_completed") {
            sendPushNotification("device_token", "Task completed: " + message);
        } else {
            logger->warning("Unhandled event: " + event);
        }
    }
}




