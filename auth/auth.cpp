#include "auth.h"
#include "../database/db_manager.h"
#include "../logging/logger.h"
#include "../config/config.h"
#include "../notification/notification_manager.h"
#include "../cloud/cloud_integration.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <openssl/sha.h>
#include <jwt-cpp/jwt.h>
#include <random>
#include <iomanip>
#include <thread>

Auth::Auth(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger, std::shared_ptr<NotificationManager> notificationManager, std::shared_ptr<ThreadingUtils> threadingUtils)
    : config(config), logger(logger), notificationManager(notificationManager), threadingUtils(threadingUtils) {

    std::string dbConnectionString = config->getDBConnectionString();
    if (dbConnectionString.empty()) {
        logger->error("Failed to retrieve DB connection string from config");
        return;
    }

    dbManager = std::make_unique<DBManager>(dbConnectionString, nullptr, logger, nullptr, nullptr, config);

    cloudIntegration = std::make_shared<CloudIntegration>("AWS", "apiKey", config, threadingUtils, notificationManager);

    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database", {"DB", "Auth"});
        return;
    }

    config->load("config_file_path");
}

bool Auth::login(const std::string& username, const std::string& password) {
    bool success = false;
    threadingUtils->runInParallel({
        [this, &success, &username, &password] {
            if (verifyCredentials(username, password)) {
                std::string sessionId = generateSessionId();
                std::string query = "UPDATE users SET sessions = sessions || ',' || '" + sessionId + "' WHERE username = '" + username + "'";
                dbManager->executeQuery(query);

                logEvent("User " + username + " logged in.", LogLevel::INFO);
                notificationManager->sendEmail(username, "Login Notification", "You have successfully logged in.");
                cloudIntegration->uploadData("user_login_log.txt", "cloud_path/login_log.txt");
                success = true;
            } else {
                logEvent("Failed login attempt for user " + username + ".", LogLevel::WARNING);
                notificationManager->sendEmail(username, "Failed Login Attempt", "There was an attempt to log in with your account.");
                cloudIntegration->uploadData("failed_login_log.txt", "cloud_path/failed_login_log.txt");
            }
        }
    });
    return success;
}

bool Auth::logout(const std::string& sessionId) {
    bool success = false;
    threadingUtils->runInParallel({
        [this, &success, &sessionId] {
            std::string query = "UPDATE users SET sessions = REPLACE(sessions, '" + sessionId + "', '') WHERE sessions LIKE '%" + sessionId + "%'";
            success = dbManager->executeQuery(query);

            if (success) {
                logEvent("User logged out with session ID: " + sessionId, LogLevel::INFO);
                notificationManager->sendEmail("admin@example.com", "Logout Notification", "A user has logged out with session ID: " + sessionId);
                cloudIntegration->uploadData("user_logout_log.txt", "cloud_path/logout_log.txt");
            }
        }
    });
    return success;
}

bool Auth::checkSession(const std::string& sessionId) {
    bool valid = false;
    threadingUtils->runInParallel({
        [this, &valid, &sessionId] {
            std::string query = "SELECT username FROM users WHERE sessions LIKE '%" + sessionId + "%'";
            std::string result = dbManager->fetchData(query);
            valid = !result.empty();
        }
    });
    return valid;
}

bool Auth::enable2FA(const std::string& username) {
    bool success = false;
    threadingUtils->runInParallel({
        [this, &success, &username] {
            std::string twoFASecret = generateSessionId();
            std::string query = "UPDATE users SET twoFAEnabled = 1, twoFASecret = '" + twoFASecret + "' WHERE username = '" + username + "'";
            success = dbManager->executeQuery(query);

            if (success) {
                logEvent("2FA enabled for user " + username, LogLevel::INFO);
                notificationManager->sendEmail(username, "2FA Enabled", "Two-factor authentication has been enabled for your account.");
                cloudIntegration->uploadData("2fa_enabled_log.txt", "cloud_path/2fa_log.txt");
            }
        }
    });
    return success;
}

bool Auth::verify2FA(const std::string& username, const std::string& code) {
    bool valid = false;
    threadingUtils->runInParallel({
        [this, &valid, &username, &code] {
            std::string query = "SELECT twoFASecret FROM users WHERE username = '" + username + "' AND twoFAEnabled = 1";
            std::string secret = dbManager->fetchData(query);

            if (!secret.empty() && code == secret) {
                valid = true;
                logEvent("2FA verification successful for user " + username, LogLevel::INFO);
                notificationManager->sendEmail(username, "2FA Verified", "Two-factor authentication was successfully verified.");
                cloudIntegration->uploadData("2fa_verification_log.txt", "cloud_path/2fa_verification_log.txt");
            } else {
                logEvent("2FA verification failed for user " + username, LogLevel::WARNING);
                notificationManager->sendEmail(username, "2FA Failed", "Two-factor authentication verification failed.");
            }
        }
    });
    return valid;
}

