#include "WAL.h"

/* =========================
   WAL Implementation
   ========================= */

LSN WAL::append(const std::string& key, const std::string& value, TimeMs now_ms) {
    WalEntry e;
    e.lsn = next_lsn_++;
    e.key = key;
    e.value = value;
    e.created_ms = now_ms;
    log_.push_back(e);
    return e.lsn;
}

bool WAL::getEntry(LSN lsn, WalEntry& out) const {
    if (lsn <= 0) return false;
    // Our LSNs start at 1 and increment by 1, stored in a vector
    std::size_t idx = static_cast<std::size_t>(lsn - 1);
    if (idx >= log_.size()) return false;
    out = log_[idx];
    return true;
}

LSN WAL::lastLSN() const {
    return log_.empty() ? 0 : log_.back().lsn;
}

const std::vector<WalEntry>& WAL::entries() const {
    return log_;
}