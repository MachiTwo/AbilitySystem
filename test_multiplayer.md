# Multiplayer Testing Framework

This document describes the hybrid C++/Python testing infrastructure used to validate the Ability System's multiplayer synchronization and authority logic.

## 🏗️ Architecture

The framework is designed to simulate a real-world multiplayer environment without needing a full Godot scene tree for every test.
P

- **Orchestrator (`utility/multiplayer/runner.py`)**: The entry point. It spawns the mock server and multiple client instances.
- **State Mock Server (`mock_server.py`)**: A lightweight authoritative server that tracks global state (Attribute Sets, Tags) and validates client attempts to modify them.
- **Client Instances (Godot Headless)**: Parallel processes running `doctest` unit tests. Each instance simulates a specific entity archetype (Warrior, Mage, Boss, etc.).
- **Net Simulator (`net_simulator.py`)**: Injects latency and packet loss into the local communication to test the system's robustness.

## 🚀 Execution Workflow

1. **Setup**: The runner cleans up previous zombie processes.
2. **Launch Server**: The Python mock server starts and binds to the configured port.
3. **Spawn Clients**: Parallel Godot instances are launched with specific test filters (e.g., `--test-case=*Player_1*`).
4. **Execution**: Clients perform synchronized actions (e.g., Applying a debuff on Player 2, checking if the server replicated the tag).
5. **Collection**: Logs are saved to `doctest_mp_player_N.log`.
6. **Analysis**: The `log_analyzer.py` verifies if all 10 players completed their 300% coverage scenarios.

## 🧪 Scenarios Included

- **Authority Rejection**: Client trying to change health without server permission.
- **Tag Propagation**: Burning tag applied on a target must replicate to all observers.
- **Attribute Drivers in Sync**: Strength buff on Server must correctly update Attack on Clients.
- **Late Joiner Logic**: New client joining mid-combat must receive the current state of all `ASComponents`.
