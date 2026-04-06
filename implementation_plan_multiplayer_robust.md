# Implementation Plan - Ambitious Multiplayer Testing Infrastructure

This plan establishes a comprehensive strategy for professional-level multiplayer testing. It goes beyond simple connectivity, focusing on synchronization integrity, prediction correctness, and stress testing via AI-driven actors and simulated network chaos.

## User Review Required

> [!IMPORTANT]
> **Dynamic Spawning**: The demo project will be refactored to remove static players. This requires a new `MultiplayerController` autoload in GDScript.
> **CAUTION on Network Simulation**: Simulating true packet loss/latency in GHA runners requires careful Python-level delays or `tc` (Linux Traffic Control) commands. We will prioritize Python-level orchestration for portability across OSes.

## Proposed Changes

### Core C++ Testing (Low-Level Sync)

#### [NEW] [test_as_multiplayer_sync.h](file:///c:/Users/bruno/Desktop/AbilitySystem/src/tests/bridge/test_as_multiplayer_sync.h)

A new bridge test header focusing on synchronization logic:

- **Mock Networking**: Simulate a Command/Correction loop between two `ASComponent` instances.
- **Rollback Validation**: Force a mismatch, trigger `rollback_to_tick()`, and verify that `AttributeSet` and `Tags` are restored to the exact millisecond precision.
- **Checksum Stability**: Implement a hash-based state verification to detect subtle synchronization drifts.

---

### Demo Refactor (Multiplayer & AI)

#### [MODIFY] [main.tscn](file:///c:/Users/bruno/Desktop/AbilitySystem/demo/scenes/main.tscn)

- Remove the static Player node.
- Add `MultiplayerSpawner` pointing to a new `players` container.
- Update UI to include a "Connect/Host" debug overlay (automatically bypassed by CI).

#### [MODIFY] [player.tscn](file:///c:/Users/bruno/Desktop/AbilitySystem/demo/player/player.tscn)

- Add `MultiplayerSynchronizer` to sync `position`, `velocity`, and `ASComponent._snapshot_state`.
- Ensure the `ASComponent` handles authority correctly via `_is_local_authority()`.

#### [NEW] [player_ai.bt](file:///c:/Users/bruno/Desktop/AbilitySystem/demo/ai/player_ai.bt)

A behavior tree driven by LimboAI:

- **Wander**: Move between platforms.
- **Action Cycle**: Randomly trigger Jump, Dash, and Hyper-Dash abilities.
- **Blackboard**: Store `target_platform`, `has_stamina`, and `is_grounded`.

---

### Python Orchestration (The "Chaos Engine")

#### [MODIFY] [runner.py](file:///c:/Users/bruno/Desktop/AbilitySystem/utility/multiplayer/runner.py)

Enhance the runner with "Ambition" features:

- **Latency Simulation**: Intercept and delay communication or simulate delayed launch of clients.
- **Chaos Monkey**: Randomly kill a client process (`SIGKILL`) and restart it to test peer-reconnection and state catch-up.
- **Verification Loop**: Periodically query the Godot instance for a "State Hash" via stdout to detect desyncs in real-time.

#### [MODIFY] [log_analyzer.py](file:///c:/Users/bruno/Desktop/AbilitySystem/utility/multiplayer/log_analyzer.py)

- **Deep Comparison**: Compare State Hashes across all 10 players.
- **Desync Visualizer**: Generate a Mermaid diagram showing exactly at which tick a player diverged from the server.

---

### CI/CD Pipeline Expansion

#### [MODIFY] [test_multiplayer.yml](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_multiplayer.yml)

- **Build Step**: Add a job to compile the `Godot Demo` (Release template).
- **Stress Node**: Add a specific "Server" matrix node that remains active while clients cycle.
- **Network Profiles**: Matrix-test with `Normal`, `High Latency`, and `Chaos (Disconnects)` profiles.

## Open Questions

> [!CAUTION]
> **Latency Implementation**: Should we use `tc-netem` (Linux Specific) or internal Godot network delay simulation (`multiplayer.simulated_latency`)?
>
> - **Recommendation**: Use Godot's internal simulation if available, fallback to `tc` on GHA Linux runners for 100% realistic packet drops.

## Verification Plan

### Manual Verification

1. Run a local server and 2 AI-driven clients. Observe the "Wander" behavior and ensure no "Rubber-banding" occurs during dashes.
2. Manually kill one client and reconnect: verify it inherits the correct attributes (HP/Level/Tags) from the server.

### Automated Tests

1. **Sync Test**: `python -m SCons tests=bridge` (validating the new C++ sync header).
2. **Stress Test**: Complete GHA run verifying that the `Log Analyzer` reports 0 State Drifts across 10 players under chaos.
