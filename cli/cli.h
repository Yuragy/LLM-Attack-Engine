#ifndef CLI_H
#define CLI_H

#include <string>
#include <vector>
#include <map>
#include "../dictionary/dictionary_loader.h"
#include "../config/config.h"
#include "../rules/rule_engine.h"
#include "../attack/attack_engine.h"
#include "../gpu/gpu_manager.h"
#include "../ml/ml_model_trainer.h"
#include "../ml/ml_predictor.h"
#include "../integration/external_service.h"
#include "../security/access_control.h"
#include "../system/system_manager.h"
#include "../update/update_manager.h"
#include "../data/data_analyzer.h"
#include "../logging/logger.h"
#include "../monitoring/monitor.h"
#include "../notifications/notification_manager.h"
#include "../users/user_management.h"
#include "../database/db_manager.h"

class CLI {
public:
    CLI(int argc, char* argv[]);

    bool parseArguments();
    std::string getCommand() const;
    std::vector<std::string> getArguments() const;
    std::string getArgumentValue(const std::string &key) const;
    bool hasArgument(const std::string &key) const;

    bool handleDictionaryCommand(DictionaryLoader& dictLoader);
    bool handleConfigCommand(Config& config);
    bool handleAttackCommand(AttackEngine& attackEngine);
    bool handleRuleCommand(RuleEngine& ruleEngine);
    bool handleUserCommand(UserManagement& userManagement);
    bool handleScheduleCommand(Scheduler& scheduler);
    bool handleNotificationCommand(NotificationManager& notificationManager);
    bool handleAnalyticsCommand(AnalyticsManager& analyticsManager);
    bool handleGPUCommand(GPUManager& gpuManager);
    bool handleMLModelTrainerCommand(MLModelTrainer& mlModelTrainer);
    bool handleMLPredictorCommand(MLPredictor& mlPredictor);
    bool handleIntegrationCommand(ExternalService& externalService);
    bool handleSecurityCommand(AccessControl& accessControl);
    bool handleSystemCommand(SystemManager& systemManager);
    bool handleUpdateCommand(UpdateManager& updateManager);
    bool handleDataAnalysisCommand(DataAnalyzer& dataAnalyzer);
    bool handleScriptCommand();
    bool handleLogCommand(Logger& logger);
    bool handleCustomNotificationCommand(NotificationManager& notificationManager);
    bool handleMonitorCommand(Monitor& monitor);
    bool handleDBCommand(DBManager& dbManager);
    void interactiveMode();

    void displayHelp() const;

private:
    int argc;
    char** argv;
    std::string command;
    std::vector<std::string> arguments;
    std::map<std::string, std::string> argumentMap;

    void parseArgumentMap();
    std::string getMaskFromUser();
    std::string getFilenameFromUser();
    std::string getStatusFromUser();
};

#endif // CLI_H




