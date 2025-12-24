#pragma once
#include <vector>
#include <optional>
#include "types.h"

/* =========================
   Write-Ahead Log (WAL)
   ========================= */

class WAL {
public:
    LSN append(const std::string& key, const std::string& value, TimeMs now_ms);
    bool getEntry(LSN lsn, WalEntry& out) const;

    LSN lastLSN() const;
    const std::vector<WalEntry>& entries() const;

private:
    std::vector<WalEntry> log_;
    LSN next_lsn_ = 1;
};