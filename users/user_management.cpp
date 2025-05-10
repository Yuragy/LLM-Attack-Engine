#include "user_management.h"
#include "../auth/auth.h"
#include "../policy/policy_manager.h"
#include "../rules/rule_engine.h"

UserManagement::UserManagement(const Config& config, Logger& logger, NotificationManager& notifier, DBManager& dbManager, Auth& auth, PolicyManager& policyManager, RuleEngine& ruleEngine)
    : config(config), logger(logger), notifier(notifier), dbManager(dbManager), auth(auth), policyManager(policyManager), ruleEngine(ruleEngine) {

    auto userConfig = config.getUserManagementConfig();
    maxUsers = std::get<int>(userConfig.at("max_users"));
    require2FA = std::get<bool>(userConfig.at("require_2fa"));

    if (!validateUserManagementConfig()) {
        logError("Invalid user management configuration");
        throw std::runtime_error("Invalid user management configuration");
    }

    auto users = dbManager.fetchData("SELECT * FROM users");
    for (const auto& user : users) {
        userDatabase[user.username] = user;
    }
}
UserManagement::~UserManagement() {
}
bool UserManagement::login(const std::string& username, const std::string& password) {
    return auth.login(username, password);
}
bool UserManagement::logout(const std::string& sessionId) {
    return auth.logout(sessionId);
}
bool UserManagement::checkSession(const std::string& sessionId) {
    return auth.checkSession(sessionId);
}
bool UserManagement::enable2FA(const std::string& username) {
    return auth.enable2FA(username);
}
bool UserManagement::disable2FA(const std::string& username) {
    return auth.verify2FAToken(username, "");
}

bool UserManagement::validate2FA(const std::string& username, const std::string& token) {
    return auth.verify2FA(username, token);
}
std::string UserManagement::initiatePasswordRecovery(const std::string& username) {
    return auth.generatePasswordRecoveryToken(username);
}

bool UserManagement::completePasswordRecovery(const std::string& username, const std::string& recoveryToken, const std::string& newPassword) {
    return auth.verifyPasswordRecoveryToken(username, recoveryToken);
}
void UserManagement::addUserPolicy(const std::string& username, const std::string& policyName) {
    if (!userExists(username)) {
        logError("User not found: " + username);
        return;
    }

    if (ruleEngine.addRule(policyName)) {
        logInfo("Policy " + policyName + " added for user: " + username);
    } else {
        logError("Failed to add policy " + policyName + " for user: " + username);
    }
}

void UserManagement::removeUserPolicy(const std::string& username, const std::string& policyName) {
    if (!userExists(username)) {
        logError("User not found: " + username);
        return;
    }

    if (ruleEngine.removeRule(policyName)) {
        logInfo("Policy removed for user: " + username);
    } else {
        logError("Failed to remove policy for user: " + username);
    }
}

bool UserManagement::checkUserPolicyCompliance(const std::string& username) const {
    if (!userExists(username)) {
        logError("User not found: " + username);
        return false;
    }

    std::vector<std::string> appliedRules = ruleEngine.applyRules(username);
    return !appliedRules.empty();
}
bool UserManagement::addUser(const UserProfile& profile) {
    std::lock_guard<std::mutex> lock(userMutex);
    if (userExists(profile.username)) {
        logError("User already exists: " + profile.username);
        return false;
    }

    if (userDatabase.size() >= maxUsers) {
        logError("User limit exceeded");
        return false;
    }

    std::string query = "INSERT INTO users (username, email, role, isActive, isMFAEnabled) VALUES ('" + profile.username + "', '" +
                         profile.email + "', '" + profile.role + "', " + (profile.isActive ? "1" : "0") + ", " + (profile.isMFAEnabled ? "1" : "0") + ")";
    if (!dbManager.executeQuery(query)) {
        logError("Failed to add user to database: " + profile.username);
        return false;
    }

    userDatabase[profile.username] = profile;
    logInfo("User added: " + profile.username);
    notifyUserChange(profile.username, "added");

    return true;
}

