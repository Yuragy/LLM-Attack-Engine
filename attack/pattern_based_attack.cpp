#include "pattern_based_attack.h"
#include <iostream>
#include <random>
#include <future>
#include <armadillo>
#include <filesystem>

namespace fs = std::filesystem;

PatternBasedAttack::PatternBasedAttack(MLPredictor* mlPredictor, RuleEngine* ruleEngine, 
                                       DictionaryLoader* dictionaryLoader, Logger* logger,
                                       ThreadingUtils* threadingUtils, DBManager* dbManager, 
                                       const std::string& threadingStrategy)
    : mlPredictor(mlPredictor), ruleEngine(ruleEngine), dictionaryLoader(dictionaryLoader), 
      logger(logger), threadingUtils(threadingUtils), dbManager(dbManager),
      threadingStrategy(threadingStrategy), patternVerificationCallback(nullptr), stopFlag(false) {
    logger->info("PatternBasedAttack initialized.", {"PatternBasedAttack", "Initialization"});
}

void PatternBasedAttack::setPatternVerificationCallback(std::function<bool(const std::string&)> callback) {
    patternVerificationCallback = callback;
}

void PatternBasedAttack::connectToDatabase() {
    if (!dbManager->connect()) {
        logger->error("Failed to connect to the database.", {"PatternBasedAttack", "DB"});
        stopFlag = true;
    }
}

void PatternBasedAttack::disconnectFromDatabase() {
    dbManager->disconnect();
    logger->info("Disconnected from the database.", {"PatternBasedAttack", "DB"});
}

std::future<void> PatternBasedAttack::loadDictionariesAsync() {
    logger->info("Loading dictionaries asynchronously...", {"PatternBasedAttack", "Dictionaries"});

    fs::path currentPath = fs::current_path();
    std::vector<std::string> dictionaryPaths = {
        (currentPath / "dictionaries/patterns1.txt").string(),
        (currentPath / "dictionaries/patterns2.txt").string()
    };

    return dictionaryLoader->loadMultipleAsync(dictionaryPaths).then([this](bool result) {
        if (!result) {
            logger->error("Failed to load one or more dictionaries.", {"PatternBasedAttack", "Dictionary"});
            stopFlag = true;
        } else {
            logger->info("Dictionaries loaded successfully.", {"PatternBasedAttack", "Dictionary"});
        }
    });
}

void PatternBasedAttack::loadPatternsFromDictionaries() {
    logger->info("Loading patterns from dictionaries...", {"PatternBasedAttack", "Dictionaries"});
    std::vector<std::string> loadedPatterns = dictionaryLoader->getAllWords();
    applyRulesToPatterns(loadedPatterns);

    for (const auto& pattern : loadedPatterns) {
        std::unique_lock<std::mutex> lock(mtx);
        patternQueue.push(pattern);
        lock.unlock();
    }
    logger->info("Patterns loaded from dictionaries and queued.", {"PatternBasedAttack", "Dictionaries"});
}

void PatternBasedAttack::loadPatternsFromDatabase() {
    connectToDatabase();
    logger->info("Loading patterns from database...", {"PatternBasedAttack", "Database"});

    if (!dictionaryLoader->loadFromDatabase("valid_database_connection_string")) {
        logger->error("Failed to load patterns from database.", {"PatternBasedAttack", "Database"});
        stopFlag = true;
    }

    disconnectFromDatabase();
    logger->info("Patterns loaded from database and queued.", {"PatternBasedAttack", "Database"});
}

std::vector<std::string> PatternBasedAttack::generatePatterns(size_t length) {
    logger->info("Generating patterns...", {"PatternBasedAttack", "Generation"});
    
    std::vector<std::string> patterns;
    std::string numbers = "0123456789";
    std::string symbols = "!@#$%^&*";
    
    std::vector<std::string> words = dictionaryLoader->getAllWords();

    std::default_random_engine engine;
    std::uniform_int_distribution<int> wordDist(0, words.size() - 1);
    std::uniform_int_distribution<int> numDist(0, numbers.size() - 1);
    std::uniform_int_distribution<int> symDist(0, symbols.size() - 1);

    std::vector<std::string> commonPatterns = {
        "{word}{number}{symbol}",
        "{word}{word}{number}",
        "{word}{number}{number}{symbol}"
    };

    for (size_t i = 0; i < 100; ++i) {
        std::string pattern = commonPatterns[i % commonPatterns.size()];

        std::string word1 = words[wordDist(engine)];
        std::string word2 = words[wordDist(engine)];
        std::string number = std::string(1, numbers[numDist(engine)]);
        std::string symbol = std::string(1, symbols[symDist(engine)]);

        size_t pos;
        while ((pos = pattern.find("{word}")) != std::string::npos) {
            pattern.replace(pos, 6, word1);
        }
        if ((pos = pattern.find("{number}")) != std::string::npos) {
            pattern.replace(pos, 8, number);
        }
        if ((pos = pattern.find("{symbol}")) != std::string::npos) {
            pattern.replace(pos, 8, symbol);
        }

        patterns.push_back(pattern);
    }

    logger->info("Patterns generated using dictionary and common patterns.", {"PatternBasedAttack", "Generation"});
    return patterns;
}

