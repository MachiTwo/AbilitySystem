# Ability System - Test Documentation

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](Tests.md) | [**Português**](Tests.pt.md)

## 🧪 Methodology: Test-Driven Development (TDD)

This project follows a strict **Engineering Rigor** approach. Business logic is never implemented without a corresponding test case that justifies its existence.

### Red-Green-Refactor Cycle

1. **RED**: Define the requirement through a test. The build must fail or the test must not pass.
2. **GREEN**: Implement the minimum amount of code required to satisfy the test. Avoid over-engineering.
3. **REFACTOR**: Optimize the code for performance and readability while ensuring all tests remain passing.

---

## 🏗️ Test Architecture

The testing system is designed to work in both **GDExtension** (via `doctest`) and **Native Module** (via Godot's internal test runner) environments.

### 1. Core Logic (Unit Tests)

Located in `src/tests/`, these verify atomic behaviors of the system:

- **`ASTagSpec`**: Hierarchy matching, exact matching, and registration.
- **`ASAttributeSet`**: Base value clamping and initialization.
- **`ASEffectSpec`**: Mathematical calculations for attribute modifications.
- **`ASDelivery`**: Reactive effect delivery system for targets (ideal for projectiles).
- **`Ability Triggers`**: Automatic ability activation based on Tag events (Added/Removed).

### 2. Integration Tests

Simulates real Godot scenarios using the `ASComponent`:

- **Effect Application**: Tests duration, stacking logic (Override, Intensity) and removal.
- **Ability Execution**: Validates activation requirements, costs, and cooldowns.
- **Signal Integrity**: Ensures components correctly broadcast events when tags or attributes change.

### 3. Project Tests (Top-Down RPG Scenario & LimboAI)

Located in `src/tests/test_as_integration.h`, this suite simulates a complete Top-Down RPG ecosystem to verify high-level interactions and system-wide "business rules".

**Scenario Overview:**

- **Actors**: Player and Enemy/NPC (Charger), both equipped with an `ASComponent`.
- **AI**: The Enemy uses LimboAI integrated via `BTAction` to trigger abilities on the component.
- **Verified Interaction Flows:**
  - **Dialogue vs. Combat**: Blocking social abilities by combat states.
  - **Crowd Control (CC)**: Stun canceling active abilities.
  - **Death & Lifecycle**: Transition to `State.Dead` clearing all resources.

---

## 🚀 Execution

### Local (GDExtension)

To run the full suite using your local Godot binary:

```powershell
python -m SCons target=editor platform=windows tests=playtest -j4
```

This will compile the plugin with test symbols and invoke `utility/tests.py`, which launches Godot headlessly to run the `doctest` suite.

### CI/CD (GitHub Actions)

Tests are automatically executed on every Push or Pull Request for:

- **Linux (x64 & ARM64)**
- **Windows (x64)**
- **macOS (Universal)**

---

## 📊 Coverage Goals

We aim for **100% Coverage of Core Logic**. Editor-specific UI code is excluded from automated unit tests and relies on empirical validation via the `demo/` project.
