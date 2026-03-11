# Ability System Plugin - Test Documentation

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

- **`AbilitySystemTagSpec`**: Hierarchy matching, exact matching, and registration.
- **`AbilitySystemAttributeSet`**: Base value clamping and initialization.
- **`AbilitySystemEffectSpec`**: Mathematical calculations for attribute modifications.

### 2. Integration Tests

Simulates real Godot scenarios using the `AbilitySystemComponent`:

- **Effect Application**: Tests duration, stacking logic (Override, Intensity) and removal.
- **Ability Execution**: Validates activation requirements, costs, and cooldowns.
- **Signal Integrity**: Ensures components correctly broadcast events when tags or attributes change.

### 3. Project Tests (Top-Down RPG Scenario)

Located in `src/tests/test_ability_system_integration.h`, this suite simulates a complete Top-Down RPG ecosystem to verify high-level interactions and system-wide "business rules".

**Scenario Overview:**
- **Actors**: Player and Enemy/NPC, both equipped with an `AbilitySystemComponent`.
- **Attributes**: `Health`, `Mana`, `Stamina`.
- **States (Tags)**: `State.Talking`, `State.Stunned`, `State.Dead`, `State.Burning`.

**Verified Interaction Flows:**

- **Dialogue vs. Combat**:
  - Activating `Ability.Talk` grants the `State.Talking` tag.
  - While talking, combat abilities (e.g., `Ability.Melee`) are blocked via `activation_blocked_tags`, ensuring the player can't attack during a conversation.
- **Combat & DOT (Damage Over Time)**:
  - `Ability.Fireball` checks for `Mana` cost and applies `Effect.Burning`.
  - `Effect.Burning` is a periodic effect that reduces `Health` every second, verifying the system's ability to handle time-based attribute decay.
- **Crowd Control (CC)**:
  - Application of `State.Stunned` cancels current abilities and blocks new activations.
- **Progression & Leveling**:
  - Abilities verify `Attribute` requirements (e.g., needing 150 Mana to cast a high-level spell) without necessarily consuming the attribute.
- **Death & Lifecycle**:
  - When `Health` reaches zero, `State.Dead` is applied.
  - Verified that all active abilities/effects are purged and further actions are restricted.

---

## 🚀 Execution

### Local (GDExtension)

To run the full suite using your local Godot binary:

```powershell
python -m SCons tests=yes
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
