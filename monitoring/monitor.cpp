#include "monitor.h"
#include <fstream>
#include <sstream>
#include <iostream>

Monitor::Monitor(std::shared_ptr<Config> config, std::shared_ptr<Logger> logger, std::shared_ptr<DBManager> dbManager)
    : config(config), 
      isMonitoring(false), 
      notificationManager(config, logger), 
      dbManager(dbManager), 
      threadingUtils(logger, config) {}

Monitor::~Monitor() {
    stopMonitoring();
}

void Monitor::initialize(const std::string& configFilePath) {
    loadMonitoringConfig(configFilePath);
    threadingUtils.enableMonitoring();  
    notificationManager.init(configFilePath);  
    dbManager->connect();
    dbManager->startPerformanceMonitoring();
    Logger::info("Мониторинг инициализирован");

    auto systemStatus = config->getSystemStatus();
    Logger::info("Начальное состояние системы: " + systemStatus);
}

void Monitor::startMonitoring() {
    if (isMonitoring) {
        Logger::warning("Мониторинг уже запущен");
        return;
    }
    isMonitoring = true;
    monitoringThread = std::thread(&Monitor::monitoringLoop, this);
    Logger::info("Мониторинг запущен");
}

void Monitor::stopMonitoring() {
    if (!isMonitoring) {
        Logger::warning("Мониторинг не запущен");
        return;
    }
    isMonitoring = false;
    threadingUtils.disableMonitoring();  
    dbManager->stopPerformanceMonitoring();
    dbManager->disconnect();
    if (monitoringThread.joinable()) {
        monitoringThread.join();
    }
    Logger::info("Мониторинг остановлен");
}

void Monitor::monitorAttackStatus(const std::string& attackId, const std::string& status) {
    std::string message = "Статус атаки " + attackId + ": " + status;
    logAndNotify(message, LogLevel::INFO);
    dbManager->logDBOperation("Monitor Attack Status", message);
    attackStatusHistory.push_back({attackId, status});
}

void Monitor::monitorGPUMetrics(const std::map<std::string, double>& metrics) {
    std::ostringstream oss;
    oss << "GPU Metrics: ";
    for (const auto& [key, value] : metrics) {
        oss << key << "=" << value << ", ";
        gpuMetricsHistory[key].push_back(value); 
    }
    logAndNotify(oss.str(), LogLevel::INFO);
    dbManager->logDBOperation("Monitor GPU Metrics", oss.str());
}

void Monitor::monitorMLTraining(const std::string& modelId, const std::string& status) {
    std::string message = "Статус обучения модели " + modelId + ": " + status;
    logAndNotify(message, LogLevel::INFO);
    dbManager->logDBOperation("Monitor ML Training", message);
    mlTrainingHistory[modelId].push_back(status);
}

void Monitor::monitorMLPrediction(const std::string& modelId, const std::string& status) {
    std::string message = "Статус предсказания модели " + modelId + ": " + status;
    logAndNotify(message, LogLevel::INFO);
    dbManager->logDBOperation("Monitor ML Prediction", message);
    mlPredictionHistory[modelId].push_back(status);
}

void Monitor::monitorRuleApplication(const std::string& ruleName, const std::string& target, bool success) {
    std::string status = success ? "успешно применено" : "ошибка применения";
    std::string message = "Правило " + ruleName + " было " + status + " для цели " + target;
    logAndNotify(message, success ? LogLevel::INFO : LogLevel::ERROR);
    dbManager->logDBOperation("Rule Application Monitoring", message);
    ruleApplicationHistory.push_back({ruleName, target, success});
}

void Monitor::monitorDictionaryUsage(const std::string& dictionaryName, bool loaded) {
    std::string status = loaded ? "загружен" : "не удалось загрузить";
    std::string message = "Словарь " + dictionaryName + " был " + status;
    logAndNotify(message, loaded ? LogLevel::INFO : LogLevel::ERROR);
    dbManager->logDBOperation("Dictionary Usage Monitoring", message);
    dictionaryUsageHistory.push_back({dictionaryName, loaded});
}

void Monitor::monitorUserManagement(const std::string& username, const std::string& action, bool success) {
    std::string status = success ? "успешно выполнено" : "не выполнено";
    std::string message = "Действие " + action + " для пользователя " + username + " было " + status;
    logAndNotify(message, success ? LogLevel::INFO : LogLevel::ERROR);
    dbManager->logDBOperation("User Management Monitoring", message);
    userManagementHistory.push_back({username, action, success});
}

