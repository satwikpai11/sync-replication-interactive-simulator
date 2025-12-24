/*
// JUST SIMULATOR VERSION
#include "Simulator.h"


int main() {
    Simulator::Config cfg;

    // Reasonable defaults; tweak these to demonstrate behavior
    cfg.num_writes = 200;
    cfg.num_keys = 25;

    cfg.sim_end_ms = 4000;
    cfg.tick_ms = 1;

    cfg.write_interval_ms = 8;
    cfg.write_timeout_ms = 600;

    cfg.net_delay_ms = 6;
    cfg.net_jitter_ms = 4;

    cfg.partition_start_ms = 800;
    cfg.partition_end_ms = 1300;

    Simulator sim(cfg);
    sim.run();
    return 0;
}

*/

// THIS VERSION IS INTERACTIVE
#include "Simulator.h"
#include "InteractiveConsole.h"
#include <cstring>

/* =========================
   Entry Point
   ========================= */

int main(int argc, char* argv[]) {
    if (argc > 1 && std::strcmp(argv[1], "--interactive") == 0) {
        Network net(5, 3);
        Site primary(0, net);
        Site secondary(1, net);

        InteractiveConsole console(primary, secondary, net);
        console.run();
        return 0;
    }

    Simulator::Config cfg;
    Simulator sim(cfg);
    sim.run();
    return 0;
}