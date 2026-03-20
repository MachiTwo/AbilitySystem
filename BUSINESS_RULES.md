# BUSINESS RULES: ABILITY SYSTEM - GOVERNANCE CONTRACT

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](BUSINESS_RULES.md) | [**Português**](BUSINESS_RULES.pt.md)

This document establishes the mandatory architectural boundaries and business rules for the Ability System. Any implementation violating these constraints must be immediately refactored.

---

## 1. PHILOSOPHY AND RIGOROUS ENGINEERING

This project explicitly rejects **"Vibe-Coding"** (programming by intuition, luck, or blind acceptance of AI suggestions). Every line of business logic is treated as an industrial engineering commitment.

### 1.1 Pair Programming and Governance

- **Radical Code Decoupling:** If the code fails, the failure lies in the communication or the architecture. Corrections are made through dialogue and documentation adjustments, never via manual "patches" that break the pair's mental model.

- **SSOT (Single Source of Truth):** This file is the Iron Law. Before any complex change, the rule must be documented here.

- **Language:** Code, technical documentation, and logs MUST be in **English**. Informal communication between the pair may be in Portuguese.

### 1.2 TDD Protocol (Red-Green-Refactor)

No business logic exists without a justifying test.

1. **RED:** Write the failing test, defining the contract.
2. **GREEN:** Implement the minimum code required to pass.
3. **REFACTOR:** Optimize while maintaining the passing status.

---

## 2. IDENTIFIERS: TAGS (THE SYSTEM'S DNA)

Tags are not classes; they are **Powerful Hierarchical Identifiers** based on `StringName`.

### 2.1 Tag (Identifier)

- **Role:** Represent states, actions, or categories (e.g., `State.Dead`, `Ability.Fireball`).

- **Rule:** Tags must be treated as immutable and global. Comparison must support hierarchy (checking for `State` should match `State.Dead`).

- **Activation Logic:** The system supports 4 logical states in Blueprints (Ability/Effect/Cue):
  - `Required All` (AND): Success if all tags are present.
  - `Required Any` (OR): Success if at least one tag is present.
  - `Blocked Any` (OR): Failure if any tag is present.
  - `Blocked All` (AND): Failure only if all tags are present simultaneously.

### 2.2 Tag Type & Tag Group

- **Tag Type:** Defines the technical nature of the tag (e.g., `NAME`, `CONDITIONAL`). This determines how the backend processes it.

- **Tag Group:** Exclusive logical organization for the Editor. Groups related tags for easier searching and visual manipulation.

---

## 3. THE SINGLETON: ABILITY SYSTEM (PROJECT INTERFACE)

- **Role:** The **Global Configuration API** and the bridge to `ProjectSettings`.

- **Business Rules:**
  - Solely responsible for saving and loading the global tag list in the project configuration (`project.godot`).
  - Acts as a **Central Name Registry** to ensure duplicate resources do not conflict.

- **Boundary:** It must not store the state of any Actor. If information belongs to a specific character instance, it **must not** reside here.

---

## 4. TOOLING LAYER: EDITORS

The interface between the Human and the Resources.

### 4.1 ASEditorPlugin

- **Role:** **Bootloader**.

- **Rule:** Handles registration of types, icons, and initialization of other sub-editors. Forbidden from containing gameplay logic.

### 4.2 ASTagsPanel

- **Role:** Visual interface for the **Global Registry**.

- **Rule:** Exclusively manipulates the tag dictionary within the `AbilitySystem` Singleton.

### 4.3 ASInspectorPlugin (and Property Selectors)

- **Role:** Contextualization.

- **Rule:** Must provide "smart selectors" (tag dropdowns, attribute search) to facilitate the configuration of Resources and Components in the Inspector.

---

## 5. THE BLUEPRINTS: RESOURCES (THE "WHAT")

Located in `src/resources/`. These are the **Data Definitions**.

- **Resources (Blueprints):** Static objects (`.tres`) defining the "DNA" of an ability or effect. **Rule:** Must not be modified at runtime (except `ASStateSnapshot`). They are shared across hundreds of instances.