void Monitor::monitorTaskScheduling(const std::string& taskId, const std::string& status) {
    std::string message = "Задача " + taskId + " находится в состоянии " + status;
    logAndNotify(message, LogLevel::INFO);
    dbManager->logDBOperation("Task Scheduling Monitoring", message);
    taskSchedulingHistory.push_back({taskId, status});
}

void Monitor::monitorNotification(const std::string& notificationType, const std::string& recipient, bool success) {
    std::string status = success ? "успешно отправлено" : "ошибка отправки";
    std::string message = "Уведомление типа " + notificationType + " для " + recipient + " было " + status;
    logAndNotify(message, success ? LogLevel::INFO : LogLevel::ERROR);
    dbManager->logDBOperation("Notification Monitoring", message);
    notificationHistory.push_back({notificationType, recipient, success});
}

void Monitor::monitorWebApp(const std::string& endpoint, const std::string& status) {
    std::string message = "Веб-приложение: обработка запроса на " + endpoint + " со статусом " + status;
    logAndNotify(message, LogLevel::INFO);
    dbManager->logDBOperation("Web Application Monitoring", message);
    webAppHistory.push_back({endpoint, status});
}

void Monitor::monitorCloudResources(const std::string& resourceName, const std::string& status) {
    std::string message = "Облачный ресурс " + resourceName + " находится в состоянии " + status;
    logAndNotify(message, LogLevel::INFO);
    dbManager->logDBOperation("Cloud Resource Monitoring", message);
    cloudResourcesHistory.push_back({resourceName, status});
}

void Monitor::monitorSocialEngineering(const std::string& campaignName, bool success) {
    std::string status = success ? "успешно завершена" : "провалена";
    std::string message = "Кампания социальной инженерии " + campaignName + " была " + status;
    logAndNotify(message, success ? LogLevel::INFO : LogLevel::ERROR);
    dbManager->logDBOperation("Social Engineering Monitoring", message);
    socialEngineeringHistory.push_back({campaignName, success});
}

void Monitor::monitorAnalytics(const std::string& analysisType, const std::string& result) {
    std::string message = "Аналитический процесс " + analysisType + " завершен с результатом: " + result;
    logAndNotify(message, LogLevel::INFO);
    dbManager->logDBOperation("Analytics Monitoring", message);
    analyticsHistory.push_back({analysisType, result});
}
void Monitor::monitorCLIAndAPI(const std::string& requestType, const std::string& endpoint, bool success) {
    std::string status = success ? "успешно выполнен" : "ошибка выполнения";
    std::string message = requestType + " запрос к " + endpoint + " был " + status;
    logAndNotify(message, success ? LogLevel::INFO : LogLevel::ERROR);
    dbManager->logDBOperation("CLI/API Monitoring", message);
    cliAndAPIHistory.push_back({requestType, endpoint, success});
}
void Monitor::generateAttackStatusReport(std::ofstream& reportFile) {
    reportFile << "Отчет по состоянию атак\n";
    reportFile << "========================\n";
    for (const auto& [attackId, status] : attackStatusHistory) {
        reportFile << "Атака ID: " << attackId << " - Статус: " << status << "\n";
    }
}

void Monitor::generateGPUMetricsReport(std::ofstream& reportFile) {
    reportFile << "Отчет по метрикам GPU\n";
    reportFile << "=====================\n";
    for (const auto& [metric, values] : gpuMetricsHistory) {
        reportFile << "Метрика: " << metric << "\n";
        for (const auto& value : values) {
            reportFile << value << "\n";
        }
        reportFile << "-------------------\n";
    }
}

void Monitor::generateMLTrainingReport(std::ofstream& reportFile) {
    reportFile << "Отчет по обучению моделей\n";
    reportFile << "=========================\n";
    for (const auto& [modelId, statuses] : mlTrainingHistory) {
        reportFile << "Модель ID: " << modelId << "\n";
        for (const auto& status : statuses) {
            reportFile << status << "\n";
        }
        reportFile << "-------------------\n";
    }
}

