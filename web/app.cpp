#include "app.h"
#include "logging/logger.h"
#include "config/config.h"
#include <crow.h>

bool WebApp::initialize(const std::string &configPath) {
    this->configPath = configPath;
    try {
        Config::load(configPath);
        setupRoutes();
        Logger::info("Маршруты веб-приложения настроены");
        return true;
    } catch (const std::exception &e) {
        Logger::error("Ошибка при инициализации веб-приложения: " + std::string(e.what()));
        return false;
    }
}
void WebApp::run() {
    Logger::info("Запуск веб-приложения");
    app.port(18080).multithreaded().run();
}
void WebApp::update() {
}
void WebApp::setupRoutes() {
    CROW_ROUTE(app, "/")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRoot(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/start")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleStartAttack(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/stop")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleStopAttack(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/status")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleStatus(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/config")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleConfig(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/logs")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLogs(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/add_user")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleAddUser(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/remove_user")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRemoveUser(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/user_management")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleUserManagement(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/schedule")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleSchedule(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/analytics")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleAnalytics(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/notifications")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleNotifications(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/dictionary/load")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDictionaryLoad(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/dictionary/save")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDictionarySave(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/dictionary/add")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDictionaryAdd(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/dictionary/stats")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDictionaryStats(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/rules/load")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRulesLoad(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/rules/add")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRulesAdd(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/rules/save")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRulesSave(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/rules/stats")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRulesStats(req, res);
        logResponse(res);
    });
    CROW_ROUTE(app, "/attack/start")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleAttackStart(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/attack/stop")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleAttackStop(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/attack/status")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleAttackStatus(req, res);
        logResponse(res);
    });
    CROW_ROUTE(app, "/gpu/monitor")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleGPUMonitoring(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/gpu/optimize_memory")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleGPUMemoryOptimization(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/gpu/manage_power")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleGPUPowerManagement(req, res);
        logResponse(res);
    });
    CROW_ROUTE(app, "/mlmodel/train")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleMLModelTrain(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/mlmodel/evaluate")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleMLModelEvaluate(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/mlmodel/cross_validate")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleMLModelCrossValidate(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/mlmodel/report")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleMLModelReport(req, res);
        logResponse(res);
    });
    CROW_ROUTE(app, "/role/add")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRoleAdd(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/role/remove")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRoleRemove(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/role/assign")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRoleAssign(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/role/revoke")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRoleRevoke(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/cloud/backup")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleCloudBackup(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/cloud/restore")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleCloudRestore(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/cloud/config")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleCloudConfig(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/log/download")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLogDownload(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/log/archive")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLogArchive(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/notification/create")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleCustomNotificationCreate(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/monitoring/settings")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleMonitoringSettings(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/login")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLogin(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/logout")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLogout(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/user_roles")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleUserRoles(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/db/query")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDBQuery(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/db/monitor")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDBMonitor(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/db/backup")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDBBackup(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/db/restore")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDBRestore(req, res);
        logResponse(res);
    });
    CROW_ROUTE(app, "/logs.html")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLogsPage(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/schedule.html")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleSchedulePage(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/user_management.html")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleUserManagementPage(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/reports.html")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleReportsPage(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/login.html")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLoginPage(req, res);
        logResponse(res);
    });
    CROW_ROUTE(app, "/logs/export/json")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleExportLogsJSON(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/logs/export/xml")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleExportLogsXML(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/logs/export/csv")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleExportLogsCSV(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/tasks/create")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleCreateTask(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/tasks/delete")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleDeleteTask(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/tasks/update")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleUpdateTask(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/reports/generate")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleGenerateReport(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/reports/export")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleExportReport(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/auth/2fa")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleTwoFactorAuth(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/auth/social")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleSocialAuth(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/notifications/real_time")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleRealTimeNotifications(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/settings/language")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleLanguageChange(req, res);
        logResponse(res);
    });

    CROW_ROUTE(app, "/settings/theme")([this](const crow::request &req, crow::response &res) {
        logRequest(req);
        handleThemeChange(req, res);
        logResponse(res);
    });
}

void WebApp::handleRoot(const crow::request &req, crow::response &res) {
    res.write("Добро пожаловать в веб-приложение!");
    res.end();
}

