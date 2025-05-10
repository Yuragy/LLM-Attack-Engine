#include "ml_model_trainer.h"
#include <mlpack/core/data/load.hpp>
#include <mlpack/core/data/save.hpp>
#include <mlpack/core/data/scaler_methods/min_max_scaler.hpp>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

MLModelTrainer::MLModelTrainer(const Config& config, DBManager& dbManager, GPUManager& gpuManager,
                               CloudIntegration& cloudIntegration, Monitor& monitor,
                               DictionaryLoader& dictionaryLoader, RuleEngine& ruleEngine)
    : logisticModel(nullptr), decisionTreeModel(nullptr), neuralNetworkModel(nullptr), knnModel(nullptr),
      naiveBayesModel(nullptr), svmModel(nullptr), randomForestModel(nullptr), gradientBoostingModel(nullptr),
      config(config), dbManager(dbManager), gpuManager(gpuManager), cloudIntegration(cloudIntegration),
      monitor(monitor), dictionaryLoader(dictionaryLoader), ruleEngine(ruleEngine) {

    auto logConfig = config.getLogConfig();
    if (logConfig.find("log_path") != logConfig.end()) {
        Logger::initialize(logConfig["log_path"].get<std::string>());
    } else {
        Logger::initialize("default_log_path");
    }

    auto gpuConfig = config.getGPUConfig();
    if (gpuConfig.find("enabled") != gpuConfig.end() && gpuConfig["enabled"].get<bool>()) {
        gpuManager.initialize();
        gpuManager.optimizeMemoryUsage();
    }

    logTrainingProcess("MLModelTrainer initialized.");
    monitor.monitorMLTraining("MLModelTrainer", "Initialization completed");
}

MLModelTrainer::~MLModelTrainer() {
    gpuManager.stopAllOperations();
    gpuManager.managePowerConsumption();
    Logger::shutdown();
    monitor.monitorMLTraining("MLModelTrainer", "Shutdown completed");
}

bool MLModelTrainer::loadTrainingData(const std::string& dataPath, const std::string& format) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Loading training data from " + dataPath);
        if (format == "csv") {
            DictionaryLoader::loadData(dataPath, trainingData, labels);
        } else if (format == "json") {
            auto jsonData = dbManager.parseAndValidateJSON(dataPath, {/* schema */});
            trainingData = convertJSONToMatrix(jsonData);
        } else {
            throw std::runtime_error("Unsupported data format: " + format);
        }

        if (!validateData()) {
            throw std::runtime_error("Invalid data format.");
        }

        preprocessData();
        logTrainingProcess("Data successfully loaded from " + dataPath);
        monitor.monitorMLTraining("MLModelTrainer", "Training data loaded successfully");
        return true;
    } catch (const std::exception& e) {
        logTrainingProcess("Error loading data: " + std::string(e.what()), LogLevel::ERROR);
        dbManager.logDBError("Error loading data: " + std::string(e.what()));
        monitor.monitorMLTraining("MLModelTrainer", "Error loading training data");
        return false;
    }
}

arma::mat MLModelTrainer::convertJSONToMatrix(const nlohmann::json& jsonData) {
    try {
        if (!jsonData.is_array() || jsonData.empty()) {
            throw std::runtime_error("Invalid JSON format: expected a non-empty array.");
        }

        size_t rows = jsonData.size();
        size_t cols = jsonData[0].size();
        arma::mat matrix(rows, cols);

        for (size_t i = 0; i < rows; ++i) {
            if (!jsonData[i].is_array() || jsonData[i].size() != cols) {
                throw std::runtime_error("Inconsistent row size in JSON array.");
            }
            for (size_t j = 0; j < cols; ++j) {
                matrix(i, j) = jsonData[i][j];
            }
        }
        return matrix;
    } catch (const std::exception& e) {
        logTrainingProcess("Error converting JSON to matrix: " + std::string(e.what()), LogLevel::ERROR);
        throw;
    }
}

