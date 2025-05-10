#include "policy_manager.h"
#include "../logging/logger.h"
#include <fstream>

PolicyManager::PolicyManager(Config* config, DBManager* dbManager, NotificationManager* notificationManager, RuleEngine* ruleEngine)
    : config(config), dbManager(dbManager), notificationManager(notificationManager), ruleEngine(ruleEngine) {
    loadPoliciesFromConfig();
    currentPolicy = "Low Risk Policy";
}

void PolicyManager::adjustPolicy(const std::string& policyName) {
    if (policies.find(policyName) != policies.end()) {
        applyPolicy(policyName);
        currentPolicy = policyName;
        logPolicyChange(policyName);
        dbManager->logDBOperation("Adjust Policy", "Policy " + policyName + " applied");
        Logger::log("Policy " + policyName + " applied", LogLevel::INFO);

        notificationManager->sendEmail("admin@example.com", "Policy Changed", "Policy " + policyName + " has been applied.");
    } else {
        Logger::log("Policy " + policyName + " not found", LogLevel::WARNING);
        dbManager->logDBError("Policy " + policyName + " not found");
        notificationManager->sendEmail("admin@example.com", "Policy Error", "Policy " + policyName + " not found.");
    }
}

void PolicyManager::enforceRules(const std::string& username) {
    if (userPolicies.find(username) != userPolicies.end()) {
        std::string policyName = userPolicies.at(username);
        std::vector<std::string> rules = ruleEngine->applyRules(policyName);
        for (const auto& rule : rules) {
            Logger::log("Rule applied: " + rule + " for user: " + username, LogLevel::INFO);
        }
    } else {
        Logger::log("No policy found for user " + username, LogLevel::WARNING);
    }
}

std::string PolicyManager::getCurrentPolicy() const {
    return currentPolicy;
}

std::vector<std::string> PolicyManager::getAvailablePolicies() const {
    std::vector<std::string> policyNames;
    for (const auto& policy : policies) {
        policyNames.push_back(policy.first);
    }
    return policyNames;
}

std::map<std::string, std::string> PolicyManager::getPolicyDetails(const std::string& policyName) const {
    if (policies.find(policyName) != policies.end()) {
        return policies.at(policyName);
    } else {
        Logger::log("Policy " + policyName + " not found", LogLevel::WARNING);
        dbManager->logDBError("Policy " + policyName + " not found");
        notificationManager->sendEmail("admin@example.com", "Policy Error", "Policy " + policyName + " not found.");
        return {};
    }
}

void PolicyManager::loadPolicies() {
    policies["Low Risk Policy"] = {{"threshold", "low"}, {"action", "monitor"}};
    policies["Medium Risk Policy"] = {{"threshold", "medium"}, {"action", "alert"}};
    policies["High Risk Policy"] = {{"threshold", "high"}, {"action", "block"}};
    Logger::log("Loaded default policies", LogLevel::INFO);
}

void PolicyManager::applyPolicy(const std::string& policyName) {
    Logger::log("Applying policy " + policyName, LogLevel::INFO);
    dbManager->executeQuery("UPDATE policies SET status='applied' WHERE name='" + policyName + "'");
}

void PolicyManager::logPolicyChange(const std::string& policyName) {
    Logger::log("Policy adjusted to " + policyName, LogLevel::INFO);
    dbManager->logDBOperation("Policy Change", "Policy adjusted to " + policyName);
}

void PolicyManager::loadPoliciesFromConfig() {
    auto policiesConfig = config->getPolicyConfig();
    if (!policiesConfig.empty()) {
        for (const auto& [policyName, policyDetails] : policiesConfig) {
            std::map<std::string, std::string> details;
            for (const auto& [key, value] : policyDetails) {
                details[key] = std::get<std::string>(value);
            }
            policies[policyName] = details;
        }
        Logger::log("Policies loaded from configuration", LogLevel::INFO);
    } else {
        Logger::log("No policies found in configuration, using default policies", LogLevel::WARNING);
        loadPolicies();
    }
}

void PolicyManager::addUserPolicy(const std::string& username, const std::string& policyName) {
    if (policies.find(policyName) != policies.end()) {
        userPolicies[username] = policyName;
        logPolicyAdded(policyName);
        dbManager->executeQuery("INSERT INTO user_policies (username, policy) VALUES ('" + username + "', '" + policyName + "')");
        Logger::log("Added policy " + policyName + " for user " + username, LogLevel::INFO);

        notificationManager->sendEmail(username + "@example.com", "Policy Added", "Policy " + policyName + " has been added to your account.");

        enforceRules(username);
    } else {
        Logger::log("Policy " + policyName + " not found", LogLevel::WARNING);
        dbManager->logDBError("Policy " + policyName + " not found");
        notificationManager->sendEmail("admin@example.com", "Policy Error", "Policy " + policyName + " not found for user " + username + ".");
    }
}

void PolicyManager::removeUserPolicy(const std::string& username) {
    if (userPolicies.find(username) != userPolicies.end()) {
        std::string policyName = userPolicies[username];
        userPolicies.erase(username);
        logPolicyRemoved(policyName);
        dbManager->executeQuery("DELETE FROM user_policies WHERE username='" + username + "'");
        Logger::log("Removed policy " + policyName + " for user " + username, LogLevel::INFO);

        notificationManager->sendEmail(username + "@example.com", "Policy Removed", "Policy " + policyName + " has been removed from your account.");
    } else {
        Logger::log("User policy for " + username + " not found", LogLevel::WARNING);
        dbManager->logDBError("User policy for " + username + " not found");
        notificationManager->sendEmail("admin@example.com", "Policy Error", "User policy for " + username + " not found.");
    }
}

bool PolicyManager::checkUserPolicyCompliance(const std::string& username) const {
    if (userPolicies.find(username) != userPolicies.end()) {
        std::string policyName = userPolicies.at(username);
        Logger::log("Checking policy compliance for user " + username + " with policy " + policyName, LogLevel::INFO);

        enforceRules(username);

        return true;
    } else {
        Logger::log("User policy for " + username + " not found", LogLevel::WARNING);
        dbManager->logDBError("User policy for " + username + " not found");
        notificationManager->sendEmail("admin@example.com", "Policy Compliance Error", "User policy for " + username + " not found.");
        return false;
    }
}


void PolicyManager::logPolicyAdded(const std::string& policyName) {
    Logger::log("Policy " + policyName + " added for a user", LogLevel::INFO);
    dbManager->logDBOperation("Add Policy", "Policy " + policyName + " added for user");
}

void PolicyManager::logPolicyRemoved(const std::string& policyName) {
    Logger::log("Policy " + policyName + " removed from a user", LogLevel::INFO);
    dbManager->logDBOperation("Remove Policy", "Policy " + policyName + " removed from user");
}


bool PolicyManager::backupPolicies(const std::string& backupFilePath) {
    Logger::log("Backing up policies", LogLevel::INFO);
    return dbManager->backupDatabase(backupFilePath);
}

bool PolicyManager::restorePolicies(const std::string& backupFilePath) {
    Logger::log("Restoring policies", LogLevel::INFO);
    return dbManager->restoreDatabase(backupFilePath);
}








