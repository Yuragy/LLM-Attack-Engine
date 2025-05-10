#include "ml_predictor.h"
#include <mlpack/core/data/load.hpp>
#include <mlpack/core/data/save.hpp>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

MLPredictor::MLPredictor(std::shared_ptr<Monitor> monitor, std::shared_ptr<DBManager> dbManager, 
                         std::shared_ptr<RuleEngine> ruleEngine, std::shared_ptr<DictionaryLoader> dictionaryLoader)
    : monitor(monitor), dbManager(dbManager), ruleEngine(ruleEngine), dictionaryLoader(dictionaryLoader) {
    modelTrainer = std::make_unique<MLModelTrainer>(config, *dbManager, *monitor, *dictionaryLoader, *ruleEngine);
}

bool MLPredictor::trainModel(ModelType modelType, const std::map<std::string, double>& hyperparameters) {
    logPredictionProcess("Training model using MLModelTrainer.");
    if (!modelTrainer->trainModel(modelType, hyperparameters)) {
        logError("Error during training model.");
        return false;
    }
    logPredictionProcess("Model trained successfully.");
    return true;
}

bool MLPredictor::loadModel(const std::string& modelPath, ModelType modelType) {
    logPredictionProcess("Loading model using MLModelTrainer.");
    if (!modelTrainer->loadModel(modelPath, modelType)) {
        logError("Error loading model.");
        return false;
    }
    logPredictionProcess("Model loaded successfully.");
    return true;
}

bool MLPredictor::saveModel(const std::string& modelPath, ModelType modelType) {
    logPredictionProcess("Saving model using MLModelTrainer.");
    if (!modelTrainer->saveModel(modelPath)) {
        logError("Error saving model.");
        return false;
    }
    logPredictionProcess("Model saved successfully.");
    return true;
}

arma::Row<size_t> MLPredictor::predict(const arma::mat& inputData) {
    logPredictionProcess("Prediction process started.");
    arma::mat modifiedData = inputData;
    applyRulesToData(modifiedData);
    applyDictionaryToData(modifiedData);

    arma::Row<size_t> predictions;
    if (modelTrainer) {
        predictions = modelTrainer->predict(modifiedData);
    } else {
        logError("Model not loaded or trained.");
        throw std::runtime_error("Model not loaded or trained.");
    }

    logPredictionProcess("Prediction process completed.");
    return predictions;
}

double MLPredictor::evaluate(const arma::mat& inputData, const arma::Row<size_t>& trueLabels) {
    logPredictionProcess("Evaluation process started.");
    double accuracy = modelTrainer->evaluateModel(inputData, trueLabels);
    logPredictionProcess("Evaluation completed. Accuracy: " + std::to_string(accuracy));
    return accuracy;
}

void MLPredictor::interpretModel() {
    logPredictionProcess("Model interpretation started.");
    modelTrainer->interpretModel();
    logPredictionProcess("Model interpretation completed.");
}

void MLPredictor::visualizeFeatureImportance() {
    logPredictionProcess("Visualizing feature importance.");
    modelTrainer->visualizeFeatureImportance();
    logPredictionProcess("Feature importance visualization completed.");
}

void MLPredictor::savePredictions(const std::string& filePath, const arma::Row<size_t>& predictions) {
    std::ofstream file(filePath);
    if (file.is_open()) {
        for (size_t i = 0; i < predictions.n_elem; ++i) {
            file << predictions[i] << std::endl;
        }
        file.close();
        logPredictionProcess("Predictions saved to " + filePath);
    } else {
        logError("Failed to save predictions to file: " + filePath);
    }
}

void MLPredictor::generateReport(const std::string& reportPath) {
    logPredictionProcess("Generating report.");
    modelTrainer->generateReport(reportPath);
    logPredictionProcess("Report generated.");
}

void MLPredictor::preprocessData(arma::mat& data, const std::vector<size_t>& categoricalColumns) {
    logPredictionProcess("Preprocessing data.");
    modelTrainer->preprocessData();
}

void MLPredictor::applyRulesToData(arma::mat& data) {
    logPredictionProcess("Applying rules to data.");
    modelTrainer->applyRulesToData();
}

