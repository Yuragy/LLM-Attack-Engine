#include "personalized_dictionary_generator.h"
#include <sstream> 

PersonalizedDictionaryGenerator::PersonalizedDictionaryGenerator(std::shared_ptr<Logger> logger, 
                                                                 std::shared_ptr<DBManager> dbManager, 
                                                                 std::shared_ptr<MLPredictor> mlPredictor,
                                                                 std::shared_ptr<RuleEngine> ruleEngine)
    : logger(logger), dbManager(dbManager), mlPredictor(mlPredictor), ruleEngine(ruleEngine) {
}

PersonalizedDictionaryGenerator::~PersonalizedDictionaryGenerator() {
}

std::vector<std::string> PersonalizedDictionaryGenerator::generateDictionary(const std::string& target) {
    logDictionaryGenerationStart(target);

    std::vector<std::string> dictionary;

    try {
        std::string query = "SELECT data FROM social_data WHERE target = '" + target + "'";
        auto results = dbManager->executeQuery(query);

        for (const auto& row : results) {
            dictionary.push_back(row["data"]);
        }
        addStandardPasswords(dictionary, target);
        applyRulesToDictionary(dictionary);
        applyMLModelToDictionary(dictionary, target);
        saveDictionaryToDatabase(target, dictionary);
    } catch (const std::exception& e) {
        handleError("Failed to generate personalized dictionary: " + std::string(e.what()), "Dictionary Generation");
    }

    logDictionaryGenerationEnd(dictionary);
    return dictionary;
}

void PersonalizedDictionaryGenerator::addStandardPasswords(std::vector<std::string>& dictionary, const std::string& target) {
    dictionary.push_back(target + "123");
    dictionary.push_back(target + "_password");
    dictionary.push_back("welcome_" + target);
    logger->log("Added standard passwords for target: " + target, LogLevel::INFO);
}

bool PersonalizedDictionaryGenerator::saveDictionaryToDatabase(const std::string& target, const std::vector<std::string>& dictionary) {
    try {
        for (const auto& word : dictionary) {
            std::string query = "INSERT INTO generated_dictionaries (target, word) VALUES ('" + target + "', '" + word + "')";
            dbManager->executeQuery(query);
        }
        logger->log("Dictionary saved to database for target: " + target, LogLevel::INFO);
        return true;
    } catch (const std::exception& e) {
        handleError("Failed to save dictionary to database: " + std::string(e.what()), "Database Save");
        return false;
    }
}

void PersonalizedDictionaryGenerator::logDictionaryGenerationStart(const std::string& target) {
    logger->log("Starting personalized dictionary generation for target: " + target, LogLevel::INFO);
}

void PersonalizedDictionaryGenerator::logDictionaryGenerationEnd(const std::vector<std::string>& dictionary) {
    logger->log("Finished personalized dictionary generation. Dictionary size: " + std::to_string(dictionary.size()), LogLevel::INFO);
}

void PersonalizedDictionaryGenerator::handleError(const std::string& message, const std::string& context) {
    logger->log("Error in context: " + context + " - " + message, LogLevel::ERROR);
}

void PersonalizedDictionaryGenerator::applyRulesToDictionary(std::vector<std::string>& dictionary) {
    std::vector<std::string> transformedDictionary;
    for (const auto& word : dictionary) {
        auto transformedWords = ruleEngine->applyRules(word);
        transformedDictionary.insert(transformedDictionary.end(), transformedWords.begin(), transformedWords.end());
    }

    dictionary = transformedDictionary;

    logger->log("Applied rules to dictionary. New size: " + std::to_string(dictionary.size()), LogLevel::INFO);
}

void PersonalizedDictionaryGenerator::applyMLModelToDictionary(std::vector<std::string>& dictionary, const std::string& target) {
    try {
        logger->log("Applying ML model to dictionary for target: " + target, LogLevel::INFO);

        arma::mat data(dictionary.size(), 1); 
        for (size_t i = 0; i < dictionary.size(); ++i) {
            double numValue = 0.0;
            for (char c : dictionary[i]) {
                numValue += static_cast<double>(c);
            }
            data(i, 0) = numValue;
        }

        mlPredictor->applyRulesToData(data);
        mlPredictor->applyDictionaryToData(data);

        arma::Row<size_t> predictions = mlPredictor->predict(data);

        std::vector<std::string> filteredDictionary;
        for (size_t i = 0; i < predictions.n_elem; ++i) {
            if (predictions[i] == 1) {
                filteredDictionary.push_back(dictionary[i]);
            }
        }

        dictionary = filteredDictionary;

        logger->log("ML model applied successfully to dictionary for target: " + target, LogLevel::INFO);
    } catch (const std::exception& e) {
        handleError("Failed to apply ML model to dictionary: " + std::string(e.what()), "ML Application");
    }
}

