cmake_minimum_required(VERSION 3.10)

project(HubAI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

find_package(OpenSSL REQUIRED)          
find_package(YamlCpp REQUIRED)        
find_package(PugiXML REQUIRED)          
find_package(nlohmann_json REQUIRED)  
find_package(CURL REQUIRED)             
find_package(CUDA REQUIRED)             
find_package(Threads REQUIRED)           
find_package(SQLite3 REQUIRED)          
find_package(Armadillo REQUIRED)         
find_package(mlpack REQUIRED)            
find_package(Python3 COMPONENTS Interpreter Development REQUIRED) 
find_package(TBB REQUIRED)               

set(HTTPLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/path/to/httplib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/path/to/matplotlibcpp)

set(SOURCES
    src/data_utils.cpp
    src/cloud_utils.cpp
    src/gpu_utils.cpp
    src/notification_utils.cpp
    src/threading_utils.cpp
    src/notification_manager.cpp
    src/gpu_manager.cpp
    src/logger.cpp
    src/config.cpp
    src/monitor.cpp
    src/db_manager.cpp
    src/cloud_integration.cpp
    src/rule_engine.cpp
    src/dictionary_loader.cpp
    src/ml_model_trainer.cpp
    src/ml_predictor.cpp
    src/auth.cpp
    src/analytics_manager.cpp
    src/policy_manager.cpp
    src/user_management.cpp
    src/auto_recovery.cpp
    src/brute_force_attack.cpp
    src/reverse_attack.cpp  
    src/combination_attack.cpp
    src/credential_stuffing_attack.cpp
    src/dictionary_attack.cpp
    src/fingerprint_attack.cpp
    src/hybrid_attack.cpp
    src/markov_attack.cpp
    src/mask_attack.cpp
    src/pass_the_hash_attack.cpp
    scr/rainbow_table_attack.cpp
    scr/phishing_attack.cpp
    scr/permuted_dictionary_attack.cpp
    scr/pattern_based_attack.cpp
    scr/rule_based_attack.cpp
    scr/social_engineering_attack.cpp
    scr/statistical_attack.cpp
    scr/timing_attack.cpp
    scr/bitcoin_core.cpp
    scr/social_data_collector.cpp
    scr/personalized_dictionary_generator.cpp
    scr/bitcoin_core.cpp
    scr/bitlocker.cpp
    scr/blockchain.cpp
    scr/electrum.cpp
    scr/exodus.cpp
    scr/metamask.cpp
    scr/phantom_wallet.cpp
    scr/ronin_wallet.cpp
    scr/veracrypt.cpp
    scr/adaptive_attack_manager.cpp
    scr/attack_engine.cpp
    scr/api.cpp
    scr/target_factory.cpp
    scr/scheduler.cpp
    scr/app.cpp
    scr/cli.cpp
    src/main.cpp
)

set(HEADERS
    src/data_utils.h
    src/cloud_utils.h
    src/gpu_utils.h
    src/notification_utils.h
    src/threading_utils.h
    src/notification_manager.h
    src/gpu_manager.h
    src/logger.h
    src/config.h
    src/monitor.h
    src/db_manager.h
    src/cloud_integration.h
    src/rule_engine.h
    src/dictionary_loader.h
    src/ml_model_trainer.h
    src/ml_predictor.h
    src/auth.h
    src/analytics_manager.h
    src/policy_manager.h
    src/user_management.h
    src/auto_recovery.h
    src/brute_force_attack.h
    src/reverse_attack.h  
    src/combination_attack.h
    src/credential_stuffing_attack.h
    src/dictionary_attack.h
    src/fingerprint_attack.h
    src/hybrid_attack.h
    src/markov_attack.h
    src/mask_attack.h
    src/pass_the_hash_attack.h
    scr/rainbow_table_attack.h
    scr/phishing_attack.h
    scr/permuted_dictionary_attack.h
    scr/pattern_based_attack.h
    scr/rule_based_attack.h
    scr/social_engineering_attack.h
    scr/statistical_attack.h
    scr/timing_attack.h
    scr/bitcoin_core.h
    scr/social_data_collector.h
    scr/personalized_dictionary_generator.h
    scr/bitcoin_core.h
    scr/bitlocker.h
    scr/blockchain.h
    scr/electrum.h
    scr/exodus.h
    scr/metamask.h
    scr/phantom_wallet.h
    scr/ronin_wallet.h
    scr/veracrypt.h
    scr/adaptive_attack_manager.h
    scr/attack_engine.h
    scr/api.h
    scr/target_interface.h
    scr/target_factory.h
    scr/scheduler.h
    scr/app.h
    scr/cli.h
)

add_executable(SecurityToolbox ${SOURCES} ${HEADERS})

target_link_libraries(SecurityToolbox 
    OpenSSL::SSL  
    YamlCpp::yaml-cpp 
    PugiXML::pugixml 
    nlohmann_json::nlohmann_json 
    CURL::libcurl
    SQLite::SQLite3  
    CUDA::cudart
    Threads::Threads
    Armadillo::armadillo 
    mlpack::mlpack 
    ${Python3_LIBRARIES} 
    TBB::tbb 
)

target_include_directories(SecurityToolbox PRIVATE ${HTTPLIB_DIR})


target_include_directories(SecurityToolbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

install(TARGETS SecurityToolbox DESTINATION bin)

