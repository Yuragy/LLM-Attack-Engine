#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include "cli/cli.h"
#include "config/config.h"
#include "logging/logger.h"
#include "dictionary/dictionary_loader.h"
#include "rules/rule_engine.h"
#include "attack/attack_engine.h"
#include "gpu/gpu_manager.h"
#include "database/db_manager.h"
#include "web_app/app.h"
#include "scheduling/scheduler.h"
#include "monitoring/monitor.h"
#include "cloud/cloud_integration.h"
#include "users/user_management.h"
#include "notifications/notification_manager.h"
#include "analytics/analytics_manager.h"
#include "machine_learning/ml_model_trainer.h"
#include "machine_learning/ml_predictor.h"
#include "policy/policy_manager.h"
#include "adaptive_attack/adaptive_attack_manager.h"
#include "utils/threading_utils.h"
#include "utils/gpu_utils.h"
#include "utils/data_utils.h"

int main(int argc, char* argv[]) {
    try {
        Config config;
        if (!config.load("config.json")) {
            std::cerr << "Не удалось загрузить конфигурационный файл" << std::endl;
            return 1;
        }

        Logger::initialize(config.getLogFilePath());
        Logger::info("Программа запущена");

        CLI cli(argc, argv);
        if (!cli.parseArguments()) {
            return 1;
        }

        std::string command = cli.getCommand();
        std::vector<std::string> arguments = cli.getArguments();

        if (!config.validate()) {
            Logger::error("Конфигурационный файл содержит ошибки");
            return 1;
        }
        Logger::info("Конфигурационный файл загружен и валидирован");

        auto dataUtils = std::make_shared<DataUtils>();
        auto logger = std::make_shared<Logger>();
        auto userManager = std::make_shared<UserManagement>();
        auto scheduler = std::make_shared<Scheduler>();
        auto monitor = std::make_shared<Monitor>();
        auto ruleEngine = std::make_shared<RuleEngine>();
        auto notificationManager = std::make_shared<NotificationManager>();
        auto cloudIntegration = std::make_shared<CloudIntegration>();
        auto threadingUtils = std::make_shared<ThreadingUtils>();
        auto gpuUtils = std::make_shared<GPUUtils>();

        DBManager dbManager(config.getDBConnectionString(), dataUtils, logger, userManager, scheduler, monitor, ruleEngine);
        if (!dbManager.connect()) {
            Logger::error("Не удалось подключиться к базе данных");
            return 1;
        }
        Logger::info("Подключение к базе данных успешно");

        DictionaryLoader dictLoader;
        std::vector<std::string> dictionaryPaths = config.getDictionaryPaths();
        auto dictLoadFuture = dictLoader.loadMultipleAsync(dictionaryPaths);

        if (!ruleEngine->loadRules(config.getRulesPath())) {
            Logger::error("Не удалось загрузить правила трансформации паролей");
            return 1;
        }
        Logger::info("Правила трансформации паролей загружены");

        GPUManager gpuManager;
        if (!gpuManager.initialize()) {
            Logger::error("Не удалось инициализировать управление видеокартами");
            return 1;
        }
        Logger::info("Управление видеокартами инициализировано");

        try {
            gpuManager.startMonitoring();
            Logger::info("Мониторинг состояния GPU запущен");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при запуске мониторинга состояния GPU: " + std::string(e.what()));
            return 1;
        }

        try {
            gpuManager.optimizeMemory();
            Logger::info("Оптимизация памяти GPU выполнена");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при оптимизации памяти GPU: " + std::string(e.what()));
            return 1;
        }

        try {
            gpuManager.managePower();
            Logger::info("Управление энергопотреблением GPU выполнено");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при управлении энергопотреблением GPU: " + std::string(e.what()));
            return 1;
        }

        AttackEngine attackEngine(&dictLoader, ruleEngine.get(), &gpuManager);
        if (!attackEngine.setup(config.getAttackConfig())) {
            Logger::error("Не удалось настроить движок атак");
            return 1;
        }
        Logger::info("Движок атак настроен");

        if (!monitor->initialize(config.getMonitoringConfig())) {
            Logger::error("Не удалось инициализировать мониторинг");
            return 1;
        }
        Logger::info("Мониторинг инициализирован");
        monitor->startMonitoring();

        MLModelTrainer mlModelTrainer;
        if (!mlModelTrainer.initialize(config.getMLConfig())) {
            Logger::error("Не удалось инициализировать тренер моделей машинного обучения");
            return 1;
        }
        Logger::info("Тренер моделей машинного обучения инициализирован");

        MLPredictor mlPredictor;
        if (!scheduler->initialize(config.getSchedulerConfig())) {
            Logger::error("Не удалось инициализировать планировщик задач");
            return 1;
        }
        Logger::info("Планировщик задач инициализирован");

        WebApp webApp;
        if (!webApp.initialize(config.getWebAppConfig())) {
            Logger::error("Не удалось инициализировать веб-приложение");
            return 1;
        }
        Logger::info("Веб-приложение инициализировано");

        std::thread webAppThread([&webApp]() {
            webApp.run();
        });

        if (!cloudIntegration->initialize(config.getCloudConfig())) {
            Logger::error("Не удалось инициализировать облачную интеграцию");
            return 1;
        }
        Logger::info("Облачная интеграция инициализирована");

        try {
            cloudIntegration->uploadFile("local_file.txt", "remote_file.txt");
            Logger::info("Файл успешно загружен в облако");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при загрузке файла в облако: " + std::string(e.what()));
        }

        try {
            cloudIntegration->downloadFile("remote_file.txt", "local_file_downloaded.txt");
            Logger::info("Файл успешно скачан из облака");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при скачивании файла из облака: " + std::string(e.what()));
        }
        try {
            cloudIntegration->createBackup("backup_file.bak");
            Logger::info("Резервная копия успешно создана в облаке");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при создании резервной копии в облаке: " + std::string(e.what()));
        }

        try {
            cloudIntegration->restoreBackup("backup_file.bak");
            Logger::info("Резервная копия успешно восстановлена из облака");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при восстановлении резервной копии из облака: " + std::string(e.what()));
        }
        if (!userManager->initialize(config.getUserManagementConfig())) {
            Logger::error("Не удалось инициализировать управление пользователями");
            return 1;
        }
        Logger::info("Управление пользователями инициализировано");
        if (!notificationManager->initialize(config.getNotificationConfig())) {
            Logger::error("Не удалось инициализировать модуль уведомлений");
            return 1;
        }
        Logger::info("Модуль уведомлений инициализирован");
        try {
            notificationManager->sendNotification("Test notification");
            Logger::info("Уведомление успешно отправлено");
        } catch (const std::exception& e) {
            Logger::error("Ошибка при отправке уведомления: " + std::string(e.what()));
        }
        AnalyticsManager analyticsManager;
        if (!analyticsManager.initialize(config.getAnalyticsConfig())) {
            Logger::error("Не удалось инициализировать модуль аналитики");
            return 1;
        }
        Logger::info("Модуль аналитики инициализирован");
        PolicyManager policyManager(config.getPolicyConfigPath());
        if (!policyManager.loadPolicies()) {
            Logger::error("Не удалось загрузить политики безопасности");
            return 1;
        }
        Logger::info("Политики безопасности загружены");
        AdaptiveAttackManager adaptiveManager(&attackEngine, &analyticsManager, monitor.get(), &mlModelTrainer, &policyManager);
        if (!adaptiveManager.initialize(config.getAdaptiveAttackConfig())) {
            Logger::error("Не удалось инициализировать адаптивный менеджер атак");
            return 1;
        }
        Logger::info("Адаптивный менеджер атак инициализирован");
        if (!dictLoadFuture.get()) {
            Logger::error("Ошибка при загрузке словарей паролей");
            return 1;
        }
        Logger::info("Словари паролей успешно загружены");

        if (command == "start" || command == "stop" || command == "status") {
            Logger::info("Выполнение команды: " + command);
            if (!cli.handleAttackCommand(adaptiveManager)) { 
                Logger::error("Ошибка при выполнении команды управления атакой");
                return 1;
            }
            Logger::info("Команда успешно выполнена: " + command);
        } else if (command == "config") {
            Logger::info("Выполнение команды: config");
            if (!cli.handleConfigCommand(config)) {
                Logger::error("Ошибка при выполнении команды управления конфигурацией");
                return 1;
            }
            Logger::info("Команда успешно выполнена: config");
        } else if (command == "dictionary") {
            Logger::info("Выполнение команды: dictionary");
            if (!cli.handleDictionaryCommand(dictLoader)) {
                Logger::error("Ошибка при выполнении команды управления словарями");
                return 1;
            }
            Logger::info("Команда успешно выполнена: dictionary");
        } else if (command == "rule") {
            Logger::info("Выполнение команды: rule");
            if (!cli.handleRuleCommand(*ruleEngine)) {
                Logger::error("Ошибка при выполнении команды управления правилами");
                return 1;
            }
            Logger::info("Команда успешно выполнена: rule");
        } else if (command == "user") {
            Logger::info("Выполнение команды: user");
            if (!cli.handleUserCommand(*userManager)) {
                Logger::error("Ошибка при выполнении команды управления пользователями");
                return 1;
            }
            Logger::info("Команда успешно выполнена: user");
        } else if (command == "schedule") {
            Logger::info("Выполнение команды: schedule");
            if (!cli.handleScheduleCommand(*scheduler)) {
                Logger::error("Ошибка при выполнении команды управления расписанием задач");
                return 1;
            }
            Logger::info("Команда успешно выполнена: schedule");
        } else if (command == "notification") {
            Logger::info("Выполнение команды: notification");
            if (!cli.handleNotificationCommand(*notificationManager)) {
                Logger::error("Ошибка при выполнении команды управления уведомлениями");
                return 1;
            }
            Logger::info("Команда успешно выполнена: notification");
        } else if (command == "analytics") {
            Logger::info("Выполнение команды: analytics");
            if (!cli.handleAnalyticsCommand(analyticsManager)) {
                Logger::error("Ошибка при выполнении команды управления аналитикой");
                return 1;
            }
            Logger::info("Команда успешно выполнена: analytics");
        } else if (command == "train") {
            Logger::info("Выполнение команды: train");
            try {
                mlModelTrainer.loadData("data/training_data.csv");
                mlModelTrainer.trainModel();
                mlModelTrainer.saveModel("models/password_predictor.model");
                Logger::info("Модель успешно обучена и сохранена");
            } catch (const std::exception& e) {
                Logger::error("Ошибка при обучении модели: " + std::string(e.what()));
                return 1;
            }
        } else if (command == "predict") {
            Logger::info("Выполнение команды: predict");
            try {
                if (!mlPredictor.loadModel("models/password_predictor.model", ModelType::NeuralNetwork)) {
                    Logger::error("Не удалось загрузить модель для предсказаний");
                    return 1;
                }
                arma::mat inputData;
                mlpack::data::Load("data/input_data.csv", inputData, true);
                arma::Row<size_t> predictions = mlPredictor.predict(inputData);
                mlPredictor.savePredictions("data/predictions.csv", predictions);
                Logger::info("Предсказания успешно выполнены и сохранены");
            } catch (const std::exception& e) {
                Logger::error("Ошибка при выполнении предсказаний: " + std::string(e.what()));
                return 1;
            }
        } else if (command == "help") {
            cli.displayHelp();
        } else {
            std::cerr << "Неизвестная команда: " << command << std::endl;
            cli.displayHelp();
            return 1;
        }
        if (webAppThread.joinable()) {
            webAppThread.join();
        }
        
        Logger::shutdown();

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Необработанное исключение: " + std::string(e.what()));
        Logger::shutdown();
        return 1;
    } catch (...) {
        Logger::error("Необработанное неизвестное исключение");
        Logger::shutdown();
        return 1;
    }
}



























