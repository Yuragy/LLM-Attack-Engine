#include "cli.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cstring>
#include "../rules/rule_engine.h"
#include "../users/user_management.h"
#include "../scheduling/scheduler.h"
#include "../notifications/notification_manager.h"
#include "../monitoring/monitor.h"
#include "../analytics/analytics_manager.h"
#include "../attack/attack_engine.h"
#include "../logging/logger.h"
#include "../gpu/gpu_manager.h"
#include "../ml/ml_model_trainer.h"
#include "../ml/ml_predictor.h"
#include "../integration/external_service.h"
#include "../security/access_control.h"
#include "../system/system_manager.h"
#include "../update/update_manager.h"
#include "../data/data_analyzer.h"
#include "../database/db_manager.h"

CLI::CLI(int argc, char* argv[]) : argc(argc), argv(argv), command(""), arguments() {}

bool CLI::parseArguments() {
    if (argc < 2) {
        displayHelp();
        return false;
    }

    command = argv[1];
    for (int i = 2; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    parseArgumentMap();
    return true;
}

std::string CLI::getCommand() const {
    return command;
}

std::vector<std::string> CLI::getArguments() const {
    return arguments;
}

std::string CLI::getArgumentValue(const std::string &key) const {
    auto it = argumentMap.find(key);
    if (it != argumentMap.end()) {
        return it->second;
    }
    return "";
}

bool CLI::hasArgument(const std::string &key) const {
    return argumentMap.find(key) != argumentMap.end();
}

void CLI::displayHelp() const {
    std::cout << "Использование: program <команда> [аргументы]\n";
    std::cout << "Доступные команды:\n";
    std::cout << "  start       - Запуск атаки\n";
    std::cout << "  stop        - Остановка атаки\n";
    std::cout << "  pause       - Пауза атаки\n";
    std::cout << "  resume      - Возобновление атаки\n";
    std::cout << "  status      - Показать статус текущей атаки\n";
    std::cout << "  export_logs - Экспорт логов\n";
    std::cout << "  filter_logs - Фильтрация логов\n";
    std::cout << "  help        - Показать эту справку\n";
    std::cout << "  dictionary  - Управление словарями\n";
    std::cout << "    load=<path>        - Загрузить словарь из файла\n";
    std::cout << "    save=<path>        - Сохранить текущий словарь в файл\n";
    std::cout << "    add=<words>        - Добавить слова в текущий словарь\n";
    std::cout << "    stats             - Показать статистику словаря\n";
    std::cout << "  rule       - Управление правилами\n";
    std::cout << "    load=<path>        - Загрузить правила из файла\n";
    std::cout << "    save=<path>        - Сохранить текущие правила в файл\n";
    std::cout << "    add=<rule>         - Добавить правило\n";
    std::cout << "    remove=<rule>      - Удалить правило\n";
    std::cout << "    stats              - Показать статистику правил\n";
    std::cout << "  gpu        - Управление GPU\n";
    std::cout << "    monitor            - Мониторинг состояния GPU\n";
    std::cout << "    optimize_memory    - Оптимизация памяти GPU\n";
    std::cout << "    manage_power       - Управление энергопотреблением GPU\n";
    std::cout << "  mlmodel    - Управление моделями машинного обучения\n";
    std::cout << "    train              - Обучение модели\n";
    std::cout << "    predict            - Выполнение предсказаний\n";
    std::cout << "    evaluate           - Оценка модели\n";
    std::cout << "    cross_validate     - Кросс-валидация модели\n";
    std::cout << "    report             - Генерация отчета по модели\n";
    std::cout << "  integration - Управление внешними интеграциями\n";
    std::cout << "    api_call           - Выполнить API-запрос\n";
    std::cout << "    config             - Настройка интеграции\n";
    std::cout << "  security   - Управление безопасностью\n";
    std::cout << "    set_role           - Установить роль пользователя\n";
    std::cout << "    get_permissions    - Получить права пользователя\n";
    std::cout << "  system     - Управление системными настройками\n";
    std::cout << "    set_param          - Установить параметр системы\n";
    std::cout << "    get_status         - Получить статус системы\n";
    std::cout << "  update     - Управление обновлениями\n";
    std::cout << "    check_for_updates  - Проверить наличие обновлений\n";
    std::cout << "    install_update     - Установить обновление\n";
    std::cout << "  data_analysis - Управление анализом данных\n";
    std::cout << "    query              - Выполнить запрос\n";
    std::cout << "    report             - Сгенерировать отчет\n";
    std::cout << "  script     - Управление сценариями\n";
    std::cout << "    run                - Выполнить сценарий\n";
    std::cout << "    create             - Создать сценарий\n";
    std::cout << "  log        - Управление логами\n";
    std::cout << "    view               - Просмотреть логи\n";
    std::cout << "    filter=<level>     - Фильтрация логов по уровню\n";
    std::cout << "    export=<format>    - Экспорт логов\n";
    std::cout << "    clear              - Очистить логи\n";
    std::cout << "    set_level=<level>  - Изменить уровень логирования\n";
    std::cout << "  custom_notification - Управление пользовательскими уведомлениями\n";
    std::cout << "    create             - Создать уведомление\n";
    std::cout << "    delete             - Удалить уведомление\n";
    std::cout << "    list               - Список уведомлений\n";
    std::cout << "  monitor     - Управление мониторингом\n";
    std::cout << "    start              - Запустить мониторинг\n";
    std::cout << "    stop               - Остановить мониторинг\n";
    std::cout << "    status             - Получить статус мониторинга\n";
    std::cout << "  user       - Управление пользователями\n";
    std::cout << "    add=<name> role=<role>    - Добавить пользователя с ролью\n";
    std::cout << "    remove=<name>             - Удалить пользователя\n";
    std::cout << "    update=<name> role=<role> - Обновить роль пользователя\n";
    std::cout << "    activate=<name>           - Активировать пользователя\n";
    std::cout << "    deactivate=<name>         - Деактивировать пользователя\n";
    std::cout << "    list                      - Список пользователей\n";
    std::cout << "    export=<path>             - Экспортировать пользователей в CSV\n";
    std::cout << "    import=<path>             - Импортировать пользователей из CSV\n";
    std::cout << "  db         - Управление базой данных\n";
    std::cout << "    connect=<dsn>             - Подключиться к базе данных\n";
    std::cout << "    query=<sql>               - Выполнить SQL-запрос\n";
    std::cout << "    cache_status              - Проверить состояние кэша\n";
    std::cout << "    encrypt_data              - Зашифровать данные\n";
    std::cout << "    monitor                   - Мониторинг базы данных\n";
}

void CLI::parseArgumentMap() {
    for (const auto &arg : arguments) {
        auto delimiterPos = arg.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = arg.substr(0, delimiterPos);
            std::string value = arg.substr(delimiterPos + 1);
            argumentMap[key] = value;
        }
    }
}

