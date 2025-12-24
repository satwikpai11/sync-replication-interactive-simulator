#include "ConsistencyChecker.h"

/* =========================
   ConsistencyChecker Impl
   ========================= */

ConsistencyChecker::Result
ConsistencyChecker::check(const KVStore& a, const KVStore& b) {
    Result r;
    r.hashA = a.stateHash();
    r.hashB = b.stateHash();
    r.consistent = (r.hashA == r.hashB);

    if (r.consistent) {
        r.summary = "Primary and Secondary are consistent.";
    } else {
        r.summary = "DIVERGENCE detected: Primary and Secondary hashes differ.";
    }
    return r;
}