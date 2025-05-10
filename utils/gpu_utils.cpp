#include "gpu_utils.h"
#include <iostream>
#include <stdexcept>

GPUUtils::GPUUtils(std::shared_ptr<Logger> logger, std::shared_ptr<Config> config)
    : logger(logger), config(config), stopFlag(false), activeTasks(0), monitoringEnabled(false), restartTasks(true) {
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) {
        throw std::runtime_error("Failed to initialize NVML: " + std::string(nvmlErrorString(result)));
    }
    initialize();
}

GPUUtils::~GPUUtils() {
    releaseResources();
    nvmlReturn_t result = nvmlShutdown();
    if (result != NVML_SUCCESS) {
        logEvent("Failed to shutdown NVML: " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
    }
}

void GPUUtils::initialize() {
    logEvent("Initializing GPU resources...", LogLevel::INFO);
    unsigned int deviceCount;
    nvmlReturn_t result = nvmlDeviceGetCount(&deviceCount);
    if (result != NVML_SUCCESS) {
        logEvent("Failed to get CUDA device count: " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
        throw std::runtime_error("Failed to get CUDA device count: " + std::string(nvmlErrorString(result)));
    }
    if (deviceCount == 0) {
        logEvent("No CUDA devices found.", LogLevel::ERROR);
        throw std::runtime_error("No CUDA devices found.");
    }
    maxGPU = deviceCount;
    logEvent("Found " + std::to_string(maxGPU) + " CUDA devices.", LogLevel::INFO);
}

void GPUUtils::releaseResources() {
    stopTasks();
    cudaError_t err = cudaDeviceReset();
    if (err != cudaSuccess) {
        logEvent("Failed to reset CUDA device: " + std::string(cudaGetErrorString(err)), LogLevel::ERROR);
    }
    logEvent("GPU resources released.", LogLevel::INFO);
}

void GPUUtils::addTask(const std::function<void()>& task, int priority) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        taskQueue.push({task, priority});
    }
    condition.notify_one();
}

void GPUUtils::runTasks(const std::string& strategy) {
    unsigned int availableGPU = maxGPU;
    threads.resize(availableGPU);
    for (unsigned int i = 0; i < availableGPU; ++i) {
        threads[i] = std::thread(&GPUUtils::worker, this);
    }

    runWithStrategy(strategy);
}

void GPUUtils::stopTasks() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopFlag = true;
    }
    condition.notify_all();
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads.clear();
}

void GPUUtils::monitorGPU() {
    while (monitoringEnabled) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        logEvent("Monitoring GPU: active tasks = " + std::to_string(activeTasks) + ", task queue size = " + std::to_string(taskQueue.size()), LogLevel::INFO);

        for (unsigned int i = 0; i < maxGPU; ++i) {
            nvmlDevice_t device;
            nvmlReturn_t result = nvmlDeviceGetHandleByIndex(i, &device);
            if (result != NVML_SUCCESS) {
                logEvent("Failed to get handle for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
                continue;
            }

            nvmlUtilization_t utilization;
            result = nvmlDeviceGetUtilizationRates(device, &utilization);
            if (result != NVML_SUCCESS) {
                logEvent("Failed to get utilization rates for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
                continue;
            }

            unsigned int temperature;
            result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
            if (result != NVML_SUCCESS) {
                logEvent("Failed to get temperature for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
                continue;
            }

            logEvent("GPU " + std::to_string(i) + " - Utilization: " + std::to_string(utilization.gpu) + "%, Temperature: " + std::to_string(temperature) + "C", LogLevel::INFO);
        }
    }
}

std::vector<std::string> GPUUtils::getMetrics() {
    std::vector<std::string> metrics;
    for (unsigned int i = 0; i < maxGPU; ++i) {
        nvmlDevice_t device;
        nvmlReturn_t result = nvmlDeviceGetHandleByIndex(i, &device);
        if (result != NVML_SUCCESS) {
            logEvent("Failed to get handle for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
            continue;
        }

        nvmlMemory_t memory;
        result = nvmlDeviceGetMemoryInfo(device, &memory);
        if (result != NVML_SUCCESS) {
            logEvent("Failed to get memory info for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
            continue;
        }

        nvmlUtilization_t utilization;
        result = nvmlDeviceGetUtilizationRates(device, &utilization);
        if (result != NVML_SUCCESS) {
            logEvent("Failed to get utilization rates for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
            continue;
        }

        unsigned int temperature;
        result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
        if (result != NVML_SUCCESS) {
            logEvent("Failed to get temperature for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
            continue;
        }

        metrics.push_back("GPU " + std::to_string(i) + " - Memory Usage: " + std::to_string(memory.used / 1024 / 1024) + "MB/" + std::to_string(memory.total / 1024 / 1024) + "MB, Utilization: " + std::to_string(utilization.gpu) + "%, Temperature: " + std::to_string(temperature) + "C");
    }
    return metrics;
}

void GPUUtils::worker() {
    while (!stopFlag) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stopFlag || !taskQueue.empty(); });

            if (stopFlag && taskQueue.empty()) {
                return;
            }

            task = std::move(taskQueue.top());
            taskQueue.pop();
        }

        activeTasks++;
        try {
            task.func();
        } catch (const std::exception& e) {
            handleException(e, "Task execution");
        } catch (...) {
            handleUnknownException("Task execution");
        }
        activeTasks--;
    }
}

void GPUUtils::logEvent(const std::string& event, LogLevel level) {
    logger->log(event, level);
}

void GPUUtils::handleException(const std::exception& e, const std::string& context) {
    logEvent("Exception caught in context: " + context + " - " + e.what(), LogLevel::ERROR);
    logEvent("Cleaning up resources due to exception.", LogLevel::INFO);
    cudaError_t err = cudaDeviceReset();
    if (err != cudaSuccess) {
        logEvent("Failed to reset CUDA device: " + std::string(cudaGetErrorString(err)), LogLevel::ERROR);
    }
    logEvent("Resources cleaned up successfully.", LogLevel::INFO);
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (!restartQueue.empty()) {
            Task task = restartQueue.top();
            restartQueue.pop();
            taskQueue.push(task);
            logEvent("Task rescheduled after exception.", LogLevel::INFO);
        }
    }
}

void GPUUtils::handleUnknownException(const std::string& context) {
    logEvent("Unknown exception caught in context: " + context, LogLevel::ERROR);
    logEvent("Cleaning up resources due to unknown exception.", LogLevel::INFO);
    cudaError_t err = cudaDeviceReset();
    if (err != cudaSuccess) {
        logEvent("Failed to reset CUDA device: " + std::string(cudaGetErrorString(err)), LogLevel::ERROR);
    }
    logEvent("Resources cleaned up successfully.", LogLevel::INFO);
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (!restartQueue.empty()) {
            Task task = restartQueue.top();
            restartQueue.pop();
            taskQueue.push(task);
            logEvent("Task rescheduled after unknown exception.", LogLevel::INFO);
        }
    }
}