bool MLModelTrainer::loadTrainingDataFromDictionary(const std::string& dictionaryPath) {
    try {
        if (!dictionaryLoader.load(dictionaryPath)) {
            throw std::runtime_error("Failed to load dictionary from file: " + dictionaryPath);
        }

        std::vector<std::string> words = dictionaryLoader.getAllWords();

        trainingData.set_size(words.size(), words[0].size());
        labels.set_size(words.size());

        for (size_t i = 0; i < words.size(); ++i) {
            for (size_t j = 0; j < words[i].size(); ++j) {
                trainingData(i, j) = static_cast<double>(words[i][j]);
            }
            labels[i] = words[i].size();
        }

        logTrainingProcess("Training data loaded from dictionary.");
        return true;
    } catch (const std::exception& e) {
        logTrainingProcess("Error loading training data from dictionary: " + std::string(e.what()), LogLevel::ERROR);
        return false;
    }
}

double MLModelTrainer::evaluateModel(const std::string& testDataPath, const std::string& format) {
    try {
        arma::mat testData;
        arma::Row<size_t> testLabels;

        if (format == "csv") {
            auto csvData = dbManager.parseAndValidateCSV(testDataPath, {/**/});
            testData = convertCSVToMatrix(csvData);
        } else if (format == "json") {
            auto jsonData = dbManager.parseAndValidateJSON(testDataPath, {/**/});
            testData = convertJSONToMatrix(jsonData);
        } else {
            throw std::runtime_error("Unsupported data format: " + format);
        }

        testLabels = arma::conv_to<arma::Row<size_t>>::from(testData.row(testData.n_rows - 1));
        testData.shed_row(testData.n_rows - 1);  

        arma::Row<size_t> predictions;
        if (!logisticModel && !decisionTreeModel && !neuralNetworkModel && !knnModel &&
        !naiveBayesModel && !svmModel && !randomForestModel && !gradientBoostingModel) {
        throw std::runtime_error("No trained model available.");
        }
        if (logisticModel) {
            logisticModel->Classify(testData, predictions);
        } else if (decisionTreeModel) {
            decisionTreeModel->Classify(testData, predictions);
        } else if (neuralNetworkModel) {
            arma::mat predProbs;
            neuralNetworkModel->Predict(testData, predProbs);
            predictions = arma::conv_to<arma::Row<size_t>>::from(arma::round(predProbs));
        } else if (knnModel) {
            knnModel->Classify(testData, predictions, 3);
        } else if (naiveBayesModel) {
            naiveBayesModel->Classify(testData, predictions);
        } else if (svmModel) {
            svmModel->Classify(testData, predictions);
        } else if (randomForestModel) {
            randomForestModel->Classify(testData, predictions);
        } else {
            throw std::runtime_error("No trained model available for evaluation.");
        }

        double accuracy = arma::accu(predictions == testLabels) / static_cast<double>(testLabels.n_elem);
        logTrainingProcess("Model evaluation completed. Accuracy: " + std::to_string(accuracy));
        return accuracy;
    } catch (const std::exception& e) {
        logTrainingProcess("Error evaluating model: " + std::string(e.what()), LogLevel::ERROR);
        return 0.0;
    }
}

void MLModelTrainer::preprocessData() {
    applyRulesToData();
    DataUtils::normalize(trainingData);
    logTrainingProcess("Data normalized.");
    dbManager.logDBOperation("PreprocessData", "Data normalized.");
    monitor.monitorMLTraining("MLModelTrainer", "Data preprocessing completed");
}

void MLModelTrainer::applyRulesToData() {
    for (size_t i = 0; i < trainingData.n_cols; ++i) {
        std::string originalWord = std::to_string(labels[i]);
        auto transformedWords = ruleEngine.applyRules(originalWord);

        int bestTransformedLabel = labels[i];
        bool transformationApplied = false;
        double bestScore = std::numeric_limits<double>::lowest();

        for (const auto& transformedWord : transformedWords) {
            try {
                int transformedLabel = std::stoi(transformedWord);
                double currentScore = evaluateTransformationQuality(originalWord, transformedWord);

                if (currentScore > bestScore) {
                    bestScore = currentScore;
                    bestTransformedLabel = transformedLabel;
                    transformationApplied = true;
                }
            } catch (const std::exception& e) {
                logTrainingProcess("Failed to convert transformed word: " + transformedWord + " for label: " + originalWord, LogLevel::WARNING);
            }
        }

        if (transformationApplied) {
            labels[i] = bestTransformedLabel;
            logTrainingProcess("Label " + originalWord + " transformed to " + std::to_string(bestTransformedLabel));
        } else {
            logTrainingProcess("No valid transformation found for label: " + originalWord, LogLevel::WARNING);
        }
    }
}