- **Specs (Runtime Instances):** Lightweight instances created from Resources that maintain dynamic state (cooldowns, stacks, duration).

- **Snapshot Exception:** `ASStateSnapshot` is typed as a `Resource` but populated at runtime. It breaks the immutability rule to allow native persistence (Save/Load) and network caching via Godot's filesystem.

### Usage and Performance Restrictions

> [!WARNING]
> **`ASStateSnapshot` is a heavy resource.** Due to the nature of full-state capture, it consumes significant CPU and memory if used at scale.

1. **Exclusive for Players:** Snapshot usage must be restricted to player-controlled entities (Playable Characters), where determinism and rollback are critical for multiplayer.
2. **NPCs and Enemies:** Non-playable entities **MUST NOT** use `ASStateSnapshot`. For NPC persistence, use lighter mechanisms like Zyris Engine's `SaveServer` or custom dictionary systems.
3. **Golden Rule:** If it's playable, use `ASStateSnapshot`. If not, discard it.
4. **Reference Independence:** Snapshots store primitive values and tag names, not object pointers, ensuring safe serialization.

- **Contents:** Base values, icons, names, requirement tags, and raw modifier lists.

### 5.1 ASAbility & ASEffect (Actions and Modifiers)

- **ASAbility - Role:** Defines the logic of an action (Costs, Cooldown, Triggers).

- **ASAbility - Rule:** The only Resource capable of managing activation requirements and attribute costs through specification.

- **ASEffect - Role:** State modifier (Buffs, Debuffs, Damage).

- **ASEffect - Rule:** Defines stacking policies and magnitude changes for attributes.

### 5.2 ASAttribute & ASAttributeSet (The Attribute System)

- **ASAttribute - Role:** Defines metadata (min/max limits) for a single statistic.

- **ASAttributeSet - Role:** Groups statistics and defines a character's initial state. Owns the logic for attribute modification.

- **ASAttributeSet - Rule (Attribute Drivers):** Allows deriving a base attribute's value from another (e.g., 2 \* Strength = 1 Attack). Recalculation is automatic upon base value changes.

- **ASAttributeSet - Rule (Priority):** Modifiers (Flat Add, Multiplier) are applied _after_ Driver calculations.

### 5.3 ASContainer & ASPackage (Archetypes and Payloads)

