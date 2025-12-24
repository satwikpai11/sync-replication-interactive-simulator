#pragma once

#include <string>
#include "KVStore.h"

/* =========================
   Consistency Checker
   ========================= */

class ConsistencyChecker {
public:
    struct Result {
        bool consistent;
        std::uint64_t hashA;
        std::uint64_t hashB;
        std::string summary;
    };

    static Result check(const KVStore& a, const KVStore& b);
};