double MLModelTrainer::evaluateTransformationQuality(const std::string& originalWord, const std::string& transformedWord) {
    int distance = calculateLevenshteinDistance(originalWord, transformedWord);
    return 1.0 / (1.0 + distance);
}

int MLModelTrainer::calculateLevenshteinDistance(const std::string &a, const std::string &b) {
    const size_t len1 = a.size(), len2 = b.size();
    std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

    d[0][0] = 0;
    for (unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
    for (unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

    for (unsigned int i = 1; i <= len1; ++i)
        for (unsigned int j = 1; j <= len2; ++j)
            d[i][j] = std::min({ d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1) });

    return d[len1][len2];
}

void MLModelTrainer::analyzeErrors(const std::string& testDataPath, const std::string& format) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Analyzing errors");
        arma::mat testData;
        arma::Row<size_t> testLabels;

        if (format == "csv") {
            auto csvData = dbManager.parseAndValidateCSV(testDataPath, {/* headers */});
            testData = convertCSVToMatrix(csvData);
        } else if (format == "json") {
            auto jsonData = dbManager.parseAndValidateJSON(testDataPath, {/* schema */});
            testData = convertJSONToMatrix(jsonData);
        } else {
            throw std::runtime_error("Unsupported data format: " + format);
        }

        testLabels = arma::conv_to<arma::Row<size_t>>::from(testData.row(testData.n_rows - 1));
        testData.shed_row(testData.n_rows - 1);

        arma::Row<size_t> predictions;

        if (logisticModel) {
            logisticModel->Classify(testData, predictions);
        } else if (decisionTreeModel) {
            decisionTreeModel->Classify(testData, predictions);
        } else if (neuralNetworkModel) {
            arma::mat predProbs;
            neuralNetworkModel->Predict(testData, predProbs);
            predictions = arma::conv_to<arma::Row<size_t>>::from(arma::round(predProbs));
        } else if (knnModel) {
            knnModel->Classify(testData, predictions, 3);
        } else if (naiveBayesModel) {
            naiveBayesModel->Classify(testData, predictions);
        } else if (svmModel) {
            svmModel->Classify(testData, predictions);
        } else if (randomForestModel) {
            randomForestModel->Classify(testData, predictions);
        } else if (gradientBoostingModel) {
            gradientBoostingModel->Classify(testData, predictions);
        } else {
            throw std::runtime_error("No trained model available for error analysis.");
        }

        arma::uvec falsePositives = arma::find(predictions == 1 && testLabels == 0);
        arma::uvec falseNegatives = arma::find(predictions == 0 && testLabels == 1);

        std::ostringstream oss;
        oss << "False positives: " << falsePositives.n_elem << ", False negatives: " << falseNegatives.n_elem;
        logTrainingProcess(oss.str());
        dbManager.logDBOperation("AnalyzeErrors", oss.str());

        logTrainingProcess("Error analysis completed.");
        monitor.monitorMLTraining("MLModelTrainer", "Error analysis completed");
    } catch (const std::exception& e) {
        logTrainingProcess("Error analyzing model errors: " + std::string(e.what()), LogLevel::ERROR);
        dbManager.logDBError("AnalyzeErrors failed: " + std::string(e.what()));
        monitor.monitorMLTraining("MLModelTrainer", "Error analyzing model errors");
    }
}

