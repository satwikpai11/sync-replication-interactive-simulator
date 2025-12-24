#pragma once
#include <string>
#include <vector>
#include "types.h"
#include "Network.h"
#include "Site.h"
#include "Logger.h"

/* =========================
   Simulator Orchestrator
   ========================= */

class Simulator {
public:
    struct Config {
        int num_writes = 200;
        int num_keys = 20;

        TimeMs sim_end_ms = 4000;
        TimeMs tick_ms = 1;

        TimeMs write_interval_ms = 10;    // issue a new write when possible
        TimeMs write_timeout_ms = 500;    // fail if no commit by this time

        TimeMs net_delay_ms = 5;
        TimeMs net_jitter_ms = 3;

        // Partition window
        TimeMs partition_start_ms = 800;
        TimeMs partition_end_ms   = 1300;
    };

    explicit Simulator(const Config& cfg);

    void run();

private:
    Config cfg_;
    TimeMs now_ = 0;

    Network net_;
    Site primary_;
    Site secondary_;
    Logger logger_;

    // Workload state
    int writes_issued_ = 0;
    TimeMs next_write_due_ = 0;

    // Current write tracking (strict-sync: one in-flight write at a time)
    bool have_inflight_ = false;
    LSN inflight_lsn_ = 0;
    std::string inflight_key_;
    TimeMs inflight_start_ms_ = 0;

    // Metrics
    int committed_ = 0;
    int failed_ = 0;
    TimeMs total_commit_latency_ = 0;

    // Helpers
    void maybeTogglePartition();
    void maybeIssueWrite();
    void processNetwork();
    void checkTimeout();
    std::string pickKey(int i) const;
    std::string makeValue(int i) const;

    void onCommit(LSN lsn);
    void onFail(const std::string& reason);
};