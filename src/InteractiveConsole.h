#pragma once

#include <string>
#include "Site.h"
#include "Network.h"
#include "ConsistencyChecker.h"

/* =========================
   Interactive Console
   ========================= */

class InteractiveConsole {
public:
    InteractiveConsole(Site& primary,
                       Site& secondary,
                       Network& network);

    void run();

private:
    Site& primary_;
    Site& secondary_;
    Network& network_;

    TimeMs now_ = 0;

    // Track pending write start time (for timeout logic)
    TimeMs pending_start_ms_ = -1;

    void handleCommand(const std::string& line);
    void cmdWrite(const std::string& key, const std::string& value);
    void cmdTick(TimeMs delta);
    void cmdStatus();
    void cmdSecondaryState(const std::string& state);
};