bool MLModelTrainer::backupModel(const std::string& modelPath) {
    monitor.monitorMLTraining("MLModelTrainer", "Starting model backup");
    try {
        if (cloudIntegration.uploadData(modelPath, config.getBackupCloudPath())) {
            logTrainingProcess("Model successfully backed up to cloud.", LogLevel::INFO);
            monitor.monitorMLTraining("MLModelTrainer", "Model backup completed");
            return true;
        } else {
            throw std::runtime_error("Failed to upload model to cloud.");
        }
    } catch (const std::exception& e) {
        logTrainingProcess("Error during model backup: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error during model backup");
        return false;
    }
}

bool MLModelTrainer::loadModelFromCloud(const std::string& cloudModelPath, const std::string& localModelPath) {
    monitor.monitorMLTraining("MLModelTrainer", "Starting model download from cloud");
    try {
        if (cloudIntegration.downloadData(cloudModelPath, localModelPath)) {
            logTrainingProcess("Model successfully downloaded from cloud.", LogLevel::INFO);
            monitor.monitorMLTraining("MLModelTrainer", "Model download completed");
            return true;
        } else {
            throw std::runtime_error("Failed to download model from cloud.");
        }
    } catch (const std::exception& e) {
        logTrainingProcess("Error during model download: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error during model download");
        return false;
    }
}

void MLModelTrainer::handleClassImbalance() {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Handling class imbalance");
        arma::Row<size_t> classCounts = arma::histc(labels, arma::unique(labels));

        for (size_t i = 0; i < classCounts.n_elem; ++i) {
            size_t majorityClass = arma::index_max(classCounts);
            size_t minorityClass = i;

            if (classCounts[i] < classCounts[majorityClass]) {
                size_t difference = classCounts[majorityClass] - classCounts[i];

                arma::uvec indices = arma::find(labels == i);
                arma::mat samples = trainingData.cols(indices);

                for (size_t j = 0; j < difference; ++j) {
                    size_t randIndex = arma::randi<arma::uword>(arma::distr_param(0, indices.n_elem - 1));
                    trainingData.insert_cols(trainingData.n_cols, samples.col(randIndex));
                    labels.insert_cols(labels.n_cols, arma::Row<size_t>({i}));
                }
            }
        }

        logTrainingProcess("Class imbalance handled successfully.");
        dbManager.logDBOperation("HandleClassImbalance", "Class imbalance handled.");
        monitor.monitorMLTraining("MLModelTrainer", "Class imbalance handled successfully");
    } catch (const std::exception& e) {
        logTrainingProcess("Error handling class imbalance: " + std::string(e.what()), LogLevel::ERROR);
        dbManager.logDBError("HandleClassImbalance failed: " + std::string(e.what()));
        monitor.monitorMLTraining("MLModelTrainer", "Error handling class imbalance");
    }
}

void MLModelTrainer::manageResources() {
    std::map<ModelType, std::unique_ptr<Model>> models = {
        {ModelType::LogisticRegression, std::move(logisticModel)},
        {ModelType::DecisionTree, std::move(decisionTreeModel)},
        {ModelType::NeuralNetwork, std::move(neuralNetworkModel)},
        {ModelType::KNN, std::move(knnModel)},
        {ModelType::NaiveBayes, std::move(naiveBayesModel)},
        {ModelType::SVM, std::move(svmModel)},
        {ModelType::RandomForest, std::move(randomForestModel)},
        {ModelType::GradientBoosting, std::move(gradientBoostingModel)}
    };

    try {
        monitor.monitorMLTraining("MLModelTrainer", "Managing resources");
        for (auto& model : models) {
            model.second.reset();
        }
        gpuManager.optimizeMemoryUsage();
        logTrainingProcess("Resource management completed.");
        dbManager.logDBOperation("ManageResources", "Resource management completed.");
        monitor.monitorMLTraining("MLModelTrainer", "Resource management completed");
    } catch (const std::exception& e) {
        logTrainingProcess("Error managing resources: " + std::string(e.what()), LogLevel::ERROR);
        dbManager.logDBError("ManageResources failed: " + std::string(e.what()));
        monitor.monitorMLTraining("MLModelTrainer", "Error managing resources");
    }
}