void WebApp::handleStartAttack(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на запуск атаки через веб-интерфейс");
    std::string target = req.url_params.get("target");
    if (attackEngine.startAttack(target)) {
        res.write("Атака на " + target + " запущена.");
        Logger::info("Атака на " + target + " успешно запущена.");
    } else {
        res.write("Ошибка при запуске атаки на " + target);
        Logger::error("Ошибка при запуске атаки на " + target);
    }
    res.end();
}

void WebApp::handleStopAttack(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на остановку атаки через веб-интерфейс");
    if (attackEngine.stopAttack()) {
        res.write("Атака остановлена.");
        Logger::info("Атака успешно остановлена.");
    } else {
        res.write("Ошибка при остановке атаки.");
        Logger::error("Ошибка при остановке атаки.");
    }
    res.end();
}

void WebApp::handleStatus(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на получение статуса атаки через веб-интерфейс");
    auto status = attackEngine.getAttackStatus();
    std::string statusStr = "Статус атаки: " + status;
    res.write(statusStr);
    Logger::info(statusStr);
    res.end();
}

void WebApp::handleConfig(const crow::request &req, crow::response &res) {
    res.write("Конфигурация: ...");
    res.end();
}

void WebApp::handleLogs(const crow::request &req, crow::response &res) {
    res.write("Логи: ...");
    res.end();
}

void WebApp::handleAddUser(const crow::request &req, crow::response &res) {
    res.write("Пользователь добавлен");
    res.end();
}

void WebApp::handleRemoveUser(const crow::request &req, crow::response &res) {
    res.write("Пользователь удален");
    res.end();
}

void WebApp::handleUserManagement(const crow::request &req, crow::response &res) {
    res.write("Управление пользователями: ...");
    res.end();
}

void WebApp::handleSchedule(const crow::request &req, crow::response &res) {
    res.write("Расписание атак: ...");
    res.end();
}

void WebApp::handleAnalytics(const crow::request &req, crow::response &res) {
    res.write("Аналитика: ...");
    res.end();
}

void WebApp::handleNotifications(const crow::request &req, crow::response &res) {
    res.write("Уведомления: ...");
    res.end();
}

void WebApp::handleDictionaryLoad(const crow::request &req, crow::response &res) {
    std::string path = req.url_params.get("path");
    if (dictLoader.load(path)) {
        res.write("Словарь загружен из " + path);
    } else {
        res.write("Ошибка загрузки словаря из " + path);
    }
    res.end();
}

void WebApp::handleDictionarySave(const crow::request &req, crow::response &res) {
    std::string path = req.url_params.get("path");
    if (dictLoader.save(path)) {
        res.write("Словарь сохранен в " + path);
    } else {
        res.write("Ошибка сохранения словаря в " + path);
    }
    res.end();
}

void WebApp::handleDictionaryAdd(const crow::request &req, crow::response &res) {
    std::string wordsStr = req.url_params.get("words");
    std::vector<std::string> words;
    size_t pos = 0;
    while ((pos = wordsStr.find(',')) != std::string::npos) {
        words.push_back(wordsStr.substr(0, pos));
        wordsStr.erase(0, pos + 1);
    }
    words.push_back(wordsStr);
    dictLoader.addWords(words);
    res.write("Слова добавлены в словарь");
    res.end();
}

void WebApp::handleDictionaryStats(const crow::request &req, crow::response &res) {
    auto stats = dictLoader.getStatistics();
    std::string statsStr = "Статистика словаря:\n";
    for (const auto& stat : stats) {
        statsStr += stat.first + ": " + std::to_string(stat.second) + "\n";
    }
    res.write(statsStr);
    res.end();
}

void WebApp::handleRulesLoad(const crow::request &req, crow::response &res) {
    std::string path = req.url_params.get("path");
    if (ruleEngine.loadRules(path)) {
        res.write("Правила загружены из " + path);
    } else {
        res.write("Ошибка загрузки правил из " + path);
    }
    res.end();
}

void WebApp::handleRulesAdd(const crow::request &req, crow::response &res) {
    std::string rulesStr = req.url_params.get("rules");
    std::vector<std::string> rules;
    size_t pos = 0;
    while ((pos = rulesStr.find(',')) != std::string::npos) {
        rules.push_back(rulesStr.substr(0, pos));
        rulesStr.erase(0, pos + 1);
    }
    rules.push_back(rulesStr);
    for (const auto& rule : rules) {
        if (!ruleEngine.addRule(rule)) {
            res.write("Ошибка добавления правила: " + rule);
            res.end();
            return;
        }
    }
    res.write("Правила добавлены");
    res.end();
}

