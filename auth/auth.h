#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "../database/db_manager.h"
#include "../logging/logger.h"
#include "../config/config.h"
#include "../notification/notification_manager.h"
#include "../cloud/cloud_integration.h"
#include "../utils/threading_utils.h"

class Auth {
public:
    Auth(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger, std::shared_ptr<NotificationManager> notificationManager, std::shared_ptr<ThreadingUtils> threadingUtils);
    bool login(const std::string& username, const std::string& password);
    bool logout(const std::string& sessionId);
    bool checkSession(const std::string& sessionId);
    bool enable2FA(const std::string& username);
    bool verify2FA(const std::string& username, const std::string& code);
    bool socialLogin(const std::string& provider, const std::string& token);
    std::string generate2FAToken(const std::string& username);
    bool verify2FAToken(const std::string& username, const std::string& token);
    std::string generatePasswordRecoveryToken(const std::string& username);
    bool verifyPasswordRecoveryToken(const std::string& username, const std::string& token);
    bool setUserRole(const std::string& username, const std::string& role);
    std::string getUserRole(const std::string& username);
    std::vector<std::string> getUserPermissions(const std::string& role);
    bool updateConfig(const std::string& key, const Config::ConfigValue& value);
private:
    std::string generateSessionId();
    std::string hashPassword(const std::string& password);
    void logEvent(const std::string& event, LogLevel level = LogLevel::INFO);
    std::unique_ptr<DBManager> dbManager;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<Config> config;
    std::shared_ptr<NotificationManager> notificationManager; 
    std::shared_ptr<ThreadingUtils> threadingUtils; 
    std::shared_ptr<CloudIntegration> cloudIntegration;  
};

#endif // AUTH_H