void PatternBasedAttack::applyMachineLearningModel(std::vector<std::string>& patterns) {
    logger->info("Applying machine learning model to patterns.", {"PatternBasedAttack", "MLModel"});

    arma::mat inputData(patterns.size(), 3);

    for (size_t i = 0; i < patterns.size(); ++i) {
        inputData(i, 0) = patterns[i].length(); 
        inputData(i, 1) = std::count_if(patterns[i].begin(), patterns[i].end(), ::isdigit);  
        inputData(i, 2) = std::count_if(patterns[i].begin(), patterns[i].end(), ::ispunct);  
    }

    arma::Row<double> predictions = mlPredictor->predict(inputData);

    for (size_t i = 0; i < patterns.size(); ++i) {
        double probability = predictions[i];  
        patterns[i] += "_prob:" + std::to_string(probability); 
        logger->info("Pattern: " + patterns[i] + " predicted with success probability: " + std::to_string(probability));
    }
}

void PatternBasedAttack::applyRulesToPatterns(std::vector<std::string>& patterns) {
    logger->info("Applying rules to patterns...", {"PatternBasedAttack", "Rules"});

    std::vector<std::function<void()>> tasks;
    for (const auto& pattern : patterns) {
        tasks.push_back([this, &pattern]() {
            std::vector<std::string> transformedPatterns = ruleEngine->applyRules(pattern);
            {
                std::unique_lock<std::mutex> lock(mtx);
                for (const auto& transformed : transformedPatterns) {
                    patternQueue.push(transformed);
                }
            }
        });
    }

    threadingUtils->runInParallel(tasks, threadingStrategy);
    logger->info("Patterns transformed and queued.", {"PatternBasedAttack", "Rules"});
}

void PatternBasedAttack::patternWorker() {
    while (!stopFlag) {
        std::string pattern;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (patternQueue.empty()) {
                continue;
            }
            pattern = patternQueue.front();
            patternQueue.pop();
        }

        logAttackDetails(pattern);

        if (patternVerificationCallback && patternVerificationCallback(pattern)) {
            stopFlag = true;
        }

        if (checkIfStop()) break;
    }
}

void PatternBasedAttack::execute() {
    logger->info("Starting pattern-based attack.", {"PatternBasedAttack", "Execution"});

    auto future = loadDictionariesAsync();
    future.wait();
    if (stopFlag) return;

    loadPatternsFromDictionaries();
    loadPatternsFromDatabase();

    threadingUtils->enableMonitoring();

    std::vector<std::function<void()>> tasks;
    for (size_t length = 1; length <= 16; ++length) {
        tasks.push_back([this, length]() {
            generatePatterns(length);
        });
    }

    threadingUtils->runInParallel(tasks, threadingStrategy);

    stopFlag = true;

    threadingUtils->stopThreads();

    evaluateModel();
    analyzeErrors();
    manageResources();

    logger->info("Pattern-based attack completed.", {"PatternBasedAttack", "Execution"});
}

void PatternBasedAttack::logAttackDetails(const std::string& pattern) {
    logger->trace("Attempting pattern: " + pattern, {"PatternBasedAttack", "AttackDetails"});
}

void PatternBasedAttack::evaluateModel() {
    logger->info("Evaluating model after pattern-based attack.", {"PatternBasedAttack", "Evaluation"});

    arma::mat inputData;
    arma::Row<size_t> trueLabels;

    double accuracy = mlPredictor->evaluate(inputData, trueLabels);
    logger->info("Model accuracy: " + std::to_string(accuracy), {"PatternBasedAttack", "Evaluation"});
}

void PatternBasedAttack::analyzeErrors() {
    logger->info("Analyzing errors after pattern-based attack.", {"PatternBasedAttack", "ErrorAnalysis"});

    fs::path currentPath = fs::current_path();
    std::string testDataPath = (currentPath / "data/test_data.txt").string();

    mlPredictor->analyzeErrors(testDataPath);
}

void PatternBasedAttack::manageResources() {
    logger->info("Managing resources after pattern-based attack.", {"PatternBasedAttack", "ResourceManagement"});
    mlPredictor->manageResources();
}

bool PatternBasedAttack::checkIfStop() {
    return stopFlag;
}