void WebApp::handleRulesSave(const crow::request &req, crow::response &res) {
    std::string path = req.url_params.get("path");
    if (ruleEngine.saveRules(path)) {
        res.write("Правила сохранены в " + path);
    } else {
        res.write("Ошибка сохранения правил в " + path);
    }
    res.end();
}

void WebApp::handleRulesStats(const crow::request &req, crow::response &res) {
    auto stats = ruleEngine.getStatistics();
    std::string statsStr = "Статистика правил:\n";
    for (const auto& stat : stats) {
        statsStr += stat.first + ": " + std::to_string(stat.second) + "\n";
    }
    res.write(statsStr);
    res.end();
}

void WebApp::handleGPUMonitoring(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на мониторинг состояния GPU через веб-интерфейс");
    auto status = gpuManager.monitor();
    res.write("Статус GPU: " + status);
    Logger::info("Статус GPU: " + status);
    res.end();
}

void WebApp::handleGPUMemoryOptimization(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на оптимизацию памяти GPU через веб-интерфейс");
    gpuManager.optimizeMemory();
    res.write("Оптимизация памяти GPU выполнена.");
    Logger::info("Оптимизация памяти GPU выполнена.");
    res.end();
}

void WebApp::handleGPUPowerManagement(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на управление энергопотреблением GPU через веб-интерфейс");
    gpuManager.managePower();
    res.write("Управление энергопотреблением GPU выполнено.");
    Logger::info("Управление энергопотреблением GPU выполнено.");
    res.end();
}

void WebApp::handleMLModelTrain(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на обучение модели через веб-интерфейс");
    auto result = mlModelTrainer.train();
    if (result) {
        res.write("Обучение модели завершено успешно.");
        Logger::info("Обучение модели завершено успешно.");
    } else {
        res.write("Ошибка при обучении модели.");
        Logger::error("Ошибка при обучении модели.");
    }
    res.end();
}

void WebApp::handleMLModelEvaluate(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на оценку модели через веб-интерфейс");
    auto metrics = mlModelTrainer.evaluate();
    res.write("Результаты оценки модели:\n" + metrics);
    Logger::info("Результаты оценки модели:\n" + metrics);
    res.end();
}

void WebApp::handleMLModelCrossValidate(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на кросс-валидацию модели через веб-интерфейс");
    auto result = mlModelTrainer.crossValidate();
    res.write("Результаты кросс-валидации модели:\n" + result);
    Logger::info("Результаты кросс-валидации модели:\n" + result);
    res.end();
}

void WebApp::handleMLModelReport(const crow::request &req, crow::response &res) {
    Logger::info("Запрос на генерацию отчета по модели через веб-интерфейс");
    auto report = mlModelTrainer.generateReport();
    res.write("Отчет по модели:\n" + report);
    Logger::info("Отчет по модели:\n" + report);
    res.end();
}

void WebApp::handleRoleAdd(const crow::request &req, crow::response &res) {
    res.write("Роль добавлена");
    res.end();
}

void WebApp::handleRoleRemove(const crow::request &req, crow::response &res) {
    res.write("Роль удалена");
    res.end();
}

void WebApp::handleRoleAssign(const crow::request &req, crow::response &res) {
    res.write("Роль назначена пользователю");
    res.end();
}

void WebApp::handleRoleRevoke(const crow::request &req, crow::response &res) {
    res.write("Роль отозвана у пользователя");
    res.end();
}

void WebApp::handleCloudBackup(const crow::request &req, crow::response &res) {
    res.write("Резервное копирование в облако выполнено");
    res.end();
}

void WebApp::handleCloudRestore(const crow::request &req, crow::response &res) {
    res.write("Восстановление из облака выполнено");
    res.end();
}

void WebApp::handleCloudConfig(const crow::request &req, crow::response &res) {
    res.write("Настройка облачных сервисов выполнена");
    res.end();
}

void WebApp::handleLogDownload(const crow::request &req, crow::response &res) {
    res.write("Логи скачаны");
    res.end();
}

void WebApp::handleLogArchive(const crow::request &req, crow::response &res) {
    res.write("Логи заархивированы");
    res.end();
}

void WebApp::handleCustomNotificationCreate(const crow::request &req, crow::response &res) {
    res.write("Пользовательское уведомление создано");
    res.end();
}

void WebApp::handleMonitoringSettings(const crow::request &req, crow::response &res) {
    res.write("Настройки мониторинга обновлены");
    res.end();
}