void Monitor::generateMLPredictionReport(std::ofstream& reportFile) {
    reportFile << "Отчет по предсказанию моделей\n";
    reportFile << "=============================\n";
    for (const auto& [modelId, statuses] : mlPredictionHistory) {
        reportFile << "Модель ID: " << modelId << "\n";
        for (const auto& status : statuses) {
            reportFile << status << "\n";
        }
        reportFile << "-------------------\n";
    }
}

void Monitor::generateRuleApplicationReport(std::ofstream& reportFile) {
    reportFile << "Отчет по применению правил\n";
    reportFile << "==========================\n";
    for (const auto& [ruleName, target, success] : ruleApplicationHistory) {
        reportFile << "Правило: " << ruleName << " - Цель: " << target << " - Статус: " 
                   << (success ? "Успех" : "Ошибка") << "\n";
    }
}

void Monitor::generateDictionaryUsageReport(std::ofstream& reportFile) {
    reportFile << "Отчет по использованию словарей\n";
    reportFile << "===============================\n";
    for (const auto& [dictionaryName, loaded] : dictionaryUsageHistory) {
        reportFile << "Словарь: " << dictionaryName << " - Статус: " 
                   << (loaded ? "Загружен" : "Ошибка загрузки") << "\n";
    }
}

void Monitor::generateUserManagementReport(std::ofstream& reportFile) {
    reportFile << "Отчет по управлению пользователями\n";
    reportFile << "==================================\n";
    for (const auto& [username, action, success] : userManagementHistory) {
        reportFile << "Пользователь: " << username << " - Действие: " << action << " - Статус: " 
                   << (success ? "Успех" : "Ошибка") << "\n";
    }
}

void Monitor::generateTaskSchedulingReport(std::ofstream& reportFile) {
    reportFile << "Отчет по планированию задач\n";
    reportFile << "===========================\n";
    for (const auto& [taskId, status] : taskSchedulingHistory) {
        reportFile << "Задача ID: " << taskId << " - Статус: " << status << "\n";
    }
}

void Monitor::generateNotificationReport(std::ofstream& reportFile) {
    reportFile << "Отчет по уведомлениям\n";
    reportFile << "=====================\n";
    for (const auto& [type, recipient, success] : notificationHistory) {
        reportFile << "Тип: " << type << " - Получатель: " << recipient << " - Статус: " 
                   << (success ? "Отправлено" : "Ошибка отправки") << "\n";
    }
}

void Monitor::generateWebAppReport(std::ofstream& reportFile) {
    reportFile << "Отчет по веб-приложению\n";
    reportFile << "=======================\n";
    for (const auto& [endpoint, status] : webAppHistory) {
        reportFile << "Эндпоинт: " << endpoint << " - Статус: " << status << "\n";
    }
}

void Monitor::generateCloudResourcesReport(std::ofstream& reportFile) {
    reportFile << "Отчет по облачным ресурсам\n";
    reportFile << "==========================\n";
    for (const auto& [resourceName, status] : cloudResourcesHistory) {
        reportFile << "Ресурс: " << resourceName << " - Статус: " << status << "\n";
    }
}

void Monitor::generateSocialEngineeringReport(std::ofstream& reportFile) {
    reportFile << "Отчет по социальной инженерии\n";
    reportFile << "=============================\n";
    for (const auto& [campaignName, success] : socialEngineeringHistory) {
        reportFile << "Кампания: " << campaignName << " - Статус: " 
                   << (success ? "Успех" : "Провал") << "\n";
    }
}

void Monitor::generateAnalyticsReport(std::ofstream& reportFile) {
    reportFile << "Отчет по аналитике\n";
    reportFile << "==================\n";
    for (const auto& [analysisType, result] : analyticsHistory) {
        reportFile << "Аналитика: " << analysisType << " - Результат: " << result << "\n";
    }
}