void GPUUtils::restartTask(const std::function<void()>& task) {
    logEvent("Restarting task.", LogLevel::INFO);
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        restartQueue.push({task, 0});
    }
    condition.notify_one();
}

void GPUUtils::runWithStrategy(const std::string& strategy) {
    logEvent("Running with strategy: " + strategy, LogLevel::INFO);
    if (strategy == "round-robin") {
        roundRobin();
    } else if (strategy == "fair-scheduling") {
        fairScheduling();
    } else if (strategy == "least-loaded") {
        leastLoaded();
    } else if (strategy == "temperature-based") {
        temperatureBased();
    } else {
        logEvent("Unknown strategy: " + strategy, LogLevel::WARNING);
        while (!taskQueue.empty()) {
            Task task = taskQueue.top();
            taskQueue.pop();
            addTask(task.func, task.priority);
        }
    }
}

void GPUUtils::roundRobin() {
    logEvent("Running with round-robin strategy", LogLevel::INFO);
    size_t numGPUs = maxGPU;
    size_t taskIndex = 0;

    while (!taskQueue.empty()) {
        Task task = taskQueue.top();
        taskQueue.pop();

        size_t gpuIndex = taskIndex % numGPUs;
        addTask([task, gpuIndex]() {
            cudaSetDevice(gpuIndex);
            task.func();
        }, task.priority);

        taskIndex++;
    }
}

void GPUUtils::fairScheduling() {
    logEvent("Running with fair-scheduling strategy", LogLevel::INFO);
    size_t numGPUs = maxGPU;
    std::vector<size_t> gpuLoads(numGPUs, 0);

    while (!taskQueue.empty()) {
        Task task = taskQueue.top();
        taskQueue.pop();

        size_t minLoadGPU = 0;
        for (size_t i = 1; i < numGPUs; ++i) {
            if (gpuLoads[i] < gpuLoads[minLoadGPU]) {
                minLoadGPU = i;
            }
        }

        gpuLoads[minLoadGPU]++;
        addTask([task, minLoadGPU]() {
            cudaSetDevice(minLoadGPU);
            task.func();
        }, task.priority);
    }
}

void GPUUtils::leastLoaded() {
    logEvent("Running with least-loaded strategy", LogLevel::INFO);
    size_t numGPUs = maxGPU;

    while (!taskQueue.empty()) {
        Task task = taskQueue.top();
        taskQueue.pop();

        size_t leastLoadedGPU = 0;
        unsigned int minUtilization = 100;

        for (size_t i = 0; i < numGPUs; ++i) {
            nvmlDevice_t device;
            nvmlReturn_t result = nvmlDeviceGetHandleByIndex(i, &device);
            if (result != NVML_SUCCESS) {
                logEvent("Failed to get handle for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
                continue;
            }

            nvmlUtilization_t utilization;
            result = nvmlDeviceGetUtilizationRates(device, &utilization);
            if (result != NVML_SUCCESS) {
                logEvent("Failed to get utilization rates for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
                continue;
            }

            if (utilization.gpu < minUtilization) {
                minUtilization = utilization.gpu;
                leastLoadedGPU = i;
            }
        }

        addTask([task, leastLoadedGPU]() {
            cudaSetDevice(leastLoadedGPU);
            task.func();
        }, task.priority);
    }
}

void GPUUtils::temperatureBased() {
    logEvent("Running with temperature-based strategy", LogLevel::INFO);
    size_t numGPUs = maxGPU;

    while (!taskQueue.empty()) {
        Task task = taskQueue.top();
        taskQueue.pop();

        size_t coolestGPU = 0;
        unsigned int minTemperature = 100;

        for (size_t i = 0; i < numGPUs; ++i) {
            nvmlDevice_t device;
            nvmlReturn_t result = nvmlDeviceGetHandleByIndex(i, &device);
            if (result != NVML_SUCCESS) {
                logEvent("Failed to get handle for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
                continue;
            }

            unsigned int temperature;
            result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
            if (result != NVML_SUCCESS) {
                logEvent("Failed to get temperature for GPU " + std::to_string(i) + ": " + std::string(nvmlErrorString(result)), LogLevel::ERROR);
                continue;
            }

            if (temperature < minTemperature) {
                minTemperature = temperature;
                coolestGPU = i;
            }
        }

        addTask([task, coolestGPU]() {
            cudaSetDevice(coolestGPU);
            task.func();
        }, task.priority);
    }
}