std::string Auth::generate2FAToken(const std::string& username) {
    std::string token;
    threadingUtils->runInParallel({
        [this, &token, &username] {
            std::string query = "SELECT twoFASecret FROM users WHERE username = '" + username + "'";
            std::string secret = dbManager->fetchData(query);

            if (!secret.empty()) {
                token = secret;
            }
        }
    });
    return token;
}

bool Auth::verify2FAToken(const std::string& username, const std::string& token) {
    return verify2FA(username, token);
}

std::string Auth::generatePasswordRecoveryToken(const std::string& username) {
    std::string token;
    threadingUtils->runInParallel({
        [this, &token, &username] {
            token = generateSessionId();
            std::string query = "UPDATE users SET recoveryToken = '" + token + "' WHERE username = '" + username + "'";
            dbManager->executeQuery(query);

            logEvent("Password recovery token generated for user " + username, LogLevel::INFO);
            notificationManager->sendEmail(username, "Password Recovery Token", "Your password recovery token is: " + token);
            cloudIntegration->uploadData("password_recovery_log.txt", "cloud_path/password_recovery_log.txt");
        }
    });
    return token;
}

bool Auth::verifyPasswordRecoveryToken(const std::string& username, const std::string& token) {
    bool valid = false;
    threadingUtils->runInParallel({
        [this, &valid, &username, &token] {
            std::string query = "SELECT recoveryToken FROM users WHERE username = '" + username + "' AND recoveryToken = '" + token + "'";
            std::string result = dbManager->fetchData(query);
            valid = !result.empty();

            if (valid) {
                logEvent("Password recovery token validated for user " + username, LogLevel::INFO);
                notificationManager->sendEmail(username, "Password Recovery Successful", "Your password recovery token has been validated.");
            } else {
                logEvent("Invalid password recovery token for user " + username, LogLevel::WARNING);
                notificationManager->sendEmail(username, "Invalid Password Recovery Token", "The password recovery token is invalid.");
            }
        }
    });
    return valid;
}

bool Auth::setUserRole(const std::string& username, const std::string& role) {
    bool success = false;
    threadingUtils->runInParallel({
        [this, &success, &username, &role] {
            std::string query = "UPDATE users SET role = '" + role + "' WHERE username = '" + username + "'";
            success = dbManager->executeQuery(query);

            if (success) {
                logEvent("User role updated for " + username + " to role " + role, LogLevel::INFO);
                notificationManager->sendEmail(username, "Role Updated", "Your role has been updated to: " + role);
            } else {
                logEvent("Failed to update role for user " + username, LogLevel::WARNING);
            }
        }
    });
    return success;
}

std::string Auth::getUserRole(const std::string& username) {
    std::string role;
    threadingUtils->runInParallel({
        [this, &role, &username] {
            std::string query = "SELECT role FROM users WHERE username = '" + username + "'";
            role = dbManager->fetchData(query);
        }
    });
    return role;
}

std::vector<std::string> Auth::getUserPermissions(const std::string& role) {
    std::vector<std::string> permissions;
    threadingUtils->runInParallel({
        [this, &permissions, &role] {
            if (role == "admin") {
                permissions = {"read", "write", "delete", "manage_users"};
            } else if (role == "user") {
                permissions = {"read", "write"};
            } else {
                permissions = {"read"};
            }
        }
    });
    return permissions;
}

bool Auth::verifyCredentials(const std::string& username, const std::string& password) {
    bool valid = false;
    threadingUtils->runInParallel({
        [this, &valid, &username, &password] {
            std::string query = "SELECT passwordHash FROM users WHERE username = '" + username + "'";
            std::string storedHash = dbManager->fetchData(query);

            if (!storedHash.empty()) {
                std::string hashedPassword = hashPassword(password);
                valid = storedHash == hashedPassword;
            }
        }
    });
    return valid;
}

std::string Auth::generateSessionId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
    return ss.str();
}

std::string Auth::hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

void Auth::logEvent(const std::string& event, LogLevel level) {
    logger->log(event, level);
}









