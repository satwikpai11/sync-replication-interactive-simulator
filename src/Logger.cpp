#include "Logger.h"

/* =========================
   Logger Implementation
   ========================= */

Logger::Logger(const std::string& filename) : out_(filename) {}
Logger::~Logger() { if (out_.is_open()) out_.close(); }

bool Logger::isOpen() const { return out_.is_open(); }

std::string Logger::escape(const std::string& s) {
    std::string o;
    o.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '\\') o += "\\\\";
        else if (c == '"') o += "\\\"";
        else if (c == '\n') o += "\\n";
        else o += c;
    }
    return o;
}

void Logger::logWriteEvent(TimeMs now_ms,
                           LSN lsn,
                           const std::string& key,
                           Outcome outcome,
                           TimeMs start_ms,
                           TimeMs end_ms,
                           const std::string& note) {
    if (!out_.is_open()) return;

    out_ << "{"
         << "\"ts_ms\":" << now_ms << ","
         << "\"event\":\"write_result\","
         << "\"lsn\":" << lsn << ","
         << "\"key\":\"" << escape(key) << "\","
         << "\"outcome\":\"" << toString(outcome) << "\","
         << "\"start_ms\":" << start_ms << ","
         << "\"end_ms\":" << end_ms << ","
         << "\"latency_ms\":" << (end_ms >= start_ms ? (end_ms - start_ms) : -1) << ","
         << "\"note\":\"" << escape(note) << "\""
         << "}\n";
}

void Logger::logInfo(TimeMs now_ms, const std::string& type, const std::string& message) {
    if (!out_.is_open()) return;

    out_ << "{"
         << "\"ts_ms\":" << now_ms << ","
         << "\"event\":\"info\","
         << "\"type\":\"" << escape(type) << "\","
         << "\"message\":\"" << escape(message) << "\""
         << "}\n";
}