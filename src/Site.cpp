#include "Site.h"

/* =========================
   Site Implementation
   ========================= */

Site::Site(int site_id, Network& net)
    : site_id_(site_id), net_(net) {}

int Site::id() const { return site_id_; }

LSN Site::startWrite(const std::string& key, const std::string& value, TimeMs now_ms) {
    // Primary appends to WAL but DOES NOT apply to KV until commit (strict sync)
    LSN lsn = wal_.append(key, value, now_ms);

    NetMsg m;
    m.type = MsgType::REPL_WRITE;
    m.from_site = 0;
    m.to_site = 1;
    m.lsn = lsn;
    m.key = key;
    m.value = value;

    net_.send(m, now_ms);

    pending_ = true;
    pending_lsn_ = lsn;
    pending_start_ms_ = now_ms;

    return lsn;
}

void Site::sendAck(LSN lsn, TimeMs now_ms, int to_site) {
    NetMsg a;
    a.type = MsgType::ACK;
    a.from_site = site_id_;
    a.to_site = to_site;
    a.lsn = lsn;
    net_.send(a, now_ms);
}

void Site::requestResync(TimeMs now_ms, int to_site) {
    NetMsg r;
    r.type = MsgType::RESYNC_REQ;
    r.from_site = site_id_;
    r.to_site = to_site;
    r.last_applied = last_applied_;
    net_.send(r, now_ms);
}

void Site::onMessage(const NetMsg& msg, TimeMs now_ms) {
    (void)now_ms;

    if (site_id_ == 1) {
        // Secondary behavior
        if (msg.type == MsgType::REPL_WRITE) {
            // Apply only if next in order or newer (simple: apply if lsn > last_applied)
            // For realism, you could enforce exact ordering; this keeps it simple.
            WalEntry e;
            e.lsn = msg.lsn;
            e.key = msg.key;
            e.value = msg.value;
            e.created_ms = now_ms;

            // Secondary "stores" via applying entry
            kv_.apply(e);
            if (msg.lsn > last_applied_) last_applied_ = msg.lsn;

            // ACK back to primary
            sendAck(msg.lsn, now_ms, 0);
        } else if (msg.type == MsgType::RESYNC_REQ) {
            // Secondary ignores RESYNC_REQ (only primary responds)
        } else if (msg.type == MsgType::ACK) {
            // Secondary doesn't expect ACKs
        }
        return;
    }

    // Primary behavior
    if (site_id_ == 0) {
        if (msg.type == MsgType::ACK) {
            // Commit happens when we receive ACK for the pending LSN
            if (pending_ && msg.lsn == pending_lsn_) {
                WalEntry e;
                if (wal_.getEntry(msg.lsn, e)) {
                    kv_.apply(e);
                    if (e.lsn > last_applied_) last_applied_ = e.lsn;
                }
                clearPending();
            }
        } else if (msg.type == MsgType::RESYNC_REQ) {
            // Secondary requests missing WAL entries after reconnect
            LSN start = msg.last_applied + 1;
            LSN end = wal_.lastLSN();
            for (LSN l = start; l <= end; ++l) {
                WalEntry e;
                if (!wal_.getEntry(l, e)) break;

                NetMsg repl;
                repl.type = MsgType::REPL_WRITE;
                repl.from_site = 0;
                repl.to_site = 1;
                repl.lsn = e.lsn;
                repl.key = e.key;
                repl.value = e.value;

                net_.send(repl, now_ms);
            }
        } else if (msg.type == MsgType::REPL_WRITE) {
            // Primary doesn't receive REPL_WRITE in this simplified model
        }
    }
}

LSN Site::lastApplied() const { return last_applied_; }

const KVStore& Site::store() const { return kv_; }
KVStore& Site::store() { return kv_; }

bool Site::isWritePending() const { return pending_; }
LSN Site::pendingLSN() const { return pending_lsn_; }
void Site::markPendingStart(TimeMs t) { pending_start_ms_ = t; }
TimeMs Site::pendingStartTime() const { return pending_start_ms_; }

void Site::clearPending() {
    pending_ = false;
    pending_lsn_ = 0;
    pending_start_ms_ = 0;
}

const WAL& Site::wal() const { return wal_; }
WAL& Site::wal() { return wal_; }