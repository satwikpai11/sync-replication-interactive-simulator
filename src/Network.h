#pragma once
#include <vector>
#include "types.h"

/* =========================
   Network Simulator
   ========================= */

class Network {
public:
    Network(TimeMs base_delay_ms, TimeMs jitter_ms);

    void setPartition(bool on);
    bool isPartitioned() const;

    void send(const NetMsg& msg, TimeMs now_ms);
    void deliverReady(TimeMs now_ms, std::vector<NetMsg>& out);

private:
    TimeMs base_delay_ms_;
    TimeMs jitter_ms_;
    bool partitioned_ = false;

    std::vector<InFlightMsg> queue_;
    int64_t prng_ = 1337;

    TimeMs pickDelay();
};