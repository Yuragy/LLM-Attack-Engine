#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <chrono>
#include <mutex>
#include "../logging/logger.h"
#include "../config/config.h"
#include "../notifications/notification_manager.h"
#include "../database/db_manager.h"
#include "../auth/auth.h"  
#include "../policy/policy_manager.h"
#include "../rules/rule_engine.h"

class UserManagement {
public:
    struct UserProfile {
        std::string username;
        std::string email;
        std::string role;
        bool isActive;
        std::string createdAt;
        std::string lastLogin;
        bool isMFAEnabled;
    };

    UserManagement(const Config& config, Logger& logger, NotificationManager& notifier, DBManager& dbManager, Auth& auth, PolicyManager& policyManager, RuleEngine& ruleEngine);
    ~UserManagement();

    bool addUser(const UserProfile& profile);
    bool removeUser(const std::string& username);
    std::optional<UserProfile> getUser(const std::string& username) const;
    bool updateUser(const UserProfile& profile);
    std::vector<UserProfile> getAllUsers() const;
    bool deactivateUser(const std::string& username);
    bool activateUser(const std::string& username);


    bool login(const std::string& username, const std::string& password);
    bool logout(const std::string& sessionId);
    bool checkSession(const std::string& sessionId);

    bool enable2FA(const std::string& username);
    bool disable2FA(const std::string& username);
    bool validate2FA(const std::string& username, const std::string& token);

    std::string initiatePasswordRecovery(const std::string& username);
    bool completePasswordRecovery(const std::string& username, const std::string& recoveryToken, const std::string& newPassword);
    void addUserPolicy(const std::string& username, const std::string& policyName);
    void removeUserPolicy(const std::string& username, const std::string& policyName);
    bool checkUserPolicyCompliance(const std::string& username) const;

    bool createRole(const std::string& roleName);
    bool deleteRole(const std::string& roleName);
    bool updateRole(const std::string& roleName, const std::vector<std::string>& permissions);
    std::vector<std::string> getAllRoles() const;

    bool endAllSessions(const std::string& username);
    std::vector<std::string> getActiveSessions(const std::string& username) const;

    std::string exportUsersToCSV() const;
    bool importUsersFromCSV(const std::string& csvData);

private:
    std::unordered_map<std::string, UserProfile> userDatabase;
    const Config& config;
    Logger& logger;
    NotificationManager& notifier;
    DBManager& dbManager;
    Auth& auth;  
    PolicyManager& policyManager;
    RuleEngine& ruleEngine; 

    int maxUsers;
    bool require2FA;

    bool userExists(const std::string& username) const;
    void notifyUserChange(const std::string& username, const std::string& action);
    void logError(const std::string& message) const;
    void logInfo(const std::string& message) const;
};

#endif // USER_MANAGEMENT_H








