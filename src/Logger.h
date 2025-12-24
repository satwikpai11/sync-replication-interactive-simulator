#pragma once
#include <fstream>
#include <string>
#include "types.h"

/* =========================
   JSONL Logger
   ========================= */

class Logger {
public:
    explicit Logger(const std::string& filename);
    ~Logger();

    bool isOpen() const;

    void logWriteEvent(TimeMs now_ms,
                       LSN lsn,
                       const std::string& key,
                       Outcome outcome,
                       TimeMs start_ms,
                       TimeMs end_ms,
                       const std::string& note);

    void logInfo(TimeMs now_ms, const std::string& type, const std::string& message);

private:
    std::ofstream out_;
    static std::string escape(const std::string& s);
};