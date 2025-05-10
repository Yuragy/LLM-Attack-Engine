#ifndef POLICY_MANAGER_H
#define POLICY_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include "../config/config.h"  
#include "../rules/rule_engine.h"  
#include "../notification_manager.h"  
#include "../logging/logger.h"  
#include "../database/db_manager.h"  

class PolicyManager {
public:
    PolicyManager(Config* config, DBManager* dbManager, NotificationManager* notificationManager, RuleEngine* ruleEngine);

    void adjustPolicy(const std::string& policyName);
    std::string getCurrentPolicy() const;
    std::vector<std::string> getAvailablePolicies() const;
    std::map<std::string, std::string> getPolicyDetails(const std::string& policyName) const;

    void addUserPolicy(const std::string& username, const std::string& policyName);
    void removeUserPolicy(const std::string& username);
    bool checkUserPolicyCompliance(const std::string& username) const;

    void logPolicyChange(const std::string& policyName);
    void logPolicyAdded(const std::string& policyName);
    void logPolicyRemoved(const std::string& policyName);

    bool backupPolicies(const std::string& backupFilePath);
    bool restorePolicies(const std::string& backupFilePath);

private:
    std::string currentPolicy;
    std::map<std::string, std::map<std::string, std::string>> policies;
    std::map<std::string, std::string> userPolicies; 
    Config* config; 
    DBManager* dbManager;
    NotificationManager* notificationManager;
    RuleEngine* ruleEngine;

    void loadPolicies();
    void applyPolicy(const std::string& policyName);
    void loadPoliciesFromConfig();
    void enforceRules(const std::string& username); 
};

#endif // POLICY_MANAGER_H