void Monitor::generateCLIAndAPIReport(std::ofstream& reportFile) {
    reportFile << "Отчет по CLI и API\n";
    reportFile << "===================\n";
    for (const auto& [requestType, endpoint, success] : cliAndAPIHistory) {
        reportFile << "Запрос: " << requestType << " - Эндпоинт: " << endpoint << " - Статус: " 
                   << (success ? "Успех" : "Ошибка") << "\n";
    }
}
void Monitor::generateAttackStatusGraph(const std::string& outputPath) {
    std::vector<double> x;
    std::vector<std::string> statuses;

    for (size_t i = 0; i < attackStatusHistory.size(); ++i) {
        x.push_back(i + 1);
        statuses.push_back(attackStatusHistory[i].second);
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Статус атак");
    matplotlibcpp::xlabel("Порядковый номер атаки");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/attack_status.png");
}

void Monitor::generateGPUMetricsGraph(const std::string& outputPath) {
    for (const auto& [metricName, values] : gpuMetricsHistory) {
        std::vector<double> x(values.size());
        std::iota(x.begin(), x.end(), 1);

        matplotlibcpp::plot(x, values);
        matplotlibcpp::title("GPU Metrics - " + metricName);
        matplotlibcpp::xlabel("Время");
        matplotlibcpp::ylabel(metricName);
        matplotlibcpp::save(outputPath + "/gpu_metrics_" + metricName + ".png");
        matplotlibcpp::clf();
    }
}

void Monitor::generateMLTrainingGraph(const std::string& outputPath) {
    for (const auto& [modelId, statuses] : mlTrainingHistory) {
        std::vector<double> x(statuses.size());
        std::iota(x.begin(), x.end(), 1);

        matplotlibcpp::plot(x, statuses);
        matplotlibcpp::title("Обучение модели " + modelId);
        matplotlibcpp::xlabel("Итерация");
        matplotlibcpp::ylabel("Статус");
        matplotlibcpp::save(outputPath + "/ml_training_" + modelId + ".png");
        matplotlibcpp::clf();
    }
}

void Monitor::generateMLPredictionGraph(const std::string& outputPath) {
    for (const auto& [modelId, statuses] : mlPredictionHistory) {
        std::vector<double> x(statuses.size());
        std::iota(x.begin(), x.end(), 1);

        matplotlibcpp::plot(x, statuses);
        matplotlibcpp::title("Предсказание модели " + modelId);
        matplotlibcpp::xlabel("Итерация");
        matplotlibcpp::ylabel("Статус");
        matplotlibcpp::save(outputPath + "/ml_prediction_" + modelId + ".png");
        matplotlibcpp::clf();
    }
}

void Monitor::generateRuleApplicationGraph(const std::string& outputPath) {
    std::vector<double> x(ruleApplicationHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;
    
    for (const auto& [ruleName, target, success] : ruleApplicationHistory) {
        statuses.push_back(success ? "Успех" : "Ошибка");
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Применение правил");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/rule_application.png");
}

void Monitor::generateDictionaryUsageGraph(const std::string& outputPath) {
    std::vector<double> x(dictionaryUsageHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [dictionaryName, loaded] : dictionaryUsageHistory) {
        statuses.push_back(loaded ? "Загружен" : "Ошибка");
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Использование словарей");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/dictionary_usage.png");
}

void Monitor::generateUserManagementGraph(const std::string& outputPath) {
    std::vector<double> x(userManagementHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [username, action, success] : userManagementHistory) {
        statuses.push_back(success ? "Успех" : "Ошибка");
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Управление пользователями");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/user_management.png");
}

void Monitor::generateTaskSchedulingGraph(const std::string& outputPath) {
    std::vector<double> x(taskSchedulingHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [taskId, status] : taskSchedulingHistory) {
        statuses.push_back(status);
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Планирование задач");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/task_scheduling.png");
}

void Monitor::generateNotificationGraph(const std::string& outputPath) {
    std::vector<double> x(notificationHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [type, recipient, success] : notificationHistory) {
        statuses.push_back(success ? "Отправлено" : "Ошибка");
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Уведомления");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/notifications.png");
}

void Monitor::generateWebAppGraph(const std::string& outputPath) {
    std::vector<double> x(webAppHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [endpoint, status] : webAppHistory) {
        statuses.push_back(status);
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Веб-приложение");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/web_app.png");
}

void Monitor::generateCloudResourcesGraph(const std::string& outputPath) {
    std::vector<double> x(cloudResourcesHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [resourceName, status] : cloudResourcesHistory) {
        statuses.push_back(status);
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Облачные ресурсы");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/cloud_resources.png");
}

void Monitor::generateSocialEngineeringGraph(const std::string& outputPath) {
    std::vector<double> x(socialEngineeringHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [campaignName, success] : socialEngineeringHistory) {
        statuses.push_back(success ? "Успех" : "Провал");
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("Социальная инженерия");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/social_engineering.png");
}

void Monitor::generateAnalyticsGraph(const std::string& outputPath) {
    std::vector<double> x(analyticsHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> results;

    for (const auto& [analysisType, result] : analyticsHistory) {
        results.push_back(result);
    }

    matplotlibcpp::plot(x, results);
    matplotlibcpp::title("Аналитика");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Результат");
    matplotlibcpp::save(outputPath + "/analytics.png");
}

void Monitor::generateCLIAndAPIGraph(const std::string& outputPath) {
    std::vector<double> x(cliAndAPIHistory.size());
    std::iota(x.begin(), x.end(), 1);
    std::vector<std::string> statuses;

    for (const auto& [requestType, endpoint, success] : cliAndAPIHistory) {
        statuses.push_back(success ? "Успех" : "Ошибка");
    }

    matplotlibcpp::plot(x, statuses);
    matplotlibcpp::title("CLI и API");
    matplotlibcpp::xlabel("Порядковый номер");
    matplotlibcpp::ylabel("Статус");
    matplotlibcpp::save(outputPath + "/cli_api.png");
}
void Monitor::generateReports() {
    std::ofstream reportFile("/path/to/report.txt", std::ios::out);
    if (!reportFile.is_open()) {
        Logger::error("Не удалось открыть файл для записи отчета");
        return;
    }

    generateAttackStatusReport(reportFile);
    generateGPUMetricsReport(reportFile);
    generateMLTrainingReport(reportFile);
    generateMLPredictionReport(reportFile);
    generateRuleApplicationReport(reportFile);
    generateDictionaryUsageReport(reportFile);
    generateUserManagementReport(reportFile);
    generateTaskSchedulingReport(reportFile);
    generateNotificationReport(reportFile);
    generateWebAppReport(reportFile);
    generateCloudResourcesReport(reportFile);
    generateSocialEngineeringReport(reportFile);
    generateAnalyticsReport(reportFile);
    generateCLIAndAPIReport(reportFile);

    reportFile.close();
}

void Monitor::generateGraphs() {
    std::string outputPath = "/path/to/output";
    generateAttackStatusGraph(outputPath);
    generateGPUMetricsGraph(outputPath);
    generateMLTrainingGraph(outputPath);
    generateMLPredictionGraph(outputPath);
    generateRuleApplicationGraph(outputPath);
    generateDictionaryUsageGraph(outputPath);
    generateUserManagementGraph(outputPath);
    generateTaskSchedulingGraph(outputPath);
    generateNotificationGraph(outputPath);
    generateWebAppGraph(outputPath);
    generateCloudResourcesGraph(outputPath);
    generateSocialEngineeringGraph(outputPath);
    generateAnalyticsGraph(outputPath);
    generateCLIAndAPIGraph(outputPath);
}

void Monitor::logAndNotify(const std::string& message, LogLevel level, const std::unordered_set<std::string>& tags) {
    try {
        Logger::log(message, level, tags);
        if (level == LogLevel::ERROR || level == LogLevel::WARNING) {
            sendCriticalNotification(message);
        }
        dbManager->logEvent(message, level);
    } catch (const std::exception& e) {
        Logger::error("Ошибка при логировании и уведомлении: " + std::string(e.what()));
    }
}

void Monitor::monitoringLoop() {
    try {
        while (isMonitoring) {
            monitorThreading();
            monitorDatabasePerformance();
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    } catch (const std::exception& e) {
        Logger::error("Ошибка в цикле мониторинга: " + std::string(e.what()));
    }
}

void Monitor::monitorThreading() {
    try {
        auto metrics = threadingUtils.getMetrics();
        for (const auto& metric : metrics) {
            Logger::info(metric);
            dbManager->logDBOperation("Threading Metric", metric);
        }
    } catch (const std::exception& e) {
        Logger::error("Ошибка при мониторинге многопоточности: " + std::string(e.what()));
    }
}

void Monitor::monitorDatabasePerformance() {
    try {
        dbManager->monitorRealTime();
    } catch (const std::exception& e) {
        Logger::error("Ошибка при мониторинге производительности базы данных: " + std::string(e.what()));
    }
}












