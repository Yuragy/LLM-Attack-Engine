#ifndef ML_PREDICTOR_H
#define ML_PREDICTOR_H

#include <string>
#include <memory>
#include <armadillo>
#include <mlpack/core.hpp>
#include "logging/logger.h"
#include "monitoring/monitor.h"
#include "machine_learning/ml_model_trainer.h"
#include "dictionary/dictionary_loader.h"
#include "rules/rule_engine.h"
#include "database/db_manager.h"
#include <nlohmann/json.hpp>
#include <shap.hpp>
#include "matplotlibcpp.h"
#include "config.h"

namespace plt = matplotlibcpp;

enum class ModelType {
    LogisticRegression,
    DecisionTree,
    NeuralNetwork,
    KNN,
    NaiveBayes,
    SVM,
    RandomForest,
    GradientBoosting
};

class MLPredictor {
public:
    MLPredictor(std::shared_ptr<Monitor> monitor, std::shared_ptr<DBManager> dbManager, 
                std::shared_ptr<RuleEngine> ruleEngine, std::shared_ptr<DictionaryLoader> dictionaryLoader);

    bool loadModel(const std::string& modelPath, ModelType modelType);
    bool saveModel(const std::string& modelPath, ModelType modelType);
    bool trainModel(ModelType modelType, const std::map<std::string, double>& hyperparameters);
    arma::Row<size_t> predict(const arma::mat& inputData);
    double evaluate(const arma::mat& inputData, const arma::Row<size_t>& trueLabels);
    void interpretModel();
    void visualizeFeatureImportance();
    void savePredictions(const std::string& filePath, const arma::Row<size_t>& predictions);
    void generateReport(const std::string& reportPath);
    void preprocessData(arma::mat& data, const std::vector<size_t>& categoricalColumns);

    void applyRulesToData(arma::mat& data);
    void applyDictionaryToData(arma::mat& data);

    void setConfig(const Config& config);
    void loadConfig(const std::string& configPath);

    void logPredictionProcess(const std::string& message, LogLevel level = LogLevel::INFO);
    void logError(const std::string& errorMessage);

    void setLogLevel(LogLevel level);
    std::string viewLogs(size_t numLines);
    std::string filterLogs(LogLevel level, const std::string& tag = "", const std::string& regexPattern = "");
    std::string exportLogs(const std::string& format);
    void clearLogs();
    std::map<LogLevel, size_t> getLogStatistics();
    bool backupModel(const std::string& modelPath);
    bool loadModelFromCloud(const std::string& cloudModelPath, const std::string& localModelPath);
    void analyzeErrors(const std::string& testDataPath);
    void handleClassImbalance();
    void manageResources();
    double crossValidate(ModelType modelType, int folds);
    void tuneHyperparameters(ModelType modelType, const std::map<std::string, double>& hyperparameters);

private:
    std::shared_ptr<Monitor> monitor;
    std::shared_ptr<DBManager> dbManager;
    std::shared_ptr<RuleEngine> ruleEngine;
    std::shared_ptr<DictionaryLoader> dictionaryLoader;

    Config config;
    std::unique_ptr<MLModelTrainer> modelTrainer;

    void logDBOperation(const std::string& operation, const std::string& status);
    void logDBError(const std::string& error);
};

#endif // ML_PREDICTOR_H









