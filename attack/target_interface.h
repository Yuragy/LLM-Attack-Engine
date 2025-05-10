#ifndef TARGET_INTERFACE_H
#define TARGET_INTERFACE_H

#include <string>
#include <vector>
#include <future>
#include <map>

class TargetInterface {
public:
    virtual ~TargetInterface() = default;
    virtual bool initialize(const std::string& config) = 0;
    virtual std::future<bool> executeAttackAsync(const std::string& attackType, const std::string& parameters) = 0;
    virtual std::string getStatus() const = 0;
    virtual std::vector<std::string> getLogs() const = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual std::vector<std::string> getAvailableAttacks() const = 0;
    virtual std::map<std::string, double> getMetrics() const = 0;
    virtual void setParameters(const std::map<std::string, std::string>& params) = 0;
    virtual std::vector<std::string> getNotifications() const = 0;
};

#endif // TARGET_INTERFACE_H

