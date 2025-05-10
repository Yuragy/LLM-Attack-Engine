#ifndef PERSONALIZED_DICTIONARY_GENERATOR_H
#define PERSONALIZED_DICTIONARY_GENERATOR_H

#include <string>
#include <vector>
#include <memory>
#include "../logging/logger.h"
#include "../database/db_manager.h"
#include "machine_learning/ml_predictor.h"
#include "rules/rule_engine.h"

class PersonalizedDictionaryGenerator {
public:
    PersonalizedDictionaryGenerator(std::shared_ptr<Logger> logger, 
                                    std::shared_ptr<DBManager> dbManager, 
                                    std::shared_ptr<MLPredictor> mlPredictor,
                                    std::shared_ptr<RuleEngine> ruleEngine);
    ~PersonalizedDictionaryGenerator();

    std::vector<std::string> generateDictionary(const std::string& target);

    bool saveDictionaryToDatabase(const std::string& target, const std::vector<std::string>& dictionary);

private:
    void logDictionaryGenerationStart(const std::string& target);
    void logDictionaryGenerationEnd(const std::vector<std::string>& dictionary);
    void handleError(const std::string& message, const std::string& context);
    void addStandardPasswords(std::vector<std::string>& dictionary, const std::string& target);
    void applyRulesToDictionary(std::vector<std::string>& dictionary);
    void applyMLModelToDictionary(std::vector<std::string>& dictionary, const std::string& target);

    std::shared_ptr<Logger> logger;
    std::shared_ptr<DBManager> dbManager;
    std::shared_ptr<MLPredictor> mlPredictor;
    std::shared_ptr<RuleEngine> ruleEngine; 
};

#endif // PERSONALIZED_DICTIONARY_GENERATOR_H