void MLPredictor::applyDictionaryToData(arma::mat& data) {
    logPredictionProcess("Applying dictionary validation to data.");
    
    std::string dictionaryPath = config.getDictionaryPath(); 

    if (dictionaryPath.empty()) {
        logError("Dictionary path is empty.");
        throw std::runtime_error("Dictionary path is not set.");
    }

    if (!modelTrainer->loadTrainingDataFromDictionary(dictionaryPath)) {
        logError("Failed to load training data from dictionary at: " + dictionaryPath);
        throw std::runtime_error("Failed to load dictionary data.");
    }
}


bool MLPredictor::backupModel(const std::string& modelPath) {
    logPredictionProcess("Backing up model to cloud.");
    if (!modelTrainer->backupModel(modelPath)) {
        logError("Error during model backup.");
        return false;
    }
    logPredictionProcess("Model backup completed.");
    return true;
}

bool MLPredictor::loadModelFromCloud(const std::string& cloudModelPath, const std::string& localModelPath) {
    logPredictionProcess("Loading model from cloud.");
    if (!modelTrainer->loadModelFromCloud(cloudModelPath, localModelPath)) {
        logError("Error loading model from cloud.");
        return false;
    }
    logPredictionProcess("Model loaded from cloud.");
    return true;
}

void MLPredictor::analyzeErrors(const std::string& testDataPath) {
    logPredictionProcess("Analyzing errors in predictions.");
    modelTrainer->analyzeErrors(testDataPath);
    logPredictionProcess("Error analysis completed.");
}

void MLPredictor::handleClassImbalance() {
    logPredictionProcess("Handling class imbalance.");
    modelTrainer->handleClassImbalance();
    logPredictionProcess("Class imbalance handled.");
}

void MLPredictor::manageResources() {
    logPredictionProcess("Managing resources.");
    modelTrainer->manageResources();
    logPredictionProcess("Resource management completed.");
}

double MLPredictor::crossValidate(ModelType modelType, int folds) {
    logPredictionProcess("Cross-validation started.");
    double accuracy = modelTrainer->crossValidate(modelType, folds);
    logPredictionProcess("Cross-validation completed. Accuracy: " + std::to_string(accuracy));
    return accuracy;
}

void MLPredictor::tuneHyperparameters(ModelType modelType, const std::map<std::string, double>& hyperparameters) {
    logPredictionProcess("Tuning hyperparameters.");
    if (!modelTrainer->tuneHyperparameters(modelType, hyperparameters)) {
        logError("Error tuning hyperparameters.");
        return;
    }
    logPredictionProcess("Hyperparameters tuned successfully.");
}

void MLPredictor::setConfig(const Config& config) {
    this->config = config;
    logPredictionProcess("Config set.");
}

void MLPredictor::loadConfig(const std::string& configPath) {
    logPredictionProcess("Loading config.");
    modelTrainer->loadConfig(configPath);
}

void MLPredictor::logPredictionProcess(const std::string& message, LogLevel level) {
    Logger::log(message, level);
    monitor->logAndNotify(message, level);
    dbManager->logEvent(message, level);
}

void MLPredictor::logError(const std::string& errorMessage) {
    Logger::log(errorMessage, LogLevel::ERROR);
    monitor->logAndNotify(errorMessage, LogLevel::ERROR);
    dbManager->logDBError(errorMessage);
}

void MLPredictor::setLogLevel(LogLevel level) {
    Logger::setLogLevel(level);
}

std::string MLPredictor::viewLogs(size_t numLines) {
    return Logger::viewLogs(numLines);
}

std::string MLPredictor::filterLogs(LogLevel level, const std::string& tag, const std::string& regexPattern) {
    return Logger::filterLogs(level, tag, regexPattern);
}

std::string MLPredictor::exportLogs(const std::string& format) {
    return Logger::exportLogs(format);
}

void MLPredictor::clearLogs() {
    Logger::clearLogs();
}

void MLPredictor::logDBOperation(const std::string& operation, const std::string& status) {
    dbManager->logDBOperation(operation, status);
}

void MLPredictor::logDBError(const std::string& error) {
    dbManager->logDBError(error);
}

















