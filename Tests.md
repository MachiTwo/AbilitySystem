# 🧪 Testing Reference (v0.1.0 Stable)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](Tests.md) | [**Português**](Tests.pt.md)

---

## 🏗️ Methodology: Engineering Rigor (TDD)

This project rejects "Vibe-Coding". Every business rule is backed by a **doctest** suite. We follow the **Red-Green-Refactor** cycle:

1. **RED**: Write a failing test defining the requirement.
2. **GREEN**: Implement minimal code to pass.
3. **REFACTOR**: Optimize while maintaining pass status.

---

## 🧩 Test Suites Overview

Our tests are divided into specialized headers in `src/tests/` to ensure modularity and high coverage.

### 1. Atomic Core Tests (Unit)

Verify individual class logic without side effects.

- **`ASTagSpec`**: Reference counting, hierarchical matching (`State.Dead.Bleeding` matches `State.Dead`), and exact matching.
- **`ASAttributeSet`**: Initialization, value clamping (e.g., Health not exceeding MaxHealth), and manual base value updates.
- **`ASEffect` / `ASEffectSpec`**: Mathematical operations (`ADD`, `MULTIPLY`, `OVERRIDE`) and complex stacking policies (`INTENSITY`, `DURATION`).
- **`ASAbility` / `ASAbilitySpec`**: Individual activation checks, cost/requirement satisfaction, and state transitions.

### 2. Advanced System Tests

- **`ASDelivery`**: Payload injection into targets. Verifies source-relative attribute calculations (e.g., damage based on Attacker's Strength).
- **`ASPackage`**: Validates the bundling of multiple Effects and Cues into a single portable resource.
- **`Ability Triggers`**: Automatic activation logic when tags are added or removed from a component.
- **`AS Cues`**: Execution of visual (Animation) and audio (AudioStream) feedback at specific lifecycle points.

### 3. Integration & Scenarios

Simulate real-world gameplay complexity within the `ASComponent`.

- **`Advanced Logic`**: Periodic ticks for **Damage over Time (DoT)** and **Heal over Time (HoT)**, including first-tick execution policies.
- **`Integration RPG Flow`**: High-density scenarios including:
  - **Dialogue vs. Combat**: Blocking abilities based on social states.
  - **Resource Exhaustion**: Failing actions when Mana/Stamina is insufficient.
  - **Parallel Actions**: Running multiple abilities (e.g., Walking + Jumping) simultaneously.
  - **Death Resolution**: Total action block and resource cleanup upon character death.
- **`Mega Integration`**: A full "End-to-End" unit test covering Ability -> Effect -> Cue in a single sequence.

---

## 🚀 Execution Guide

### Local Testing

The suite runs via Godot's headless mode. Execute via SCons:

```powershell
# Run all unit tests
python -m SCons target=editor tests=unit -j4

# Run integration scenarios
python -m SCons target=editor tests=playtest -j4

# Run everything
python -m SCons target=editor tests=all -j4
```

### CI/CD Environment

GitHub Actions executes the full suite on every PR for Windows, Linux, and macOS. A non-zero exit code in any test will block the merge.

---

## 🛠️ Testing Tools

- **`test_helpers.h`**: Macros like `CHECK_ATTR_EQ` and `make_standard_asc` to reduce boilerplate.
- **`test_signal_watcher.h`**: Utility to verify that GDExtension signals are being correctly emitted.

> [!IMPORTANT]
> To maintain the **Desapego Radical ao Código** principle, logic changes MUST be preceded by a PR that updates these test targets.
