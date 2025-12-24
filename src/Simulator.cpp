#include "Simulator.h"
#include "ConsistencyChecker.h"
#include <iostream>

/* =========================
   Simulator Implementation
   ========================= */

Simulator::Simulator(const Config& cfg)
    : cfg_(cfg),
      net_(cfg.net_delay_ms, cfg.net_jitter_ms),
      primary_(0, net_),
      secondary_(1, net_),
      logger_("replication_log.jsonl") {

    if (!logger_.isOpen()) {
        std::cerr << "ERROR: could not open replication_log.jsonl for writing\n";
    }
}

std::string Simulator::pickKey(int i) const {
    int k = i % cfg_.num_keys;
    return "key_" + std::to_string(k);
}

std::string Simulator::makeValue(int i) const {
    return "value_" + std::to_string(i);
}

void Simulator::maybeTogglePartition() {
    if (now_ == cfg_.partition_start_ms) {
        net_.setPartition(true);
        logger_.logInfo(now_, "network", "Partition ENABLED (Primary <-> Secondary disconnected).");
    }
    if (now_ == cfg_.partition_end_ms) {
        net_.setPartition(false);
        logger_.logInfo(now_, "network", "Partition DISABLED (Primary <-> Secondary reconnected).");

        // On reconnect, Secondary asks Primary for missing entries
        NetMsg r;
        r.type = MsgType::RESYNC_REQ;
        r.from_site = 1;
        r.to_site = 0;
        r.last_applied = secondary_.lastApplied();
        net_.send(r, now_);

        logger_.logInfo(now_, "resync", "Secondary requested resync from its last_applied_lsn.");
    }
}

void Simulator::maybeIssueWrite() {
    if (writes_issued_ >= cfg_.num_writes) return;
    if (now_ < next_write_due_) return;
    if (have_inflight_) return; // strict-sync: only one in-flight write at a time

    std::string key = pickKey(writes_issued_);
    std::string value = makeValue(writes_issued_);

    LSN lsn = primary_.startWrite(key, value, now_);

    have_inflight_ = true;
    inflight_lsn_ = lsn;
    inflight_key_ = key;
    inflight_start_ms_ = now_;

    logger_.logInfo(now_, "write", "Issued write lsn=" + std::to_string(lsn) + " key=" + key);

    writes_issued_++;
    next_write_due_ = now_ + cfg_.write_interval_ms;
}

void Simulator::processNetwork() {
    std::vector<NetMsg> ready;
    net_.deliverReady(now_, ready);

    for (const auto& m : ready) {
        if (m.to_site == 0) primary_.onMessage(m, now_);
        else secondary_.onMessage(m, now_);

        // Commit detection: primary clears pending when ACK matches
        if (m.type == MsgType::ACK && m.to_site == 0) {
            if (have_inflight_ && m.lsn == inflight_lsn_ && !primary_.isWritePending()) {
                onCommit(m.lsn);
            }
        }
    }
}

void Simulator::checkTimeout() {
    if (!have_inflight_) return;
    if ((now_ - inflight_start_ms_) >= cfg_.write_timeout_ms) {
        onFail("Write timed out waiting for Secondary ACK (network partition or high delay).");
        // Note: in strict systems you'd block; in simulator we fail to keep progress.
        primary_.clearPending();
    }
}

void Simulator::onCommit(LSN lsn) {
    (void)lsn;
    have_inflight_ = false;

    TimeMs end_ms = now_;
    TimeMs latency = end_ms - inflight_start_ms_;

    committed_++;
    total_commit_latency_ += latency;

    logger_.logWriteEvent(now_, inflight_lsn_, inflight_key_, Outcome::COMMITTED,
                         inflight_start_ms_, end_ms, "Committed after Secondary ACK (strict sync).");
}

void Simulator::onFail(const std::string& reason) {
    failed_++;
    logger_.logWriteEvent(now_, inflight_lsn_, inflight_key_, Outcome::FAILED,
                         inflight_start_ms_, now_, reason);
    have_inflight_ = false;
}

void Simulator::run() {
    logger_.logInfo(now_, "start", "Starting synchronous replication simulator (strict sync).");

    for (now_ = 0; now_ <= cfg_.sim_end_ms; now_ += cfg_.tick_ms) {
        maybeTogglePartition();
        maybeIssueWrite();
        processNetwork();
        checkTimeout();
    }

    auto res = ConsistencyChecker::check(primary_.store(), secondary_.store());
    logger_.logInfo(now_, "consistency", res.summary);

    std::cout << "\nSimulation complete.\n";
    std::cout << "Writes issued: " << writes_issued_ << "\n";
    std::cout << "Committed: " << committed_ << "\n";
    std::cout << "Failed: " << failed_ << "\n";

    double avg = (committed_ > 0) ? (double)total_commit_latency_ / (double)committed_ : 0.0;
    std::cout << "Avg commit latency (ms): " << avg << "\n";

    std::cout << "Primary KV keys: " << primary_.store().size() << "\n";
    std::cout << "Secondary KV keys: " << secondary_.store().size() << "\n";
    std::cout << "Consistency: " << (res.consistent ? "CONSISTENT" : "DIVERGED") << "\n";
    std::cout << "Log file: replication_log.jsonl\n";
}