bool CLI::handleDictionaryCommand(DictionaryLoader& dictLoader) {
    if (command == "dictionary") {
        if (hasArgument("load")) {
            std::string path = getArgumentValue("load");
            if (dictLoader.load(path)) {
                std::cout << "Словарь загружен из " << path << std::endl;
                Logger::info("Словарь загружен из " + path);
            } else {
                std::cerr << "Ошибка загрузки словаря из " << path << std::endl;
                Logger::error("Ошибка загрузки словаря из " + path);
                return false;
            }
        } else if (hasArgument("save")) {
            std::string path = getArgumentValue("save");
            if (dictLoader.save(path)) {
                std::cout << "Словарь сохранен в " << path << std::endl;
                Logger::info("Словарь сохранен в " + path);
            } else {
                std::cerr << "Ошибка сохранения словаря в " << path << std::endl;
                Logger::error("Ошибка сохранения словаря в " + path);
                return false;
            }
        } else if (hasArgument("add")) {
            std::string wordsStr = getArgumentValue("add");
            std::vector<std::string> words;
            size_t pos = 0;
            while ((pos = wordsStr.find(',')) != std::string::npos) {
                words.push_back(wordsStr.substr(0, pos));
                wordsStr.erase(0, pos + 1);
            }
            words.push_back(wordsStr);
            dictLoader.addWords(words);
            std::cout << "Слова добавлены в словарь" << std::endl;
            Logger::info("Слова добавлены в словарь");
        } else if (hasArgument("stats")) {
            auto stats = dictLoader.getStatistics();
            std::cout << "Статистика словаря:" << std::endl;
            Logger::info("Запрос на статистику словаря");
            for (const auto& stat : stats) {
                std::cout << stat.first << ": " << stat.second << std::endl;
                Logger::info(stat.first + ": " + std::to_string(stat.second));
            }
        } else {
            std::cerr << "Неизвестная команда для словаря" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleRuleCommand(RuleEngine& ruleEngine) {
    if (command == "rule") {
        if (hasArgument("load")) {
            std::string path = getArgumentValue("load");
            if (ruleEngine.loadRules(path)) {
                std::cout << "Правила загружены из " << path << std::endl;
                Logger::info("Правила загружены из " + path);
            } else {
                std::cerr << "Ошибка загрузки правил из " << path << std::endl;
                Logger::error("Ошибка загрузки правил из " + path);
                return false;
            }
        } else if (hasArgument("save")) {
            std::string path = getArgumentValue("save");
            if (ruleEngine.saveRules(path)) {
                std::cout << "Правила сохранены в " << path << std::endl;
                Logger::info("Правила сохранены в " + path);
            } else {
                std::cerr << "Ошибка сохранения правил в " << path << std::endl;
                Logger::error("Ошибка сохранения правил в " + path);
                return false;
            }
        } else if (hasArgument("add")) {
            std::string rule = getArgumentValue("add");
            if (ruleEngine.addRule(rule)) {
                std::cout << "Правило добавлено: " << rule << std::endl;
                Logger::info("Правило добавлено: " + rule);
            } else {
                std::cerr << "Ошибка добавления правила: " << rule << std::endl;
                Logger::error("Ошибка добавления правила: " + rule);
                return false;
            }
        } else if (hasArgument("remove")) {
            std::string rule = getArgumentValue("remove");
            if (ruleEngine.removeRule(rule)) {
                std::cout << "Правило удалено: " << rule << std::endl;
                Logger::info("Правило удалено: " + rule);
            } else {
                std::cerr << "Ошибка удаления правила: " << rule << std::endl;
                Logger::error("Ошибка удаления правила: " + rule);
                return false;
            }
        } else if (hasArgument("stats")) {
            auto stats = ruleEngine.getStatistics();
            std::cout << "Статистика правил:" << std::endl;
            Logger::info("Запрос на статистику правил");
            for (const auto& stat : stats) {
                std::cout << stat.first << ": " << stat.second << std::endl;
                Logger::info(stat.first + ": " + std::to_string(stat.second));
            }
        } else {
            std::cerr << "Неизвестная команда для правил" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleConfigCommand(Config& config) {
    if (command == "config") {
        if (hasArgument("set")) {
            std::string key = getArgumentValue("set");
            std::string value = getArgumentValue("value");
            config.set(key, value);
            config.save("config.json");
            std::cout << "Конфигурация обновлена: " + key + " = " + value << std::endl;
            Logger::info("Конфигурация обновлена: " + key + " = " + value);
        } else {
            std::cout << config.toString() << std::endl;
            Logger::info("Запрос на отображение конфигурации");
        }
    }
    return true;
}

bool CLI::handleAttackCommand(AttackEngine& attackEngine) {
    if (command == "start") {
        std::string attackType = getArgumentValue("type");
        std::string parameter = getArgumentValue("param");
        attackEngine.startAttackCLI(attackType, parameter);
        Logger::info("Запуск атаки: " + attackType);
    } else if (command == "stop") {
        attackEngine.stopAttackCLI();
        Logger::info("Остановка атаки");
    } else if (command == "pause") {
        attackEngine.pauseAttackCLI();
        Logger::info("Пауза атаки");
    } else if (command == "resume") {
        attackEngine.resumeAttackCLI();
        Logger::info("Возобновление атаки");
    } else if (command == "status") {
        std::cout << "Статус атаки: " << attackEngine.getStatusCLI() << std::endl;
        Logger::info("Запрос статуса атаки: " + attackEngine.getStatusCLI());
    } else {
        std::cerr << "Неизвестная команда для управления атаками" << std::endl;
        displayHelp();
        return false;
    }
    return true;
}

bool CLI::handleUserCommand(UserManagement& userManagement) {
    if (command == "user") {
        if (hasArgument("add")) {
            std::string userName = getArgumentValue("add");
            std::string userRole = getArgumentValue("role");
            if (userManagement.addUser(userName, userRole)) {
                std::cout << "Пользователь " << userName << " добавлен с ролью " << userRole << std::endl;
                Logger::info("Пользователь " + userName + " добавлен с ролью " + userRole);
            } else {
                std::cerr << "Ошибка добавления пользователя " << userName << std::endl;
                Logger::error("Ошибка добавления пользователя " + userName);
                return false;
            }
        } else if (hasArgument("remove")) {
            std::string userName = getArgumentValue("remove");
            if (userManagement.removeUser(userName)) {
                std::cout << "Пользователь " << userName << " удален" << std::endl;
                Logger::info("Пользователь " + userName + " удален");
            } else {
                std::cerr << "Ошибка удаления пользователя " << userName << std::endl;
                Logger::error("Ошибка удаления пользователя " + userName);
                return false;
            }
        } else if (hasArgument("update")) {
            std::string userName = getArgumentValue("update");
            std::string userRole = getArgumentValue("role");
            if (userManagement.updateUser(userName, userRole)) {
                std::cout << "Роль пользователя " << userName << " обновлена на " << userRole << std::endl;
                Logger::info("Роль пользователя " + userName + " обновлена на " + userRole);
            } else {
                std::cerr << "Ошибка обновления роли пользователя " << userName << std::endl;
                Logger::error("Ошибка обновления роли пользователя " + userName);
                return false;
            }
        } else if (hasArgument("activate")) {
            std::string userName = getArgumentValue("activate");
            if (userManagement.activateUser(userName)) {
                std::cout << "Пользователь " << userName << " активирован" << std::endl;
                Logger::info("Пользователь " + userName + " активирован");
            } else {
                std::cerr << "Ошибка активации пользователя " << userName << std::endl;
                Logger::error("Ошибка активации пользователя " + userName);
                return false;
            }
        } else if (hasArgument("deactivate")) {
            std::string userName = getArgumentValue("deactivate");
            if (userManagement.deactivateUser(userName)) {
                std::cout << "Пользователь " << userName << " деактивирован" << std::endl;
                Logger::info("Пользователь " + userName + " деактивирован");
            } else {
                std::cerr << "Ошибка деактивации пользователя " << userName << std::endl;
                Logger::error("Ошибка деактивации пользователя " + userName);
                return false;
            }
        } else if (hasArgument("list")) {
            auto userList = userManagement.listUsers();
            std::cout << "Список пользователей:" << std::endl;
            for (const auto& user : userList) {
                std::cout << "Имя: " << user.name << ", Роль: " << user.role << std::endl;
            }
        } else if (hasArgument("export")) {
            std::string path = getArgumentValue("export");
            if (userManagement.exportUsers(path)) {
                std::cout << "Пользователи экспортированы в " << path << std::endl;
                Logger::info("Пользователи экспортированы в " + path);
            } else {
                std::cerr << "Ошибка экспорта пользователей в " << path << std::endl;
                Logger::error("Ошибка экспорта пользователей в " + path);
                return false;
            }
        } else if (hasArgument("import")) {
            std::string path = getArgumentValue("import");
            if (userManagement.importUsers(path)) {
                std::cout << "Пользователи импортированы из " << path << std::endl;
                Logger::info("Пользователи импортированы из " + path);
            } else {
                std::cerr << "Ошибка импорта пользователей из " << path << std::endl;
                Logger::error("Ошибка импорта пользователей из " + path);
                return false;
            }
        } else {
            std::cerr << "Неизвестная команда для управления пользователями" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleScheduleCommand(Scheduler& scheduler) {
    if (command == "schedule") {
        if (hasArgument("add")) {
            std::string taskName = getArgumentValue("name");
            std::string taskTime = getArgumentValue("time");
            if (scheduler.addTask(taskName, taskTime)) {
                std::cout << "Задача " << taskName << " добавлена на " << taskTime << std::endl;
                Logger::info("Задача " + taskName + " добавлена на " + taskTime);
            } else {
                std::cerr << "Ошибка добавления задачи " << taskName << std::endl;
                Logger::error("Ошибка добавления задачи " + taskName);
                return false;
            }
        } else if (hasArgument("remove")) {
            std::string taskName = getArgumentValue("name");
            if (scheduler.removeTask(taskName)) {
                std::cout << "Задача " << taskName << " удалена" << std::endl;
                Logger::info("Задача " + taskName + " удалена");
            } else {
                std::cerr << "Ошибка удаления задачи " << taskName << std::endl;
                Logger::error("Ошибка удаления задачи " + taskName);
                return false;
            }
        } else if (hasArgument("list")) {
            auto taskList = scheduler.listTasks();
            std::cout << "Список задач:" << std::endl;
            for (const auto& task : taskList) {
                std::cout << "Имя: " << task.name << ", Время: " << task.time << std::endl;
            }
        } else {
            std::cerr << "Неизвестная команда для управления расписанием задач" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleNotificationCommand(NotificationManager& notificationManager) {
    if (command == "notification") {
        if (hasArgument("send")) {
            std::string message = getArgumentValue("message");
            if (notificationManager.sendNotification(message)) {
                std::cout << "Уведомление отправлено: " << message << std::endl;
                Logger::info("Уведомление отправлено: " + message);
            } else {
                std::cerr << "Ошибка отправки уведомления: " << message << std::endl;
                Logger::error("Ошибка отправки уведомления: " + message);
                return false;
            }
        } else if (hasArgument("list")) {
            auto notifications = notificationManager.listNotifications();
            std::cout << "Список уведомлений:" << std::endl;
            for (const auto& notification : notifications) {
                std::cout << "Уведомление: " << notification << std::endl;
            }
        } else {
            std::cerr << "Неизвестная команда для управления уведомлениями" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleAnalyticsCommand(AnalyticsManager& analyticsManager) {
    if (command == "analytics") {
        if (hasArgument("generate_report")) {
            std::string reportType = getArgumentValue("type");
            if (analyticsManager.generateReport(reportType)) {
                std::cout << "Отчет " << reportType << " сгенерирован" << std::endl;
                Logger::info("Отчет " + reportType + " сгенерирован");
            } else {
                std::cerr << "Ошибка генерации отчета " << reportType << std::endl;
                Logger::error("Ошибка генерации отчета " + reportType);
                return false;
            }
        } else if (hasArgument("list_reports")) {
            auto reportList = analyticsManager.listReports();
            std::cout << "Список отчетов:" << std::endl;
            for (const auto& report : reportList) {
                std::cout << "Отчет: " << report << std::endl;
            }
        } else {
            std::cerr << "Неизвестная команда для управления аналитикой" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleGPUCommand(GPUManager& gpuManager) {
    if (command == "gpu") {
        if (hasArgument("monitor")) {
            Logger::info("Запрос на мониторинг состояния GPU");
            gpuManager.monitor();
            std::cout << "Мониторинг состояния GPU выполнен" << std::endl;
            Logger::info("Мониторинг состояния GPU выполнен");
        } else if (hasArgument("optimize_memory")) {
            Logger::info("Запрос на оптимизацию памяти GPU");
            gpuManager.optimizeMemory();
            std::cout << "Оптимизация памяти GPU выполнена" << std::endl;
            Logger::info("Оптимизация памяти GPU выполнена");
        } else if (hasArgument("manage_power")) {
            Logger::info("Запрос на управление энергопотреблением GPU");
            gpuManager.managePower();
            std::cout << "Управление энергопотреблением GPU выполнено" << std::endl;
            Logger::info("Управление энергопотреблением GPU выполнено");
        } else {
            std::cerr << "Неизвестная команда для GPU" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleMLModelTrainerCommand(MLModelTrainer& mlModelTrainer) {
    if (command == "mlmodel") {
        if (hasArgument("train")) {
            Logger::info("Запрос на обучение модели");
            std::string dataPath = getArgumentValue("data");
            mlModelTrainer.loadTrainingData(dataPath);
            mlModelTrainer.trainModel();
            mlModelTrainer.saveModel(getArgumentValue("save"));
            std::cout << "Модель обучена и сохранена" << std::endl;
            Logger::info("Модель обучена и сохранена");
        } else if (hasArgument("evaluate")) {
            Logger::info("Запрос на оценку модели");
            std::string dataPath = getArgumentValue("data");
            mlModelTrainer.loadTrainingData(dataPath);
            mlModelTrainer.evaluateModel();
            std::cout << "Модель оценена" << std::endl;
            Logger::info("Модель оценена");
        } else if (hasArgument("cross_validate")) {
            Logger::info("Запрос на кросс-валидацию модели");
            mlModelTrainer.crossValidateModel();
            std::cout << "Кросс-валидация модели выполнена" << std::endl;
            Logger::info("Кросс-валидация модели выполнена");
        } else if (hasArgument("report")) {
            Logger::info("Запрос на генерацию отчета по модели");
            mlModelTrainer.generateReport();
            std::cout << "Отчет по модели сгенерирован" << std::endl;
            Logger::info("Отчет по модели сгенерирован");
        } else {
            std::cerr << "Неизвестная команда для моделей машинного обучения" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleMLPredictorCommand(MLPredictor& mlPredictor) {
    if (command == "mlmodel" && hasArgument("predict")) {
        Logger::info("Запрос на предсказание модели");
        std::string dataPath = getArgumentValue("data");
        if (mlPredictor.loadModel(getArgumentValue("model")) && mlPredictor.loadTestData(dataPath)) {
            auto predictions = mlPredictor.predict();
            std::cout << "Предсказания:" << std::endl;
            for (const auto& prediction : predictions) {
                std::cout << prediction << std::endl;
            }
            Logger::info("Предсказания выполнены");
        } else {
            std::cerr << "Ошибка выполнения предсказаний" << std::endl;
            Logger::error("Ошибка выполнения предсказаний");
            return false;
        }
    }
    return true;
}

bool CLI::handleIntegrationCommand(ExternalService& externalService) {
    if (command == "integration") {
        if (hasArgument("api_call")) {
            std::string apiEndpoint = getArgumentValue("endpoint");
            std::string apiParams = getArgumentValue("params");
            if (externalService.apiCall(apiEndpoint, apiParams)) {
                std::cout << "API-запрос выполнен успешно" << std::endl;
                Logger::info("API-запрос выполнен успешно");
            } else {
                std::cerr << "Ошибка выполнения API-запроса" << std::endl;
                Logger::error("Ошибка выполнения API-запроса");
                return false;
            }
        } else if (hasArgument("config")) {
            std::string configKey = getArgumentValue("key");
            std::string configValue = getArgumentValue("value");
            if (externalService.configure(configKey, configValue)) {
                std::cout << "Интеграция настроена: " << configKey << " = " << configValue << std::endl;
                Logger::info("Интеграция настроена: " + configKey + " = " + configValue);
            } else {
                std::cerr << "Ошибка настройки интеграции" << std::endl;
                Logger::error("Ошибка настройки интеграции");
                return false;
            }
        } else {
            std::cerr << "Неизвестная команда для интеграции" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleSecurityCommand(AccessControl& accessControl) {
    if (command == "security") {
        if (hasArgument("set_role")) {
            std::string userName = getArgumentValue("name");
            std::string role = getArgumentValue("role");
            if (accessControl.setUserRole(userName, role)) {
                std::cout << "Роль пользователя " << userName << " установлена на " << role << std::endl;
                Logger::info("Роль пользователя " + userName + " установлена на " + role);
            } else {
                std::cerr << "Ошибка установки роли пользователя " << userName << std::endl;
                Logger::error("Ошибка установки роли пользователя " + userName);
                return false;
            }
        } else if (hasArgument("get_permissions")) {
            std::string userName = getArgumentValue("name");
            auto permissions = accessControl.getUserPermissions(userName);
            std::cout << "Права пользователя " << userName << ":" << std::endl;
            for (const auto& perm : permissions) {
                std::cout << perm << std::endl;
            }
        } else {
            std::cerr << "Неизвестная команда для управления безопасностью" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleSystemCommand(SystemManager& systemManager) {
    if (command == "system") {
        if (hasArgument("set_param")) {
            std::string paramName = getArgumentValue("name");
            std::string paramValue = getArgumentValue("value");
            if (systemManager.setSystemParameter(paramName, paramValue)) {
                std::cout << "Параметр системы " << paramName << " установлен на " << paramValue << std::endl;
                Logger::info("Параметр системы " + paramName + " установлен на " + paramValue);
            } else {
                std::cerr << "Ошибка установки параметра системы " << paramName << std::endl;
                Logger::error("Ошибка установки параметра системы " + paramName);
                return false;
            }
        } else if (hasArgument("get_status")) {
            auto status = systemManager.getSystemStatus();
            std::cout << "Статус системы:" << std::endl;
            for (const auto& [key, value] : status) {
                std::cout << key << ": " << value << std::endl;
            }
        } else {
            std::cerr << "Неизвестная команда для управления системой" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleUpdateCommand(UpdateManager& updateManager) {
    if (command == "update") {
        if (hasArgument("check_for_updates")) {
            if (updateManager.checkForUpdates()) {
                std::cout << "Обновления доступны" << std::endl;
                Logger::info("Обновления доступны");
            } else {
                std::cout << "Нет доступных обновлений" << std::endl;
                Logger::info("Нет доступных обновлений");
            }
        } else if (hasArgument("install_update")) {
            if (updateManager.installUpdate()) {
                std::cout << "Обновление установлено" << std::endl;
                Logger::info("Обновление установлено");
            } else {
                std::cerr << "Ошибка установки обновления" << std::endl;
                Logger::error("Ошибка установки обновления");
                return false;
            }
        } else {
            std::cerr << "Неизвестная команда для управления обновлениями" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleDataAnalysisCommand(DataAnalyzer& dataAnalyzer) {
    if (command == "data_analysis") {
        if (hasArgument("query")) {
            std::string query = getArgumentValue("query");
            auto result = dataAnalyzer.executeQuery(query);
            std::cout << "Результат запроса:" << std::endl;
            for (const auto& row : result) {
                std::cout << row << std::endl;
            }
        } else if (hasArgument("report")) {
            std::string reportType = getArgumentValue("type");
            if (dataAnalyzer.generateReport(reportType)) {
                std::cout << "Отчет " << reportType << " сгенерирован" << std::endl;
                Logger::info("Отчет " + reportType + " сгенерирован");
            } else {
                std::cerr << "Ошибка генерации отчета " << reportType << std::endl;
                Logger::error("Ошибка генерации отчета " + reportType);
                return false;
            }
        } else {
            std::cerr << "Неизвестная команда для анализа данных" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleScriptCommand() {
    if (command == "script") {
        if (hasArgument("run")) {
            std::string scriptPath = getArgumentValue("run");
            std::ifstream scriptFile(scriptPath);
            if (scriptFile.is_open()) {
                std::string line;
                while (std::getline(scriptFile, line)) {
                    system(line.c_str());
                }
                scriptFile.close();
                std::cout << "Сценарий выполнен" << std::endl;
                Logger::info("Сценарий выполнен: " + scriptPath);
            } else {
                std::cerr << "Ошибка открытия сценария " << scriptPath << std::endl;
                Logger::error("Ошибка открытия сценария " + scriptPath);
                return false;
            }
        } else if (hasArgument("create")) {
            std::string scriptPath = getArgumentValue("create");
            std::ofstream scriptFile(scriptPath);
            if (scriptFile.is_open()) {
                scriptFile << "#!/bin/bash\n";
                scriptFile.close();
                system(("chmod +x " + scriptPath).c_str());
                std::cout << "Сценарий создан" << std::endl;
                Logger::info("Сценарий создан: " + scriptPath);
            } else {
                std::cerr << "Ошибка создания сценария " << scriptPath << std::endl;
                Logger::error("Ошибка создания сценария " + scriptPath);
                return false;
            }
        } else {
            std::cerr << "Неизвестная команда для сценариев" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleLogCommand(Logger& logger) {
    if (command == "log") {
        if (hasArgument("view")) {
            std::cout << logger.viewLogs(100) << std::endl;
        } else if (hasArgument("filter")) {
            std::string levelStr = getArgumentValue("filter");
            LogLevel level = LogLevel::INFO;
            if (levelStr == "TRACE") level = LogLevel::TRACE;
            else if (levelStr == "DEBUG") level = LogLevel::DEBUG;
            else if (levelStr == "WARNING") level = LogLevel::WARNING;
            else if (levelStr == "ERROR") level = LogLevel::ERROR;
            std::cout << logger.filterLogs(level, 100) << std::endl;
        } else if (hasArgument("export")) {
            std::string format = getArgumentValue("export");
            std::cout << logger.exportLogs(format) << std::endl;
        } else if (hasArgument("clear")) {
            logger.clearLogs();
            std::cout << "Логи очищены" << std::endl;
        } else if (hasArgument("set_level")) {
            std::string levelStr = getArgumentValue("set_level");
            LogLevel level = LogLevel::INFO;
            if (levelStr == "TRACE") level = LogLevel::TRACE;
            else if (levelStr == "DEBUG") level = LogLevel::DEBUG;
            else if (levelStr == "WARNING") level = LogLevel::WARNING;
            else if (levelStr == "ERROR") level = LogLevel::ERROR;
            logger.setLogLevel(level);
            std::cout << "Уровень логирования изменен на " << levelStr << std::endl;
        } else {
            std::cerr << "Неизвестная команда для управления логами" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleCustomNotificationCommand(NotificationManager& notificationManager) {
    if (command == "custom_notification") {
        if (hasArgument("create")) {
            std::string message = getArgumentValue("message");
            std::string recipient = getArgumentValue("recipient");
            if (notificationManager.createCustomNotification(message, recipient)) {
                std::cout << "Уведомление создано" << std::endl;
                Logger::info("Уведомление создано: " + message + " для " + recipient);
            } else {
                std::cerr << "Ошибка создания уведомления" << std::endl;
                Logger::error("Ошибка создания уведомления: " + message + " для " + recipient);
                return false;
            }
        } else if (hasArgument("delete")) {
            std::string notificationId = getArgumentValue("id");
            if (notificationManager.deleteCustomNotification(notificationId)) {
                std::cout << "Уведомление удалено" << std::endl;
                Logger::info("Уведомление удалено: " + notificationId);
            } else {
                std::cerr << "Ошибка удаления уведомления" << std::endl;
                Logger::error("Ошибка удаления уведомления: " + notificationId);
                return false;
            }
        } else if (hasArgument("list")) {
            auto notifications = notificationManager.listCustomNotifications();
            std::cout << "Список уведомлений:" << std::endl;
            for (const auto& notification : notifications) {
                std::cout << "ID: " << notification.id << ", Сообщение: " << notification.message << ", Получатель: " << notification.recipient << std::endl;
            }
        } else {
            std::cerr << "Неизвестная команда для пользовательских уведомлений" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleMonitorCommand(Monitor& monitor) {
    if (command == "monitor") {
        if (hasArgument("start")) {
            monitor.startMonitoring();
            std::cout << "Мониторинг запущен" << std::endl;
            Logger::info("Мониторинг запущен");
        } else if (hasArgument("stop")) {
            monitor.stopMonitoring();
            std::cout << "Мониторинг остановлен" << std::endl;
            Logger::info("Мониторинг остановлен");
        } else if (hasArgument("status")) {
            std::cout << "Статус мониторинга: " << (monitor.isMonitoring() ? "Запущен" : "Остановлен") << std::endl;
            Logger::info("Запрос статуса мониторинга: " + std::string(monitor.isMonitoring() ? "Запущен" : "Остановлен"));
        } else {
            std::cerr << "Неизвестная команда для управления мониторингом" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

bool CLI::handleDBCommand(DBManager& dbManager) {
    if (command == "db") {
        if (hasArgument("connect")) {
            std::string dsn = getArgumentValue("connect");
            if (dbManager.connect(dsn)) {
                std::cout << "Подключено к базе данных " << dsn << std::endl;
                Logger::info("Подключено к базе данных " + dsn);
            } else {
                std::cerr << "Ошибка подключения к базе данных " << dsn << std::endl;
                Logger::error("Ошибка подключения к базе данных " + dsn);
                return false;
            }
        } else if (hasArgument("query")) {
            std::string sql = getArgumentValue("query");
            auto result = dbManager.executeQuery(sql);
            std::cout << "Результат запроса:" << std::endl;
            for (const auto& row : result) {
                std::cout << row << std::endl;
            }
        } else if (hasArgument("cache_status")) {
            auto status = dbManager.getCacheStatus();
            std::cout << "Состояние кэша базы данных:" << std::endl;
            for (const auto& [key, value] : status) {
                std::cout << key << ": " << value << std::endl;
            }
        } else if (hasArgument("encrypt_data")) {
            if (dbManager.encryptData()) {
                std::cout << "Данные зашифрованы" << std::endl;
                Logger::info("Данные зашифрованы");
            } else {
                std::cerr << "Ошибка шифрования данных" << std::endl;
                Logger::error("Ошибка шифрования данных");
                return false;
            }
        } else if (hasArgument("monitor")) {
            if (dbManager.monitor()) {
                std::cout << "Мониторинг базы данных выполнен" << std::endl;
                Logger::info("Мониторинг базы данных выполнен");
            } else {
                std::cerr << "Ошибка мониторинга базы данных" << std::endl;
                Logger::error("Ошибка мониторинга базы данных");
                return false;
            }
        } else {
            std::cerr << "Неизвестная команда для управления базой данных" << std::endl;
            displayHelp();
            return false;
        }
    }
    return true;
}

std::string CLI::getMaskFromUser() {
    std::string mask;
    std::cout << "Введите маску для атаки: ";
    std::cin >> mask;
    return mask;
}

std::string CLI::getFilenameFromUser() {
    std::string filename;
    std::cout << "Введите имя файла для экспорта логов: ";
    std::cin >> filename;
    return filename;
}

std::string CLI::getStatusFromUser() {
    std::string status;
    std::cout << "Введите статус для фильтрации логов: ";
    std::cin >> status;
    return status;
}

void CLI::interactiveMode() {
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "exit" || input == "quit") {
            break;
        }

        std::istringstream iss(input);
        std::vector<std::string> args{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
        if (!args.empty()) {
            argc = args.size() + 1;
            argv = new char*[argc];
            argv[0] = new char[8];
            strcpy(argv[0], "program");
            for (size_t i = 0; i < args.size(); ++i) {
                argv[i + 1] = new char[args[i].size() + 1];
                strcpy(argv[i + 1], args[i].c_str());
            }

            parseArguments();

            if (command == "dictionary") handleDictionaryCommand(DictionaryLoader());
            else if (command == "config") handleConfigCommand(Config());
            else if (command == "start" || command == "stop" || command == "pause" || command == "resume" || command == "status") handleAttackCommand(AttackEngine());
            else if (command == "rule") handleRuleCommand(RuleEngine());
            else if (command == "user") handleUserCommand(UserManagement());
            else if (command == "schedule") handleScheduleCommand(Scheduler());
            else if (command == "notification") handleNotificationCommand(NotificationManager());
            else if (command == "analytics") handleAnalyticsCommand(AnalyticsManager());
            else if (command == "gpu") handleGPUCommand(GPUManager());
            else if (command == "mlmodel") {
                if (hasArgument("train") || hasArgument("evaluate") || hasArgument("cross_validate") || hasArgument("report")) {
                    handleMLModelTrainerCommand(MLModelTrainer());
                } else if (hasArgument("predict")) {
                    handleMLPredictorCommand(MLPredictor());
                }
            } else if (command == "integration") handleIntegrationCommand(ExternalService());
            else if (command == "security") handleSecurityCommand(AccessControl());
            else if (command == "system") handleSystemCommand(SystemManager());
            else if (command == "update") handleUpdateCommand(UpdateManager());
            else if (command == "data_analysis") handleDataAnalysisCommand(DataAnalyzer());
            else if (command == "script") handleScriptCommand();
            else if (command == "log") handleLogCommand(Logger());
            else if (command == "custom_notification") handleCustomNotificationCommand(NotificationManager());
            else if (command == "monitor") handleMonitorCommand(Monitor());
            else if (command == "db") handleDBCommand(DBManager());
            else displayHelp();

            for (size_t i = 0; i < args.size() + 1; ++i) {
                delete[] argv[i];
            }
            delete[] argv;
        }
    }
}

















