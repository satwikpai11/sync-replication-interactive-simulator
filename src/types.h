#pragma once
#include <string>
#include <cstdint>

using TimeMs = int64_t;
using LSN = int64_t;

/* =========================
   Message + Operation Types
   ========================= */

enum class MsgType {
    REPL_WRITE,
    ACK,
    RESYNC_REQ
};

enum class Outcome {
    COMMITTED,
    FAILED,
    PENDING
};

inline const char* toString(MsgType t) {
    switch (t) {
        case MsgType::REPL_WRITE: return "REPL_WRITE";
        case MsgType::ACK:        return "ACK";
        case MsgType::RESYNC_REQ: return "RESYNC_REQ";
    }
    return "UNKNOWN";
}

inline const char* toString(Outcome o) {
    switch (o) {
        case Outcome::COMMITTED: return "COMMITTED";
        case Outcome::FAILED:    return "FAILED";
        case Outcome::PENDING:   return "PENDING";
    }
    return "UNKNOWN";
}

/* =========================
   WAL Entry + Network Msg
   ========================= */

struct WalEntry {
    LSN lsn = 0;
    std::string key;
    std::string value;
    TimeMs created_ms = 0;
};

struct NetMsg {
    MsgType type;
    int from_site = 0;     // 0 = Primary, 1 = Secondary
    int to_site = 0;
    LSN lsn = 0;           // for REPL_WRITE and ACK
    std::string key;       // for REPL_WRITE
    std::string value;     // for REPL_WRITE
    LSN last_applied = 0;  // for RESYNC_REQ
};

struct InFlightMsg {
    TimeMs deliver_ms = 0;
    NetMsg msg;
};