void MLModelTrainer::generateReport(const std::string& reportPath) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Generating report");
        if (trainingData.empty()) {
            throw std::runtime_error("Training data is empty. Cannot generate report.");
        }

        std::ofstream reportFile(reportPath);
        if (!reportFile.is_open()) {
            throw std::runtime_error("Cannot open report file: " + reportPath);
        }

        reportFile << "Model Training Report\n";
        reportFile << "=====================\n";
        reportFile << "Training Data Size: " << trainingData.n_rows << " samples\n";
        reportFile << "Feature Count: " << trainingData.n_cols << "\n";
        reportFile << "Labels: " << arma::unique(labels).t() << "\n";
        reportFile << "Evaluation Metrics: \n";
        reportFile << "Accuracy: " << evaluateModel(reportPath, "csv") << "\n";

        reportFile.close();
        logTrainingProcess("Report generated at " + reportPath);
        dbManager.logDBOperation("GenerateReport", "Report generated at " + reportPath);
        monitor.monitorMLTraining("MLModelTrainer", "Report generation completed");
    } catch (const std::exception& e) {
        logTrainingProcess("Error generating report: " + std::string(e.what()), LogLevel::ERROR);
        dbManager.logDBError("GenerateReport failed: " + std::string(e.what()));
        monitor.monitorMLTraining("MLModelTrainer", "Error generating report");
    }
}

bool MLModelTrainer::uploadModelToCloud(const std::string& localModelPath, const std::string& cloudModelPath) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Uploading model to cloud");
        bool success = cloudIntegration.uploadData(localModelPath, cloudModelPath);
        if (success) {
            logTrainingProcess("Model successfully uploaded to cloud: " + cloudModelPath);
            monitor.monitorMLTraining("MLModelTrainer", "Model uploaded to cloud successfully");
        } else {
            throw std::runtime_error("Failed to upload model to cloud");
        }
        return success;
    } catch (const std::exception& e) {
        logTrainingProcess("Error uploading model to cloud: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error uploading model to cloud");
        return false;
    }
}

bool MLModelTrainer::downloadModelFromCloud(const std::string& cloudModelPath, const std::string& localModelPath) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Downloading model from cloud");
        bool success = cloudIntegration.downloadData(cloudModelPath, localModelPath);
        if (success) {
            logTrainingProcess("Model successfully downloaded from cloud: " + cloudModelPath);
            monitor.monitorMLTraining("MLModelTrainer", "Model downloaded from cloud successfully");
        } else {
            throw std::runtime_error("Failed to download model from cloud");
        }
        return success;
    } catch (const std::exception& e) {
        logTrainingProcess("Error downloading model from cloud: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error downloading model from cloud");
        return false;
    }
}

bool MLModelTrainer::validateData() const {
    if (trainingData.empty() || labels.empty() || trainingData.n_cols != labels.n_cols) {
        return false;
    }

    for (size_t i = 0; i < trainingData.n_cols; ++i) {
        if (arma::any(trainingData.col(i).is_nan())) {
            return false;
        }
    }

    return true;
}

void MLModelTrainer::plotData() const {
    if (trainingData.n_cols < 2) {
        logTrainingProcess("Cannot plot data: insufficient columns", LogLevel::WARNING);
        return;
    }
    namespace plt = matplotlibcpp;
    std::vector<double> x(trainingData.col(0).begin(), trainingData.col(0).end());
    std::vector<double> y(trainingData.col(1).begin(), trainingData.col(1).end());
    plt::scatter(x, y);
    plt::show();
}

void MLModelTrainer::logTrainingProcess(const std::string& message, LogLevel level) {
    Logger::log(message, level, {"MLModelTrainer"});
}

