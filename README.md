# Synchronous Replication and Consistency Simulator

## Overview

This project implements a **synchronous replication simulator** that models how data is replicated between a **Primary** site and a **Secondary** site under normal operation, network delays, failures, and recovery scenarios.

The system supports both:
- an automated simulation mode
- an interactive mode for manual experimentation

---

## Key Concepts Modeled

- Strict synchronous replication semantics
- Write-Ahead Logging (WAL)
- Network delay and partition simulation
- Blocking writes during secondary failure
- Timeout-based failure handling
- Recovery and resynchronization
- Consistency verification between replicas

---

## System Model

- A **Primary** site accepts client writes
- A **Secondary** site synchronously replicates writes
- Writes are committed only after the Secondary acknowledges them
- If the Secondary is unavailable, writes block and may fail after a timeout
- On recovery, the Secondary resynchronizes using the Primary’s WAL

---

## Project Structure

The simulator is organized into modular components:

- **KVStore** – in-memory key-value storage
- **Write-Ahead Log (WAL)** – durable operation logging
- **Site** – abstraction for Primary and Secondary behavior
- **Network Simulator** – simulates message delivery, delay, and partitions
- **Interactive Console** – manual control and inspection
- **Consistency Checker** – verifies replica correctness

---

## Build Environment

- Operating System: macOS
- Compiler: `clang++` with C++17 support
- Development Environment: VS Code or any C++ IDE

The project builds into a single executable named `syncsim`.

---

## Building the Project

Compile from the project root directory:

```bash
clang++ -std=c++17 src/*.cpp -o syncsim
