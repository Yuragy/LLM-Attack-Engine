#ifndef PHISHING_ATTACK_H
#define PHISHING_ATTACK_H

#include "../logging/logger.h"
#include "../dictionary/dictionary_loader.h"
#include "../machine_learning/ml_predictor.h" 
#include "../utils/threading_utils.h"         
#include "../database/db_manager.h"           
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>
#include <future>
#include <atomic>

class PhishingAttack {
public:
    PhishingAttack(DictionaryLoader* dictionaryLoader, Logger* logger, 
                   MLPredictor* mlPredictor, ThreadingUtils* threadingUtils, 
                   DBManager* dbManager);

    void execute();
    void setPageSubmissionCallback(std::function<void(const std::string&)> callback);

private:
    DictionaryLoader* dictionaryLoader;
    Logger* logger;
    MLPredictor* mlPredictor;                
    ThreadingUtils* threadingUtils;          
    DBManager* dbManager;                    

    std::function<void(const std::string&)> pageSubmissionCallback;
    std::mutex mtx;
    std::queue<std::string> pageQueue;
    std::atomic<bool> stopFlag = false;
    std::vector<std::string> loadPhishingPages();
    void applyMachineLearningModel(std::vector<std::string>& pages); 
    void logAttackDetails(const std::string& pageContent);
    void phishingWorker(); 
    void evaluateModel();       
    void analyzeErrors();       
    void manageResources();     
    bool checkIfStop();
    void connectToDatabase();   
    void disconnectFromDatabase(); 
};

#endif // PHISHING_ATTACK_H



