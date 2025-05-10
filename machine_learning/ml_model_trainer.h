#ifndef ML_MODEL_TRAINER_H
#define ML_MODEL_TRAINER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <armadillo>
#include <mlpack/core.hpp>
#include <mlpack/methods/logistic_regression/logistic_regression.hpp>
#include <mlpack/methods/decision_tree/decision_tree.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/layer/layer.hpp>
#include <mlpack/methods/knn/knn.hpp>
#include <mlpack/methods/naive_bayes/naive_bayes_classifier.hpp>
#include <mlpack/methods/svm/svm.hpp>
#include <mlpack/methods/random_forest/random_forest.hpp>
#include <mlpack/methods/adaboost/adaboost.hpp>
#include <mlpack/core/cv/k_fold_cv.hpp>
#include "logging/logger.h"
#include "gpu_manager.h"
#include "database/db_manager.h"
#include "cloud_integration.h"
#include "monitor.h"
#include "dictionary_loader.h"
#include "rule_engine.h"
#include "config.h"

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

class MLModelTrainer {
public:
    MLModelTrainer(const Config& config, DBManager& dbManager, GPUManager& gpuManager, 
                   CloudIntegration& cloudIntegration, Monitor& monitor, 
                   DictionaryLoader& dictionaryLoader, RuleEngine& ruleEngine);
    ~MLModelTrainer();

    bool loadTrainingData(const std::string& dataPath, const std::string& format = "csv");
    bool loadTrainingDataFromDictionary(const std::string& dictionaryPath);
    bool trainModel(ModelType modelType, const std::map<std::string, double>& hyperparameters = {});
    bool saveModel(const std::string& modelPath);
    bool loadModel(const std::string& modelPath, ModelType modelType);
    double evaluateModel(const std::string& testDataPath, const std::string& format = "csv");
    bool tuneHyperparameters(ModelType modelType, const std::map<std::string, double>& hyperparameters = {});
    void interpretModel();
    void visualizeFeatureImportance();
    double crossValidate(ModelType modelType, int folds = 5);
    void analyzeErrors(const std::string& testDataPath, const std::string& format = "csv");
    void handleClassImbalance();
    void manageResources();
    void generateReport(const std::string& reportPath);
    void logTrainingProcess(const std::string& message, LogLevel level = LogLevel::INFO);
    bool backupModel(const std::string& modelPath);
    bool loadModelFromCloud(const std::string& cloudModelPath, const std::string& localModelPath);

private:
    arma::mat trainingData;
    arma::Row<size_t> labels;
    std::unique_ptr<mlpack::regression::LogisticRegression<>> logisticModel;
    std::unique_ptr<mlpack::tree::DecisionTree<>> decisionTreeModel;
    std::unique_ptr<mlpack::ann::FFN<mlpack::ann::CrossEntropyError<>, mlpack::ann::RandomInitialization>> neuralNetworkModel;
    std::unique_ptr<mlpack::knn::KNN> knnModel;
    std::unique_ptr<mlpack::naive_bayes::NaiveBayesClassifier<>> naiveBayesModel;
    std::unique_ptr<mlpack::svm::SVM<>> svmModel;
    std::unique_ptr<mlpack::tree::RandomForest<>> randomForestModel;
    std::unique_ptr<mlpack::adaboost::AdaBoost<mlpack::tree::DecisionTree<>>> gradientBoostingModel;

    const Config& config;
    DBManager& dbManager;
    GPUManager& gpuManager;
    CloudIntegration& cloudIntegration;
    Monitor& monitor;
    DictionaryLoader& dictionaryLoader;
    RuleEngine& ruleEngine;

    bool validateData() const;
    void preprocessData();
    void applyRulesToData();
    double evaluateTransformationQuality(const std::string& originalWord, const std::string& transformedWord);
    void plotData() const;
    arma::mat convertWordsToVectors(const std::vector<std::string>& words);
    arma::mat convertJSONToMatrix(const nlohmann::json& jsonData);
    arma::mat convertCSVToMatrix(const std::string& csvData);
};

#endif // ML_MODEL_TRAINER_H













