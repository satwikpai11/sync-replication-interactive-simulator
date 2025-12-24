#include "Network.h"
#include <algorithm>

/* =========================
   Network Implementation
   ========================= */

Network::Network(TimeMs base_delay_ms, TimeMs jitter_ms)
    : base_delay_ms_(base_delay_ms), jitter_ms_(jitter_ms) {}

void Network::setPartition(bool on) {
    partitioned_ = on;
}

bool Network::isPartitioned() const {
    return partitioned_;
}

TimeMs Network::pickDelay() {
    // Tiny deterministic PRNG for jitter
    prng_ = (prng_ * 1103515245 + 12345) & 0x7fffffff;
    TimeMs j = (jitter_ms_ <= 0) ? 0 : (prng_ % (jitter_ms_ + 1));
    return base_delay_ms_ + j;
}

void Network::send(const NetMsg& msg, TimeMs now_ms) {
    // During partition, messages are dropped (simulates disconnected sites)
    if (partitioned_) return;

    InFlightMsg f;
    f.deliver_ms = now_ms + pickDelay();
    f.msg = msg;
    queue_.push_back(std::move(f));
}

void Network::deliverReady(TimeMs now_ms, std::vector<NetMsg>& out) {
    out.clear();
    if (queue_.empty()) return;

    // Pull all messages with deliver_ms <= now_ms
    std::vector<InFlightMsg> remaining;
    remaining.reserve(queue_.size());

    for (auto& f : queue_) {
        if (f.deliver_ms <= now_ms) out.push_back(f.msg);
        else remaining.push_back(std::move(f));
    }

    queue_.swap(remaining);
}