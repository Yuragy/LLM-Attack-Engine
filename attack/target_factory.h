#ifndef TARGET_FACTORY_H
#define TARGET_FACTORY_H

#include "target_interface.h"
#include "db_manager.h"
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

class TargetFactory {
public:
    static std::unique_ptr<TargetInterface> createTarget(const std::string& targetType);
    static void registerTarget(const std::string& targetType, std::function<std::unique_ptr<TargetInterface>()> creator);
    static void saveTargetToDatabase(const std::string& targetType, const std::string& targetData);
    static std::vector<std::string> loadTargetsFromDatabase();
private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<TargetInterface>()>> targetRegistry;
    static std::shared_ptr<DBManager> dbManager;
};

#endif // TARGET_FACTORY_H




