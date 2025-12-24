#include "InteractiveConsole.h"
#include <iostream>
#include <sstream>

/* =========================
   Interactive Console Impl
   ========================= */

static constexpr TimeMs WRITE_TIMEOUT_MS = 200;

InteractiveConsole::InteractiveConsole(Site& p,
                                       Site& s,
                                       Network& n)
    : primary_(p), secondary_(s), network_(n) {}

void InteractiveConsole::run() {
    std::cout << "Interactive Synchronous Replication Simulator\n";
    std::cout << "Commands: write <k> <v> | tick <ms> | status | secondary up|down | exit\n";

    std::string line;
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit") break;
        handleCommand(line);
    }
}

void InteractiveConsole::handleCommand(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "write") {
        std::string k, v;
        iss >> k >> v;
        cmdWrite(k, v);
    } else if (cmd == "tick") {
        TimeMs t;
        iss >> t;
        cmdTick(t);
    } else if (cmd == "status") {
        cmdStatus();
    } else if (cmd == "secondary") {
        std::string state;
        iss >> state;
        cmdSecondaryState(state);
    } else {
        std::cout << "Unknown command\n";
    }
}

void InteractiveConsole::cmdWrite(const std::string& key,
                                  const std::string& value) {
    if (primary_.isWritePending()) {
        std::cout << "Write blocked: another write is pending\n";
        return;
    }

    LSN lsn = primary_.startWrite(key, value, now_);
    pending_start_ms_ = now_;
    std::cout << "Issued write LSN=" << lsn << " (waiting for secondary ACK)\n";
}

void InteractiveConsole::cmdTick(TimeMs delta) {
    bool committed = false;

    for (TimeMs i = 0; i < delta; ++i) {
        now_++;

        std::vector<NetMsg> msgs;
        network_.deliverReady(now_, msgs);

        for (auto& m : msgs) {
            if (m.to_site == 0)
                primary_.onMessage(m, now_);
            else
                secondary_.onMessage(m, now_);
        }

        // Commit ONLY if pending cleared legitimately
        if (!primary_.isWritePending() && pending_start_ms_ >= 0) {
            std::cout << "Write COMMITTED\n";
            pending_start_ms_ = -1;
            committed = true;
            break;
        }

        // Correctness fix: timeout if secondary is DOWN
        if (primary_.isWritePending() &&
            network_.isPartitioned() &&
            (now_ - pending_start_ms_ >= WRITE_TIMEOUT_MS)) {

            std::cout << "Write FAILED (secondary unavailable, timeout)\n";
            primary_.clearPending();
            pending_start_ms_ = -1;
            return;
        }
    }

    if (!committed && primary_.isWritePending()) {
        std::cout << "(write still pending)\n";
    }
}

void InteractiveConsole::cmdSecondaryState(const std::string& state) {
    if (state == "down") {
        network_.setPartition(true);
        std::cout << "Secondary is now DOWN\n";
    } else if (state == "up") {
        network_.setPartition(false);

        NetMsg r;
        r.type = MsgType::RESYNC_REQ;
        r.from_site = 1;
        r.to_site = 0;
        r.last_applied = secondary_.lastApplied();
        network_.send(r, now_);

        std::cout << "Secondary recovered, requesting resync\n";
    } else {
        std::cout << "Invalid state (use up/down)\n";
    }
}

void InteractiveConsole::cmdStatus() {
    std::cout << "Primary KV: " << primary_.store().size() << " entries\n";
    std::cout << "Secondary KV: " << secondary_.store().size() << " entries\n";

    if (primary_.isWritePending())
        std::cout << "Pending write LSN=" << primary_.pendingLSN() << "\n";
    else
        std::cout << "No pending writes\n";

    auto res = ConsistencyChecker::check(primary_.store(), secondary_.store());
    std::cout << "Consistency: "
              << (res.consistent ? "CONSISTENT" : "DIVERGED") << "\n";
}