bool MLModelTrainer::trainModel(ModelType modelType, const std::map<std::string, double>& hyperparameters) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Training model: " + std::to_string(static_cast<int>(modelType)));

        switch (modelType) {
            case ModelType::LogisticRegression:
                logisticModel = std::make_unique<mlpack::regression::LogisticRegression<>>();
                logisticModel->Train(trainingData, labels);
                if (hyperparameters.find("lambda") != hyperparameters.end()) {
                 double lambda = hyperparameters.at("lambda");
                if (lambda < 0.0 || lambda > 1.0) {
                  throw std::runtime_error("Invalid lambda value. Must be between 0 and 1.");
                }
                logisticModel->Lambda(lambda);
                 }

                break;
            case ModelType::DecisionTree:
                decisionTreeModel = std::make_unique<mlpack::tree::DecisionTree<>>();
                decisionTreeModel->Train(trainingData, labels);
                break;
            case ModelType::NeuralNetwork:
                neuralNetworkModel = std::make_unique<mlpack::ann::FFN<mlpack::ann::CrossEntropyError<>, mlpack::ann::RandomInitialization>>();
                if (hyperparameters.find("hiddenUnits") != hyperparameters.end()) {
                    neuralNetworkModel->Add<mlpack::ann::Linear<>>(trainingData.n_rows, static_cast<int>(hyperparameters.at("hiddenUnits")));
                    neuralNetworkModel->Add<mlpack::ann::ReLULayer<>>();
                    neuralNetworkModel->Add<mlpack::ann::Linear<>>(static_cast<int>(hyperparameters.at("hiddenUnits")), labels.max() + 1);
                    neuralNetworkModel->Add<mlpack::ann::Softmax<>>();
                }
                neuralNetworkModel->Train(trainingData, labels);
                break;
            case ModelType::KNN:
                knnModel = std::make_unique<mlpack::knn::KNN>();
                knnModel->Train(trainingData, labels);
                break;
            case ModelType::NaiveBayes:
                naiveBayesModel = std::make_unique<mlpack::naive_bayes::NaiveBayesClassifier<>>();
                naiveBayesModel->Train(trainingData, labels);
                break;
            case ModelType::SVM:
                svmModel = std::make_unique<mlpack::svm::SVM<>>();
                if (hyperparameters.find("C") != hyperparameters.end()) {
                    svmModel->C(hyperparameters.at("C"));
                }
                svmModel->Train(trainingData, labels);
                break;
            case ModelType::RandomForest:
                randomForestModel = std::make_unique<mlpack::tree::RandomForest<>>();
                randomForestModel->Train(trainingData, labels);
                break;
            case ModelType::GradientBoosting:
                gradientBoostingModel = std::make_unique<mlpack::adaboost::AdaBoost<mlpack::tree::DecisionTree<>>>();
                gradientBoostingModel->Train(trainingData, labels);
                break;
            default:
                throw std::invalid_argument("Unsupported model type.");
        }

        logTrainingProcess("Model trained successfully.");
        monitor.monitorMLTraining("MLModelTrainer", "Model training completed");
        return true;
    } catch (const std::exception& e) {
        logTrainingProcess("Error training model: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error training model");
        return false;
    }
}

bool MLModelTrainer::saveModel(const std::string& modelPath) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Saving model to: " + modelPath);

        if (logisticModel) {
            mlpack::data::Save(modelPath, "logisticModel", *logisticModel);
        } else if (decisionTreeModel) {
            mlpack::data::Save(modelPath, "decisionTreeModel", *decisionTreeModel);
        } else if (neuralNetworkModel) {
            mlpack::data::Save(modelPath, "neuralNetworkModel", *neuralNetworkModel);
        } else if (knnModel) {
            mlpack::data::Save(modelPath, "knnModel", *knnModel);
        } else if (naiveBayesModel) {
            mlpack::data::Save(modelPath, "naiveBayesModel", *naiveBayesModel);
        } else if (svmModel) {
            mlpack::data::Save(modelPath, "svmModel", *svmModel);
        } else if (randomForestModel) {
            mlpack::data::Save(modelPath, "randomForestModel", *randomForestModel);
        } else if (gradientBoostingModel) {
            mlpack::data::Save(modelPath, "gradientBoostingModel", *gradientBoostingModel);
        } else {
            throw std::runtime_error("No model to save.");
        }

        logTrainingProcess("Model saved successfully.");
        monitor.monitorMLTraining("MLModelTrainer", "Model saved successfully");
        return true;
    } catch (const std::exception& e) {
        logTrainingProcess("Error saving model: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error saving model");
        return false;
    }
}