bool UserManagement::removeUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(userMutex);
    if (!userExists(username)) {
        logError("User not found: " + username);
        return false;
    }

    std::string query = "DELETE FROM users WHERE username = '" + username + "'";
    if (!dbManager.executeQuery(query)) {
        logError("Failed to remove user from database: " + username);
        return false;
    }

    userDatabase.erase(username);
    logInfo("User removed: " + username);
    notifyUserChange(username, "removed");

    return true;
}

std::optional<UserManagement::UserProfile> UserManagement::getUser(const std::string& username) const {
    std::lock_guard<std::mutex> lock(userMutex);
    if (!userExists(username)) {
        logError("User not found: " + username);
        return std::nullopt;
    }
    return userDatabase.at(username);
}

bool UserManagement::updateUser(const UserProfile& profile) {
    std::lock_guard<std::mutex> lock(userMutex);
    if (!userExists(profile.username)) {
        logError("User not found: " + profile.username);
        return false;
    }

    std::string query = "UPDATE users SET email = '" + profile.email + "', role = '" + profile.role +
                         "', isActive = " + (profile.isActive ? "1" : "0") + ", isMFAEnabled = " + (profile.isMFAEnabled ? "1" : "0") +
                         " WHERE username = '" + profile.username + "'";
    if (!dbManager.executeQuery(query)) {
        logError("Failed to update user in database: " + profile.username);
        return false;
    }

    userDatabase[profile.username] = profile;
    logInfo("User updated: " + profile.username);
    notifyUserChange(profile.username, "updated");

    return true;
}

std::vector<UserManagement::UserProfile> UserManagement::getAllUsers() const {
    std::lock_guard<std::mutex> lock(userMutex);
    auto users = dbManager.fetchData("SELECT * FROM users");
    std::vector<UserProfile> userList;
    for (const auto& user : users) {
        userList.push_back(user);
    }
    return userList;
}

bool UserManagement::deactivateUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(userMutex);
    if (!userExists(username)) {
        logError("User not found: " + username);
        return false;
    }

    std::string query = "UPDATE users SET isActive = 0 WHERE username = '" + username + "'";
    if (!dbManager.executeQuery(query)) {
        logError("Failed to deactivate user in database: " + username);
        return false;
    }

    userDatabase[username].isActive = false;
    logInfo("User deactivated: " + username);
    notifyUserChange(username, "deactivated");

    return true;
}

bool UserManagement::activateUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(userMutex);
    if (!userExists(username)) {
        logError("User not found: " + username);
        return false;
    }

    std::string query = "UPDATE users SET isActive = 1 WHERE username = '" + username + "'";
    if (!dbManager.executeQuery(query)) {
        logError("Failed to activate user in database: " + username);
        return false;
    }

    userDatabase[username].isActive = true;
    logInfo("User activated: " + username);
    notifyUserChange(username, "activated");

    return true;
}
bool UserManagement::createRole(const std::string& roleName) {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.createRole(roleName);
}

bool UserManagement::deleteRole(const std::string& roleName) {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.deleteRole(roleName);
}

bool UserManagement::updateRole(const std::string& roleName, const std::vector<std::string>& permissions) {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.updateRole(roleName, permissions);
}

std::vector<std::string> UserManagement::getAllRoles() const {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.getAllRoles();
}

bool UserManagement::endAllSessions(const std::string& username) {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.endAllSessions(username);
}

std::vector<std::string> UserManagement::getActiveSessions(const std::string& username) const {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.getActiveSessions(username);
}
std::string UserManagement::exportUsersToCSV() const {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.exportUsersToCSV();
}

bool UserManagement::importUsersFromCSV(const std::string& csvData) {
    std::lock_guard<std::mutex> lock(userMutex);
    return dbManager.importUsersFromCSV(csvData);
}












