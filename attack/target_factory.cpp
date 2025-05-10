#include "target_factory.h"
#include <stdexcept>
#include "metamask.h"
#include "exodus.h"
#include "electrum.h"
#include "bitcoin_core.h"
#include "blockchain.h"
#include "ronin_wallet.h"
#include "phantom_wallet.h"
#include "veracrypt.h"
#include "bitlocker.h"
#include "dictionary_attack.h"
#include "brute_force_attack.h"
#include "mask_attack.h"
#include "rule_based_attack.h"
#include "hybrid_attack.h"
#include "rainbow_table_attack.h"
#include "markov_attack.h"
#include "combination_attack.h"
#include "permuted_dictionary_attack.h"
#include "fingerprint_attack.h"
#include "statistical_attack.h"
#include "reverse_attack.h"
#include "pattern_based_attack.h"
#include "social_engineering_attack.h"
#include "phishing_attack.h"
#include "credential_stuffing_attack.h"
#include "pass_the_hash_attack.h"
#include "timing_attack.h"

std::unordered_map<std::string, std::function<std::unique_ptr<TargetInterface>()>> TargetFactory::targetRegistry;
std::shared_ptr<DBManager> TargetFactory::dbManager = std::make_shared<DBManager>();

std::unique_ptr<TargetInterface> TargetFactory::createTarget(const std::string& targetType) {
    auto it = targetRegistry.find(targetType);
    if (it != targetRegistry.end()) {
        return it->second();
    } else {
        throw std::invalid_argument("Unknown target type: " + targetType);
    }
}

void TargetFactory::registerTarget(const std::string& targetType, std::function<std::unique_ptr<TargetInterface>()> creator) {
    targetRegistry[targetType] = creator;
}

void TargetFactory::saveTargetToDatabase(const std::string& targetType, const std::string& targetData) {
    if (!dbManager->isConnected()) {
        dbManager->connect();
    }

    std::string query = "INSERT INTO targets (type, data) VALUES (?, ?);";
    dbManager->executeQuery(query, { targetType, targetData });
}

std::vector<std::string> TargetFactory::loadTargetsFromDatabase() {
    if (!dbManager->isConnected()) {
        dbManager->connect();
    }

    std::vector<std::string> targets;
    std::string query = "SELECT data FROM targets;";
    auto results = dbManager->executeQuery(query);

    for (const auto& row : results) {
        targets.push_back(row.at("data"));
    }

    return targets;
}
namespace {
    struct TargetFactoryInitializer {
        TargetFactoryInitializer() {
            TargetFactory::registerTarget("dictionary_attack", [] { return std::make_unique<DictionaryAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("brute_force_attack", [] { return std::make_unique<BruteForceAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("mask_attack", [] { return std::make_unique<MaskAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("rule_based_attack", [] { return std::make_unique<RuleBasedAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("hybrid_attack", [] { return std::make_unique<HybridAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("rainbow_table_attack", [] { return std::make_unique<RainbowTableAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("markov_attack", [] { return std::make_unique<MarkovAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("combination_attack", [] { return std::make_unique<CombinationAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("permuted_dictionary_attack", [] { return std::make_unique<PermutedDictionaryAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("fingerprint_attack", [] { return std::make_unique<FingerprintAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("statistical_attack", [] { return std::make_unique<StatisticalAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("reverse_attack", [] { return std::make_unique<ReverseAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("pattern_based_attack", [] { return std::make_unique<PatternBasedAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("social_engineering_attack", [] { return std::make_unique<SocialEngineeringAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("phishing_attack", [] { return std::make_unique<PhishingAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("credential_stuffing_attack", [] { return std::make_unique<CredentialStuffingAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("pass_the_hash_attack", [] { return std::make_unique<PassTheHashAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("timing_attack", [] { return std::make_unique<TimingAttack>(nullptr, nullptr, nullptr); });
            TargetFactory::registerTarget("metamask", [] { return std::make_unique<MetaMaskTarget>(); });
            TargetFactory::registerTarget("exodus", [] { return std::make_unique<ExodusTarget>(); });
            TargetFactory::registerTarget("electrum", [] { return std::make_unique<ElectrumTarget>(); });
            TargetFactory::registerTarget("bitcoin_core", [] { return std::make_unique<BitcoinCoreTarget>(); });
            TargetFactory::registerTarget("blockchain", [] { return std::make_unique<BlockchainTarget>(); });
            TargetFactory::registerTarget("ronin_wallet", [] { return std::make_unique<RoninWalletTarget>(); });
            TargetFactory::registerTarget("phantom_wallet", [] { return std::make_unique<PhantomWalletTarget>(); });
            TargetFactory::registerTarget("veracrypt", [] { return std::make_unique<VeraCryptTarget>(); });
            TargetFactory::registerTarget("bitlocker", [] { return std::make_unique<BitLockerTarget>(); });
        }
    };
    static TargetFactoryInitializer initializer;
}