bool MLModelTrainer::loadModel(const std::string& modelPath, ModelType modelType) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Loading model from: " + modelPath);

        switch (modelType) {
            case ModelType::LogisticRegression:
                logisticModel = std::make_unique<mlpack::regression::LogisticRegression<>>();
                mlpack::data::Load(modelPath, "logisticModel", *logisticModel);
                break;
            case ModelType::DecisionTree:
                decisionTreeModel = std::make_unique<mlpack::tree::DecisionTree<>>();
                mlpack::data::Load(modelPath, "decisionTreeModel", *decisionTreeModel);
                break;
            case ModelType::NeuralNetwork:
                neuralNetworkModel = std::make_unique<mlpack::ann::FFN<mlpack::ann::CrossEntropyError<>, mlpack::ann::RandomInitialization>>();
                mlpack::data::Load(modelPath, "neuralNetworkModel", *neuralNetworkModel);
                break;
            case ModelType::KNN:
                knnModel = std::make_unique<mlpack::knn::KNN>();
                mlpack::data::Load(modelPath, "knnModel", *knnModel);
                break;
            case ModelType::NaiveBayes:
                naiveBayesModel = std::make_unique<mlpack::naive_bayes::NaiveBayesClassifier<>>();
                mlpack::data::Load(modelPath, "naiveBayesModel", *naiveBayesModel);
                break;
            case ModelType::SVM:
                svmModel = std::make_unique<mlpack::svm::SVM<>>();
                mlpack::data::Load(modelPath, "svmModel", *svmModel);
                break;
            case ModelType::RandomForest:
                randomForestModel = std::make_unique<mlpack::tree::RandomForest<>>();
                mlpack::data::Load(modelPath, "randomForestModel", *randomForestModel);
                break;
            case ModelType::GradientBoosting:
                gradientBoostingModel = std::make_unique<mlpack::adaboost::AdaBoost<mlpack::tree::DecisionTree<>>>();
                mlpack::data::Load(modelPath, "gradientBoostingModel", *gradientBoostingModel);
                break;
            default:
                throw std::invalid_argument("Unsupported model type.");
        }

        logTrainingProcess("Model loaded successfully.");
        monitor.monitorMLTraining("MLModelTrainer", "Model loaded successfully");
        return true;
    } catch (const std::exception& e) {
        logTrainingProcess("Error loading model: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error loading model");
        return false;
    }
}

bool MLModelTrainer::tuneHyperparameters(ModelType modelType, const std::map<std::string, double>& hyperparameters) {
    try {
        monitor.monitorMLTraining("MLModelTrainer", "Tuning hyperparameters for model: " + std::to_string(static_cast<int>(modelType)));
        
        switch (modelType) {
            case ModelType::LogisticRegression:
                if (logisticModel) {
                    if (hyperparameters.find("lambda") != hyperparameters.end()) {
                        logisticModel->Lambda(hyperparameters.at("lambda"));
                    }
                }
                break;
            case ModelType::SVM:
                if (svmModel) {
                    if (hyperparameters.find("C") != hyperparameters.end()) {
                        svmModel->C(hyperparameters.at("C"));
                    }
                    if (hyperparameters.find("kernel") != hyperparameters.end()) {
                        svmModel->KernelType(static_cast<mlpack::svm::KernelTypes>(hyperparameters.at("kernel")));
                    }
                }
                break;
            case ModelType::NeuralNetwork:
                if (neuralNetworkModel) {
                    if (hyperparameters.find("learningRate") != hyperparameters.end()) {
                        neuralNetworkModel->Optimizer().StepSize(hyperparameters.at("learningRate"));
                    }
                    if (hyperparameters.find("epochs") != hyperparameters.end()) {
                        neuralNetworkModel->Optimizer().MaxIterations(hyperparameters.at("epochs"));
                    }
                }
                break;
            case ModelType::RandomForest:
                if (randomForestModel) {
                    if (hyperparameters.find("numTrees") != hyperparameters.end()) {
                        randomForestModel->NumTrees(static_cast<size_t>(hyperparameters.at("numTrees")));
                    }
                    if (hyperparameters.find("minLeafSize") != hyperparameters.end()) {
                        randomForestModel->MinLeafSize(static_cast<size_t>(hyperparameters.at("minLeafSize")));
                    }
                }
                break;
            default:
                throw std::invalid_argument("Unsupported model type for hyperparameter tuning.");
        }

        logTrainingProcess("Hyperparameters tuned successfully.");
        monitor.monitorMLTraining("MLModelTrainer", "Hyperparameter tuning completed");
        return true;
    } catch (const std::exception& e) {
        logTrainingProcess("Error tuning hyperparameters: " + std::string(e.what()), LogLevel::ERROR);
        monitor.monitorMLTraining("MLModelTrainer", "Error tuning hyperparameters");
        return false;
    }
}