- **ASContainer - Role:** Full archetype (The Actor's Identity Dictionary).

- **ASContainer - Rule:** Acts as the "Factory Template" for total `ASComponent` initialization.

- **ASPackage - Role:** Transport gatherer (Data Envelope).

- **ASPackage - Rule:** Used exclusively to transmit collections of effects and cues via `ASDelivery`.

### 5.4 ASCue (Visual Feedback)

- **Role:** Pure audiovisual feedback (Animation, Sound, Particles).

- **Rule:** Forbidden from altering any gameplay data. Must be triggered reactively.

---

## 6. THE EXECUTORS: SPECS (THE "HOW")

Located in `src/core/`. Where runtime state and execution logic reside.

- **Role:** Represents the **Active Instance**. Owner of the **"Now"**.

- **Golden Rule: STATE SOVEREIGNTY.**

- **Contents (Logic that lives here, not in the Component):**
  - `duration_remaining`: Individual timer for each instance.
  - `stack_count`: Current count of a specific active effect.
  - `calculate_...`: Calculation logic dependent on variable attributes (e.g., damage scaled by current strength).

- **Responsibility:** The Spec must know if it has "finished." The Component merely queries it.

### 6.1 ASAbilitySpec & ASEffectSpec (Execution)

- **ASAbilitySpec - Role:** An ability currently in execution or equipped.

- **ASAbilitySpec - Rule:** Manages individual cooldowns and activation state.

- **ASEffectSpec - Role:** Active instance of a modifier.

- **ASEffectSpec - Rule:** Holds sovereignty over remaining `duration` and `stacks`.

### 6.2 ASCueSpec & ASTagSpec (Feedback and Identity)

- **ASCueSpec - Role:** Manages the lifecycle of scene-based feedback.

- **ASCueSpec - Rule:** Ensures cleanup (`queue_free`) of the instantiated Node after completion.

- **ASTagSpec - Role:** Reference counter (Refcount) for Tags.

- **ASTagSpec - Rule:** Ensures a Tag only leaves the actor when all its originating Specs expire.

---

## 7. THE ORCHESTRATOR: COMPONENT (THE HUB)

The `ASComponent` (ASC).

- **Role:** **Collection Manager** and **Signal Router**.

- **Business Rules:**
  - Must not manage individual effect timers (this is the Spec's job).
  - Responsible for maintaining the `active_specs` and `unlocked_specs` lists.
  - Acts as the **Attribute Owner** (via `AttributeSet`).
  - Sole entity authorized to add/remove tags from the Actor.

- **Ensuring Determinism:** The ASC must be capable of rewinding and advancing its state (Rollback/Prediction) via `ASStateSnapshot`.

- **State Caching:** Maintains an internal snapshot buffer for network synchronization.

- **Boundary:** The ASC must not know internal details of how an ability works. It only commands: `spec->_activate()`, `spec->tick(delta)`, `spec->_deactivate()`.

- **Node Registry:** The Component must maintain an alias registry (e.g., "Muzzle") so Cues know where to instance visual effects without hardcoded scene paths.

---

## 8. DELIVERY AND REACTIVITY SYSTEMS

### 8.1 ASDelivery (Payload Injections)

- **Role:** Decouples the emitter from the target in spatial interactions (Projectiles, AoEs).

- **Rule:** Transports an `ASPackage` and injects the content upon colliding with an ASC.

### 8.2 Ability Triggers (Reactive Automation)

- **Role:** Allows automatic ability activation based on state events (Tags).

- **Rule:** Activation based exclusively on `ON_TAG_ADDED` or `ON_TAG_REMOVED`.

---

## 9. REPLICATION AND PERSISTENCE (DETERMINISMO)

The Ability System is designed for authoritative multiplayer with Prediction and Rollback support. An Actor's state at any given moment (Tick) is managed by two synchronized mechanisms:

- **Source of Truth (Physics Only):** The `tick` is the only valid temporal identifier. `ASComponent` operates **exclusivamente** via `physics_process`. Use of `_process` (Idle/Frame) is strictly forbidden for gameplay logic to ensure determinism and Rollback compatibility.

### 9.1 ASStateSnapshot (The Heavy Resource)

- **Role:** Long-term persistence (Save/Load) and external "State Differential" synchronization.

- **Nature:** A **Godot Resource** (`.tres`). Heap-allocated, supports native serialization.

- **Usage Rule:** Reserved for **Players** (Playable Characters) or states that must survive scene reinitialization.

- **SSOT:** The only resource authorized to be mutable at runtime for full-state capture.

### 9.2 ASStateCache (The Lightweight Structure)

- **Role:** Short-term memory for Prediction, Reconciliation, and NPCs.

- **Nature:** **Pure C++ struct**. Allocated on the stack or inline within a circular buffer (`Vector`).

- **Usage Rule:** Used to maintain recent tick history (e.g., last 64-128 ticks) for network calculations.

- **Advantage:** Zero Resource allocation overhead. Ideal for rapid synchronization of non-playable entities (NPCs/Enemies).

### 9.3 Network Activation and Determinism Flow

1. **Request:** The client requests activation by calling `request_activate_ability(tag)`.
2. **Predict:** The client locally executes the action (zero latency) and generates an entry in the `cache_buffer` via `capture_snapshot()`. For Players, the `ASStateSnapshot` is also updated.
3. **Confirm/Correct:** The server validates the request and responds. If a divergence occurs, the server sends the authoritative state. The client then performs a **Rollback**, locating the corresponding tick in the `cache_buffer` to instantly restore attributes and tags.
4. **Determinism:** Gameplay logic (Damage magnitude, costs) must be pure and rely exclusively on data contained within the ASC and its Specs to ensure the same input generates identical output across all instances.

---

## 10. TECHNICAL RIGOR AND TEST QUALITY

### 10.1 The 300% Standard (Iron Law)

Each feature must be proven by at least **3 variations** in the same test:

1. **Happy Path:** Ideal base scenario.
2. **Negative:** Invalid input or expected failure.
3. **Edge Case:** Complex combinations (multi-tags, boundary limits).

### 10.2 Test Suites

- **Core (Unit):** Atomic, side-effect free tests.

- **Advanced (Integration):** Periodic DoT/HoT, complex RPG flows.

- **Multiplayer (Simulation):** Executed via `utility/multiplayer/runner.py` with injected latency.

---

## 11. API NAMING CONVENTIONS (AS-SPECIFIC)

To maintain consistency, the public API must strictly follow these Ability System-specific patterns:

### 11.1 Method Prefixes

Methods are categorized by intent and access layer:

- **🎮 Gameplay Layer (Game Logic Usage)**
  - `try_activate_...`: **Safe Execution.** Attempts to trigger logic that depends on prerequisites. Integrates validation and action. **Mandatory for abilities and effects.**
  - `can_...`: **Pre-authorization.** Evaluates if an action can be executed without starting it.
  - `is_...`: **Status Query.** Checks boolean conditions for state or identity (e.g., `is_ability_active`).
  - `has_...`: **Ownership Query.** Verifies if the object holds a specific key (e.g., `has_tag`).
  - `get_...`: **Information Extraction.** Retrieves values, references, or read-only metadata.
  - `cancel_...`: **Interruption.** Voluntarily terminates an active flow.
  - `request_...`: **Network Intent.** Requests action execution via RPC (Multiplayer).

- **🏗️ Infrastructure/Internal Layer (Restricted or Configuration Usage)**
  - `apply_...`: **Forced Application.** Injects a payload or container, bypassing activation rules. Used during initialization or by delivery systems (`ASDelivery`).
  - `add_...` / `remove_...`: **Low-level Mutation.** Modifies internal collections. Must not be used as a shortcut for activating game logic (e.g., use `try_activate` instead of trying to manually "add" an effect).
  - `unlock_...` / `lock_...`: **Inventory Management.** Changes ability availability in the actor's catalog.
  - `register_...` / `unregister_...`: **System Binding.** Connects the component to external nodes or manages the global Singleton.
  - `rename_...`: **Internal Refactoring.** Changes Tag identifiers and propagates the change.
  - `set_...`: **Direct Mutation.** Defines raw values (Base Values).
  - `clear_...`: **Total Reset.** Absolutely clears states or collections.
  - `capture_...`: **Persistence.** Freezes current state into a snapshot.

> [!IMPORTANT]
> **Direct `activate` does not exist in the public API.** Using `try_...` is the only safe path for gameplay. Infrastructure methods (`apply_`, `add_`) exist for low-level systems and must not bypass cost and cooldown checks.

### 11.2 Origin and Context Suffixes

Suffixes are mandatory to resolve ambiguities in input, target, or behavior:

- **Origin (Input):**
  - `..._by_tag`: Operation based on the global tag identity (e.g., `has_tag`, `try_activate_ability_by_tag`).
  - `..._by_resource`: Operation based on the resource file instance (e.g., `try_activate_ability_by_resource`).

- **Target:**
  - `..._to_self`: Action applies exclusively to the calling component.
  - `..._to_target`: Action requires an explicit target component as an argument.

- **Behavior:**
  - `..._debug`: Returns verbose or non-optimized data for diagnostic tools.
  - `..._preview`: Performs theoretical calculations ("What if?") without side effects or resource consumption.
  - `..._all`: Mass operation affecting the entire relevant collection (e.g., `cancel_all_abilities`).

### 11.3 Arguments and Typing

Argument naming must be self-explanatory and follow the project's type hierarchy:

- **Descriptive Naming:** Single-character variables (e.g., `t`, `a`) are forbidden. Use full conceptual names (`tag`, `ability`, `effect`, `attribute`, `level`, `value`, `target_node`, `data`).

- **Performance Typing:**
  - `StringName`: Used for all tag identifiers and attribute names (dictionary keys).
  - `float`: Used for all magnitudes, levels, and durations (even if logically integers, the attribute engine operates in floating point).
  - `Dictionary`: Used for variable data payloads (e.g., in the Cue system).

- **Ordem Canônica:** When multiple arguments are required, follow importance:
  1. **Identifier:** `tag` or `resource`.
  2. **Magnitude:** `level` or `value`.
  3. **Target/Context:** `target_node` or `data`.

- **Default Values:** Optional arguments must have documented neutral values (e.g., `level = 1.0`, `target_node = null`).

### 11.4 Signals

Signals communicate events that have **already occurred**, following the passive voice pattern:

- **Format:** `snake_case` in past tense.

- **Correct Examples:** `ability_activated`, `effect_removed`, `attribute_changed`, `tag_event_received`.

- **Incorrect Examples:** `on_ability_activate` (unnecessary prefix), `activate_ability` (confuses with method).

### 11.5 Internal Members and Properties (C++)

To ensure safety and readability in GDExtension source code:

- **Private/Protected Variables:** Must start with `_` (underscore). E.g., `_attribute_set`, `_is_active`.

- **Reactive Getters/Setters:** Whenever a variable change requires re-evaluation (e.g., changing a tag requires recounting `ASTagSpec`), a formal setter must be used instead of direct access.

- **Public Properties:** Must mirror raw names (without `_`) to be correctly exposed to the Godot Inspector.

- **Dual Build:** All test code must support compilation via `#ifdef ABILITY_SYSTEM_GDEXTENSION`.

---

## 12. DESIGN PATTERNS (C++/GDEXTENSION)

The system utilizes classic patterns adapted for Godot Engine's high-performance architecture.

### 12.1 Spec Pattern (Resource vs. Instance)

- **The Problem:** Godot Resources are shared. Modifying one would affect all enemies of that type.

- **The Solution:** Total separation between **Resource** (Immutable Data/DNA) and **Spec** (Runtime State/Instance).

- **Rule:** All logic requiring value changes (timers, stacks) must reside in the Spec. The Resource is solely a Data Provider.

### 12.2 Flyweight Pattern (Memory Optimization)

- **Concept:** Thousands of actors share the same `ASAbility` and `ASEffect` references.

- **Implementation:** `ASComponent` stores only pointers (RefCounters) to Resources. Heavy data (icons, damage curves) is never duplicated in memory.

### 12.3 Command Pattern (Abilities)

- **Concept:** Each ability is a self-contained command knowing how to start, execute, and cancel.

- **Requirement:** Total encapsulation. The component must not know the internal logic of the ability; it merely dispatches the command.

### 12.4 Data-Driven Design

- **Rule:** Behavior must be defined in `.tres` files within the Editor, not in hardcoded logic.

- **Advantage:** Allows designers to adjust balance without recompiling the plugin.

---

## 13. TEST PATTERNS (RIGOR AND DETERMINISM)

System reliability is ensured through industrial test patterns.

### 13.1 Deterministic Physics Ticking

- **Rule:** Time-based tests (Cooldowns/Duration) must be validated via fixed-step `physics_process` (Ticks).

- **Time Mocking:** In unit tests, we simulate time progression by manually calling `tick(delta)` to ensure 1.0s is exactly 1.0s, regardless of hardware lag.

### 13.2 Isolation & Mocking

- **Mock Assets:** To test complex abilities, we create temporary in-memory Resources via code within the `TEST_CASE`.

- **Dummy Actors:** Using simple nodes with an `ASComponent` is preferred over loading complex scenes (`.tscn`) for unit tests, ensuring execution speed.

### 13.3 State Injection

- **Pattern:** Instead of waiting 10 seconds to test an effect's end, the test should inject a Spec with `duration_remaining = 0.1` and validate the subsequent tick.

### 13.4 Signal Auditing

- **Pattern:** Every activation test must audit whether the corresponding signal (e.g., `ability_activated`) was emitted with the correct arguments, ensuring visual feedback (Cues) also triggers correctly.