void WebApp::handleLogin(const crow::request &req, crow::response &res) {
    res.write("Вход в систему выполнен");
    res.end();
}

void WebApp::handleLogout(const crow::request &req, crow::response &res) {
    res.write("Выход из системы выполнен");
    res.end();
}

void WebApp::handleUserRoles(const crow::request &req, crow::response &res) {
    res.write("Роли пользователя получены");
    res.end();
}

void WebApp::handleDBQuery(const crow::request &req, crow::response &res) {
    std::string query = req.url_params.get("query");
    auto result = dbManager.executeQuery(query);
    res.write("Результаты запроса:\n" + result);
    res.end();
}

void WebApp::handleDBMonitor(const crow::request &req, crow::response &res) {
    auto status = dbManager.getStatus();
    res.write("Статус базы данных:\n" + status);
    res.end();
}

void WebApp::handleDBBackup(const crow::request &req, crow::response &res) {
    if (dbManager.backup()) {
        res.write("Резервное копирование базы данных выполнено.");
    } else {
        res.write("Ошибка при резервном копировании базы данных.");
    }
    res.end();
}

void WebApp::handleDBRestore(const crow::request &req, crow::response &res) {
    if (dbManager.restore()) {
        res.write("Восстановление базы данных выполнено.");
    } else {
        res.write("Ошибка при восстановлении базы данных.");
    }
    res.end();
}
void WebApp::logRequest(const crow::request &req) {
    Logger::info("HTTP Request: " + req.method_name + " " + req.url);
}

void WebApp::logResponse(const crow::response &res) {
    Logger::info("HTTP Response: " + std::to_string(res.code));
}

void WebApp::logError(const std::string &errorMsg) {
    Logger::error("Error: " + errorMsg);
}

void WebApp::handleLogsPage(const crow::request &req, crow::response &res) {
    res.write("Страница логов");
    res.end();
}

void WebApp::handleSchedulePage(const crow::request &req, crow::response &res) {
    res.write("Страница расписания");
    res.end();
}

void WebApp::handleUserManagementPage(const crow::request &req, crow::response &res) {
    res.write("Страница управления пользователями");
    res.end();
}

void WebApp::handleReportsPage(const crow::request &req, crow::response &res) {
    res.write("Страница отчетов");
    res.end();
}

void WebApp::handleLoginPage(const crow::request &req, crow::response &res) {
    res.write("Страница входа");
    res.end();
}

void WebApp::handleExportLogsJSON(const crow::request &req, crow::response &res) {
    res.write("Экспорт логов в формате JSON");
    res.end();
}

void WebApp::handleExportLogsXML(const crow::request &req, crow::response &res) {
    res.write("Экспорт логов в формате XML");
    res.end();
}

void WebApp::handleExportLogsCSV(const crow::request &req, crow::response &res) {
    res.write("Экспорт логов в формате CSV");
    res.end();
}

void WebApp::handleCreateTask(const crow::request &req, crow::response &res) {
    res.write("Создание задачи");
    res.end();
}

void WebApp::handleDeleteTask(const crow::request &req, crow::response &res) {
    res.write("Удаление задачи");
    res.end();
}

void WebApp::handleUpdateTask(const crow::request &req, crow::response &res) {
    res.write("Обновление задачи");
    res.end();
}

void WebApp::handleGenerateReport(const crow::request &req, crow::response &res) {
    res.write("Генерация отчета");
    res.end();
}

void WebApp::handleExportReport(const crow::request &req, crow::response &res) {
    res.write("Экспорт отчета");
    res.end();
}

void WebApp::handleTwoFactorAuth(const crow::request &req, crow::response &res) {
    res.write("Двухфакторная аутентификация");
    res.end();
}

void WebApp::handleSocialAuth(const crow::request &req, crow::response &res) {
    res.write("Социальная аутентификация");
    res.end();
}

void WebApp::handleRealTimeNotifications(const crow::request &req, crow::response &res) {
    res.write("Уведомления в реальном времени");
    res.end();
}

void WebApp::handleLanguageChange(const crow::request &req, crow::response &res) {
    res.write("Смена языка");
    res.end();
}

void WebApp::handleThemeChange(const crow::request &req, crow::response &res) {
    res.write("Смена темы");
    res.end();
}

void WebApp::sendNotification(const std::string &message) {
    notificationManager.send(message);
    cloudUtils.logNotification(message);
}

