void MLModelTrainer::interpretModel() {
    logTrainingProcess("Interpreting model...");
    if (logisticModel) {
        arma::mat parameters = logisticModel->Parameters();
        std::cout << "Logistic regression parameters: " << parameters << std::endl;
    } else if (decisionTreeModel) {
        std::cout << "Decision tree depth: " << decisionTreeModel->Depth() << std::endl;
        std::cout << "Number of nodes: " << decisionTreeModel->NumNodes() << std::endl;
    } else if (randomForestModel) {
        std::cout << "Random forest - number of trees: " << randomForestModel->NumTrees() << std::endl;
    }
    logTrainingProcess("Model interpretation completed.");
}

void MLModelTrainer::visualizeFeatureImportance() {
    logTrainingProcess("Visualizing feature importance...");
    if (randomForestModel) {
        arma::rowvec importances = randomForestModel->FeatureImportance();
        std::cout << "Feature importance: " << importances << std::endl;
    } else if (decisionTreeModel) {
        arma::rowvec importances = decisionTreeModel->FeatureImportance();
        std::cout << "Feature importance: " << importances << std::endl;
    } else {
        std::cout << "Feature importance visualization is not supported for this model type." << std::endl;
    }
    logTrainingProcess("Feature importance visualized.");
}

double MLModelTrainer::crossValidate(ModelType modelType, int folds) {
    logTrainingProcess("Performing cross-validation...");
    double avgAccuracy = 0.0;
    
    switch (modelType) {
        case ModelType::LogisticRegression:
            if (logisticModel) {
                mlpack::cv::KFoldCV<mlpack::regression::LogisticRegression<>, mlpack::cv::Accuracy> cv(folds);
                avgAccuracy = cv.Evaluate(*logisticModel, trainingData, labels);
            }
            break;
        case ModelType::DecisionTree:
            if (decisionTreeModel) {
                mlpack::cv::KFoldCV<mlpack::tree::DecisionTree<>, mlpack::cv::Accuracy> cv(folds);
                avgAccuracy = cv.Evaluate(*decisionTreeModel, trainingData, labels);
            }
            break;
        case ModelType::NeuralNetwork:
            if (neuralNetworkModel) {
                mlpack::cv::KFoldCV<mlpack::ann::FFN<mlpack::ann::CrossEntropyError<>, mlpack::ann::RandomInitialization>, mlpack::cv::Accuracy> cv(folds);
                avgAccuracy = cv.Evaluate(*neuralNetworkModel, trainingData, labels);
            }
            break;
        case ModelType::RandomForest:
            if (randomForestModel) {
                mlpack::cv::KFoldCV<mlpack::tree::RandomForest<>, mlpack::cv::Accuracy> cv(folds);
                avgAccuracy = cv.Evaluate(*randomForestModel, trainingData, labels);
            }
            break;
        default:
            throw std::invalid_argument("Unsupported model type for cross-validation.");
    }

    logTrainingProcess("Cross-validation completed with accuracy: " + std::to_string(avgAccuracy));
    return avgAccuracy;
}

































