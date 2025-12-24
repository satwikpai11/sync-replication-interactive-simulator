#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>
#include "types.h"

/* =========================
   Simple Key-Value Store
   ========================= */

class KVStore {
public:
    struct Entry {
        std::string value;
        int64_t version = 0;   // increments on each write
        LSN last_lsn = 0;      // last applied log sequence number
    };

    void apply(const WalEntry& e);
    bool get(const std::string& key, Entry& out) const;

    std::size_t size() const;
    std::uint64_t stateHash() const;

private:
    std::unordered_map<std::string, Entry> map_;
};