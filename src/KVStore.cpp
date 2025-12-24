#include "KVStore.h"
#include <vector>
#include <algorithm>
#include <functional>

/* =========================
   KVStore Implementation
   ========================= */

void KVStore::apply(const WalEntry& e) {
    auto& ent = map_[e.key];
    ent.value = e.value;
    ent.version += 1;
    ent.last_lsn = e.lsn;
}

bool KVStore::get(const std::string& key, Entry& out) const {
    auto it = map_.find(key);
    if (it == map_.end()) return false;
    out = it->second;
    return true;
}

std::size_t KVStore::size() const {
    return map_.size();
}

std::uint64_t KVStore::stateHash() const {
    // Stable hash by sorting keys (unordered_map iteration is nondeterministic)
    std::vector<std::string> keys;
    keys.reserve(map_.size());
    for (const auto& kv : map_) keys.push_back(kv.first);
    std::sort(keys.begin(), keys.end());

    std::uint64_t h = 1469598103934665603ULL; // FNV-ish seed
    auto mix = [&](std::uint64_t x) {
        h ^= x;
        h *= 1099511628211ULL;
    };

    std::hash<std::string> hs;
    for (const auto& k : keys) {
        const auto& ent = map_.at(k);
        mix((std::uint64_t)hs(k));
        mix((std::uint64_t)hs(ent.value));
        mix((std::uint64_t)ent.version);
        mix((std::uint64_t)ent.last_lsn);
    }
    return h;
}