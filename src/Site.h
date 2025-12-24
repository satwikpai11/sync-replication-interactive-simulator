#pragma once

#include <string>
#include "types.h"
#include "KVStore.h"
#include "WAL.h"
#include "Network.h"

/* =========================
   Site (Primary / Secondary)
   ========================= */

class Site {
public:
    Site(int site_id, Network& net);

    int id() const;

    LSN startWrite(const std::string& key,
                   const std::string& value,
                   TimeMs now_ms);

    void onMessage(const NetMsg& msg, TimeMs now_ms);

    LSN lastApplied() const;

    const KVStore& store() const;
    KVStore& store();

    bool isWritePending() const;
    LSN pendingLSN() const;
    void markPendingStart(TimeMs t);
    TimeMs pendingStartTime() const;
    void clearPending();

    const WAL& wal() const;
    WAL& wal();

private:
    int site_id_;
    Network& net_;

    KVStore kv_;
    WAL wal_;

    LSN last_applied_ = 0;

    bool pending_ = false;
    LSN pending_lsn_ = 0;
    TimeMs pending_start_ms_ = 0;

    void sendAck(LSN lsn, TimeMs now_ms, int to_site);
    void requestResync(TimeMs now_ms, int to_site);
};