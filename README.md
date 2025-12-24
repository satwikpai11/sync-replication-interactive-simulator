SYNCHRONOUS REPLICATION AND CONSISTENCY SIMULATOR

Overview
This project implements a synchronous replication simulator that models how data is replicated between a Primary site and a Secondary site under normal operation, network delays, failures, and recovery scenarios. The system is inspired by real-world Business Continuity and Disaster Recovery (BCDR) and High Availability (HA) storage systems, focusing on correctness and consistency rather than raw performance.

The simulator supports both automated simulations and an interactive mode that allows a user to manually issue writes, inject failures, and observe recovery behavior in a controlled and intuitive manner.

⸻

Key Concepts Modeled
• Strict synchronous replication semantics
• Write-Ahead Logging (WAL)
• Network delay and partition simulation
• Blocking writes during secondary failure
• Timeout-based failure handling
• Recovery and resynchronization
• Consistency verification between replicas

⸻

Project Structure
The project is organized into modular components that separate storage, replication logic, network simulation, and user interaction.

Main components include:
• Key-Value Store (KVStore)
• Write-Ahead Log (WAL)
• Site abstraction representing Primary and Secondary
• Network simulator for message delivery and partitions
• Interactive console for manual experimentation
• Consistency checker for replica verification

⸻

Build and Run Environment
• Operating System: macOS
• Compiler: clang++ with C++17 support
• Development Environment: VS Code or any C++ IDE

The project is built as a single executable named “syncsim”.

⸻

Running the Simulator

Automated Simulation Mode
In this mode, the simulator runs a predefined workload that injects failures and measures system behavior. It prints a summary to the console and generates structured logs for analysis.

Run the simulator normally to start this mode.

⸻

Interactive Mode
Interactive mode allows the user to manually control the system and observe synchronous replication behavior in real time.

Run the simulator with the interactive flag to start this mode.

Once started, the user can issue commands to write data, simulate failures, advance time, and inspect system state.

⸻

Interactive Commands

Write Command
Issues a synchronous write to the Primary site. The write commits only after the Secondary site acknowledges it.

Format:
write  

Example:
write x 100

⸻

Tick Command
Advances the simulated clock by the specified number of milliseconds. Network messages are delivered during ticks.

Format:
tick 

Example:
tick 10

⸻

Secondary Failure Simulation

To simulate the Secondary site going down:
secondary down

To bring the Secondary site back up and trigger resynchronization:
secondary up

⸻

Status Command
Displays the current system state including:
• Number of entries in Primary and Secondary
• Whether a write is pending
• Consistency status between replicas

Format:
status

⸻

Exit Command
Exits the interactive simulator.

Format:
exit

⸻

WORKING (How the System Operates)

Normal Operation
When a client issues a write, the Primary site first appends the operation to its Write-Ahead Log. The write is then sent to the Secondary site. The Secondary applies the write and sends an acknowledgment back to the Primary. Only after receiving this acknowledgment does the Primary commit the write. This guarantees strong consistency between the two sites.

⸻

Behavior During Secondary Failure
If the Secondary site is unavailable, acknowledgments cannot be received. As a result, synchronous writes remain blocked. If the block exceeds a defined timeout threshold, the write fails. This behavior models the real-world tradeoff between availability and consistency in synchronous replication systems.

⸻

Recovery and Resynchronization
When the Secondary site recovers, it requests any missing log entries from the Primary. The Primary resends the required Write-Ahead Log entries, allowing the Secondary to catch up. Once resynchronization completes, normal synchronous operation resumes and new writes can commit successfully.

⸻

Consistency Verification
The simulator verifies consistency by comparing the internal state of the Primary and Secondary key-value stores. Temporary divergence may occur during in-flight writes, but committed states are guaranteed to converge after acknowledgments or resynchronization.

⸻

Importance
This project demonstrates core distributed systems concepts used in high-availability storage platforms, including synchronous replication, failure handling, recovery workflows, and consistency guarantees. It is designed to closely mirror the reasoning and behavior of real-world BCDR and HA systems.
