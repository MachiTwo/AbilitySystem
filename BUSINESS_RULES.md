# BUSINESS RULES: ABILITY SYSTEM - GOVERNANCE CONTRACT

This document establishes the architectural boundaries and mandatory business rules. Any implementation violating these limits must be refactored immediately.

---

## 1. PHILOSOPHY AND RIGOROUS ENGINEERING

The project rejects **"Vibe-Coding"** (programming by intuition or luck). Every line of business logic is treated as an industrial engineering commitment.

### 1.1 Pair Programming and Governance

- **Radical Code Detachment:** If code fails, it's a communication or architecture flaw. Corrections are made via dialogue and documentation adjustment, never manual patches.
- **SSOT (Single Source of Truth):** This file is the Iron Law. Before any complex change, the rule must be documented here.
- **Language:** Code and technical documentation in **English**. Dialogue and pair programming pitch in **Portuguese**.

### 1.2 TDD Protocol (Red-Green-Refactor)

No business logic exists without a test justifying it.

1. **RED:** Write the failing test, defining the contract.
2. **GREEN:** Implement the minimal code to pass.
3. **REFACTOR:** Optimize while maintaining the passing status.

---

## 2. THE IDENTITY MATRIX: TAGS (THE "STATE")

Tags are not classes; they are **Superpowered Hierarchical Identifiers** based on `StringName`. They represent the absolute truth about an Actor's present state.

### 2.1 Tags Golden Rules (What I AM)

- **Role:** Represent continuous states, traits, and blocks (e.g., `State.Dead`, `Status.Stunned`).
- **Nature:** Persistent. Require formal addition (`add_tag`) and formal removal (`remove_tag`). They consume CPU time calculating RefCounts in `ASTagSpec`.
- **Answers the Question:** _"In this exact microsecond, is this actor under condition X?"_
- **Absolute Prohibition (Anti-Pattern):** NEVER use tags to represent instant occurrences (e.g., DO NOT use `State.JustGotHit`). If the condition lasts only 1 frame or serves to alert systems, it MUST be an Event, never a Tag.

### 2.2 The 3 Canonical Tag Types

The `Tag Type` defines the **semantic role** a tag has. It determines how the Singleton and Editor treat that identifier.

| Type          | Conventional Prefix  | Role                                                                                                 | Example                                     |
| ------------- | -------------------- | ---------------------------------------------------------------------------------------------------- | ------------------------------------------- |
| `NAME`        | `Char.` / `Team.`    | Static identity and categorization of the actor                                                      | `Char.Warrior`, `Team.Blue`                 |
| `CONDITIONAL` | `State.` / `Status.` | Persistent gameplay state that can be added/removed from the actor                                   | `State.Stunned`, `Status.Poisoned`          |
| `EVENT`       | `Event.`             | Occurrence identifiers. Registered for autocomplete, but **the payload never reaches the Singleton** | `Event.Weapon.Hit`, `Event.Damage.Critical` |

### 2.2.1 Tag Groups (Visual Organization)

**Tag Groups are not code entities.** They are an editorial convention emerging automatically from dot hierarchy (`.`) in the tag identifier.

- `ASTagsPanel` renders tags as a **tree**, using each dot-separated segment as a parent node.
- `State.Stunned`, `State.Dead` automatically group under the visual node `State`.
- There is no `TagGroup` C++ object — the "group" is just the shared prefix.
- **Mandatory convention:** The root prefix MUST reflect its `Tag Type` (e.g., `Event.*` tags are always `TAG_TYPE_EVENT`).

### 2.3 Logical Evaluation (Predicates)

The system supports 4 logic states in Blueprints (Ability, Effect, Cue) for evaluating target requirements/blocks:

- `Required All` (AND): Success if possessing ALL.
- `Required Any` (OR): Success if possessing AT LEAST ONE.
- `Blocked Any` (OR): Fails if possessing ANY.
- `Blocked All` (AND): Fails if possessing ALL SIMULTANEOUSLY.

> [!NOTE]
> Predicates work exclusively on `CONDITIONAL` tags. `NAME` and `EVENT` tags do not enter requirement/block lists. The Editor enforces this automatically via `ASInspectorPlugin`.

---

## 3. THE NERVOUS SYSTEM: EVENTS & HISTORICAL (THE "ACTION")

Unlike Tags, **AS Events** are instant messengers (transient signals) flowing through the system informing crucial occurrences. They prevent spaghetti code and eliminate the destructive practice of adding/removing tags on the same tick.

### 3.1 Events Golden Rules (What HAPPENED)

- **Role:** Represent specific informative occurrences (e.g., `Event.Weapon.Hit`, `Event.Ability.Failed`).
- **Nature:** Volatile/Fleeting. Dispatched (`dispatch_event`) and processed by the event bus instantly.
- **Answers the Question:** _"What just happened, where, and how strong was it?"_

### 3.2 The Event Payload (`ASEvent`)

An event is never just a name. The power resides in its Payload (`ASEvent`), transmitting full context:

- `event_tag`: Exact occurrence identifier (e.g., `Event.Interrupt`).
- `instigator`: The Node that caused it (offender).
- `target`: The affected Node (victim).
- `magnitude`: Base intensity of the event (Impact power).
- `custom_payload`: GDScript/Variant dictionary for transmitting metadata.
- `timestamp`: Registered natively in precise milliseconds.

### 3.3 Short-Term Memory (Events Historical)

Events die in 1 tick, but their _"memory"_ persists lightly:

- `ASComponent` keeps an `_event_history` (optimized C++ circular buffer up to 64 entries).
- **Practical use:** Reactive components (like _Parry_ or _Counter-Attack_) don't need to be in an eternal "parrying" state. They can check recent past: `has_event_occurred(&"Event.Damage.Block", 0.4)`. If it occurred in the last 0.4s, authorize the counter-attack.
- **Rule:** Never use this cache to persist history (quests, missions). Use exclusively for temporal reactivity frames.

### 3.4 Split Registry Pattern

Event identifiers (e.g., `Event.Weapon.Hit`) **are registered in the Singleton** like any other tag — for autocomplete and typos prevention. The difference is their type: registered as `Tag Type = EVENT`.
What **never** goes to the Singleton is the **data instance** — the `ASEvent` struct. This is the **Split Registry**:

- **Singleton (Registry):** Knows the _name_ `Event.Weapon.Hit`. Guarantees it exists, has the right type, appears in autocomplete.
- **ASComponent (Occurrence):** Knows the _happening_. Knows who hit, whom, with what force, and on what tick. The Singleton doesn't — and shouldn't — know this.

> [!IMPORTANT]
> **Golden Rule:** Never call `dispatch_event` with an `event_tag` not registered in the Singleton with `Tag Type = EVENT`. It's equivalent to using a hardcoded StringName without validation.

### 3.5 How to Emit an Event (API & Usage Contract)

**Events are always imperative — never automatic.** The system does not passively observe effects or abilities and emits events on its own. Code does (GDScript, C++ or `ASDelivery`).

#### API

```gdscript
# Signature: dispatch_event(event_tag, instigator, magnitude, custom_payload)
var asc: ASComponent = target.get_node("ASComponent")
asc.dispatch_event(&"Event.Weapon.Hit", self, 35.0, {})
```

#### Who can emit:

| Origin                        | When to use                                                           |
| ----------------------------- | --------------------------------------------------------------------- |
| **GDScript / Direct C++**     | Physical collisions, player inputs, custom logic                      |
| **ASDelivery**                | Automatically emits the `ASPackage.tag` event upon colliding with ASC |
| **ASAbility (via GDVirtual)** | `_on_activate_ability` can call `dispatch_event` at the right time    |

#### Events and Resources — Fundamental Distinction

- **Imperative Emission (Code):** Whenever you need absolute control, call `dispatch_event` directly.
- **Declarative Emission (Resource):** A Resource can **declare intention** of which events should be fired at canonical moments of its lifecycle. The executor (`ASComponent`, `ASDelivery`) reads this list and calls `dispatch_event` at the right time.
- **Reaction:** An `ASAbility` Resource **MAY react** to events by registering a `TRIGGER_ON_EVENT` in the `triggers` list.
- **Summary:** The Resource is the **declarer/reactor**. The executor is always the **emitter**.

### 3.6 Event Declaration Pattern

To make the system data-driven without breaking rules, Resources **declare** which events are relevant for the actor and when they should be emitted. The executor still uniquely calls `dispatch_event` — but reads the Resource intent.

> [!NOTE]
> The Resource **never** calls `dispatch_event` directly. It only declares `StringName` lists with `EVENT` tags. The executor iterates these lists and dispatches.

#### Table 1 — Subscription: Who listens to what?

This is the `ASContainer` responsibility. Without declaring events here, the `ASComponent` won't know which event tags to route to its `TRIGGER_ON_EVENT`.
| Resource | Property | Role |
| ------------- | --------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ASContainer` | `events: Array[StringName]` | Declares which `EVENT` tags the actor is subscribed to. When initialized, the ASComponent routes these events to triggers and history buffer. |

> [!IMPORTANT]
> If `Event.Weapon.Hit` isn't in the Archer's `ASContainer`, a `Counter-Shot` ability with `TRIGGER_ON_EVENT = Event.Weapon.Hit` **will never activate**, even if someone calls `dispatch_event` manually.

#### Table 2 — Emission: When each Resource fires an event?

Canonical moments where the executor reads the declared list and automatically emits them.
| Resource | Property | Fire Moment | Executor |
| ----------- | --------------------------------------- | ----------------------------------------- | ------------- |
| `ASPackage` | `events_on_deliver: Array[StringName]` | When `ASDelivery` delivers to ASC | `ASDelivery` |
| `ASAbility` | `events_on_activate: Array[StringName]` | When the ability successfully activates | `ASComponent` |
| `ASAbility` | `events_on_end: Array[StringName]` | When the ability ends/expires | `ASComponent` |
| `ASEffect` | `events_on_apply: Array[StringName]` | When the effect is applied to the actor | `ASComponent` |

#### Full Data-Driven Flow Example

```
# 1. "Archer" ASContainer declares listening to:
ASContainer.events = [&"Event.Weapon.Arrow.Hit", &"Event.Damage.Taken"]

# 2. "Arrow" ASPackage declares emitting on impact:
ASPackage.events_on_deliver = [&"Event.Weapon.Arrow.Hit"]

# 3. "Counter-Shot" ASAbility reacts:
ASAbility.triggers = [{tag: &"Event.Weapon.Arrow.Hit", type: TRIGGER_ON_EVENT}]
```

Result: Archer hit by arrow → `ASDelivery` delivers package → emits `Event.Weapon.Arrow.Hit` → `Counter-Shot` auto-activates. **Zero lines of code.**

---

## 4. THE SINGLETON: ABILITY SYSTEM (PROJECT INTERFACE)

- **Role:** **Global Configuration API** and bridge to `ProjectSettings`.
- **Rules:**
  - Exclusively responsible for saving/loading the global tags list in `project.godot`.
  - Acts as a **Central Name Registry** ensuring resources don't conflict.
- **Limit:** Must not store state for any Actor.

---

## 5. TOOLS LAYER: EDITORS

Human interface to Resources.

### 5.1 ASEditorPlugin

- **Role:** **Bootloader**.
- **Rule:** Type registry, icons, sub-editors initialization. Forbidden from containing game logic.

### 5.2 ASTagsPanel

- **Role:** Visual interface for **Global Registry**.
- **Rule:** Exclusively manipulates the `AbilitySystem` Singleton tag dictionary.

### 5.3 ASInspectorPlugin (and Property Selectors)

- **Role:** Contextualization.
- **Rule:** Must provide smart selectors (tag dropdowns, attribute searches) for Inspector configuration.

### 5.4 The Compat Layer

Located in `src/compat/`.
The project is strictly architected under the **Dual Build Strategy**, supporting both Godot native Module and GDExtension compilation.

- **Role:** Shield the framework core from Module and GDExtension internal API divergences.
- **Rule:** All central logic (`src/core`, `src/resources`, `src/scene`) MUST be agnostic. Any structural difference needed to interact with the Engine (especially Editor GUI features) must be isolated in this folder, solving compatibility via `#ifdef ABILITY_SYSTEM_GDEXTENSION`.
- **Exclusivity:** Core files must never directly use GDExtension-exclusive classes that break Module C++ compilation; they must invoke the Wrapper in `compat/`.

---

## 6. THE BLUEPRINTS: RESOURCES (THE "WHAT")

Located in `src/resources/`. **Data Definitions**.

- **Resources (Blueprints):** Static (`.tres`) objects defining the "DNA". **Rule:** Must not be modified at runtime (except `ASStateSnapshot`).
- **Specs (Runtime Instances):** Lightweight instances holding dynamic state (cooldowns, stacks).
- **Snapshot Exception:** `ASStateSnapshot` breaks immutability for native persistence (Save/Load) and net caching.

### Usage & Performance Restrictions

> [!WARNING]
> **`ASStateSnapshot` is a heavy resource.** Capturing full state consumes significant CPU/RAM if misused.

1. **Player Exclusive:** Snapshots restricted to Playable Characters (where determinism/rollback are critical).
2. **NPCs/Enemies:** Non-playables **MUST NOT** use `ASStateSnapshot`.
3. **Golden Rule:** If it's playable, use `ASStateSnapshot`. If not, discard it.
4. **Reference Independence:** Stores primitive values and tag names, not pointers, ensuring safe serialization.

### 6.1 ASAbility & ASEffect (Actions & Modifiers)

- **ASAbility - Role:** Define action logic (Costs, Cooldown, Triggers).
- **ASAbility - Rule:** Only Resource capable of managing activation requirements and attribute costs. Supports **Ability Phases** for complex executions.
- **ASEffect - Role:** State modifier (Buffs, Debuffs, Damage).
- **ASEffect - Rule:** Defines stacking policies and attribute change magnitudes.

### 6.2 ASAttribute & ASAttributeSet (Attribute System)

- **ASAttribute - Role:** Represents metadata (min/max limits) of a single stat.
- **ASAttributeSet - Role:** Groups stats and defines an initial character state. Holds modification logic.
- **ASAttributeSet - Rule (Attribute Drivers):** Allows deriving base values from another (e.g., 2 \* Strength = 1 Attack). Recalculates automatically.
- **ASAttributeSet - Rule (Priority):** Modifiers (Flat, Multiplier) apply _after_ Driver calculation.

### 6.3 ASContainer & ASPackage (Archetypes & Payloads)

- **ASContainer - Role:** Complete archetype (Actor Identity Dictionary). Factory template for `ASComponent`.
- **ASPackage - Role:** Transport wrapper (Data Envelope). Used exclusively for `ASDelivery`.

### 6.4 ASCue (Visual Feedbacks)

- **Role:** Pure audiovisual feedback (Animation, Sound, Particles).
- **Rule:** Forbidden from altering gameplay data. Reactive dispatch only.

### 6.5 ASAbilityPhase (Complex Lifecycle)

Most powerful feature for designers, acting as embedded "State Machines" (Hierarchical Abilities).

- **Role:** Fragment rigid ability execution into granular stages (`Windup`, `Execution`, `Recovery`).
- **Nature:** Unlike "pistol" standard abilities, phased abilities act as "rituals" with timed steps.
- **Critical Rules:**
- **Temporary & Specific:** Each Phase can apply/remove transit `ASEffects` lasting only while the phase is active.
- **Duration or Event:** Advances via (a) expired duration; (b) _Transition Trigger Event_ occurrence (e.g., waiting for an Animation Node `.Hit` Event).
- **Autonomous Alerts:** Phase transitions always fire an automatic AS Event for UI/systems fluency.

---

## 7. THE EXECUTORS: SPECS (THE "HOW")

Located in `src/core/`. Where execution state and logic reside.

- **Role:** Represent the **Active Instance**. Owner of the **"Now"**.
- **Golden Rule: STATE SOVEREIGNTY.**
- **What lives here (not in Component):**
  - `duration_remaining`: Individual timers.
  - `stack_count`: Accumulation amount.
  - `calculate_...`: Logic dependent on variable attributes.

### 7.1 ASAbilitySpec & ASEffectSpec (Execution)

- **ASAbilitySpec:** Active/equipped ability handling cooldowns.
- **ASEffectSpec:** Active modifier holding sovereignty over `duration` and `stacks`.

### 7.2 ASCueSpec & ASTagSpec (Feedback & Identity)

- **ASCueSpec:** Manages scene feedback lifespan. Ensures Queue Free on end.
- **ASTagSpec:** Tag Reference Counter (Refcount). Guarantees tag leaves only when all origin Specs expire.

### 7.3 ASAbilitySpec (Phase Management)

- **Role:** Manages current phase index and timeline progression.
- **Rule:** Validates advancing via time ticks or receiving a specific `ASEvent`.

---

## 8. THE ORCHESTRATOR: COMPONENT (THE HUB)

The `ASComponent` (ASC).

- **Role:** **Collection Manager** and **Signal Router**.
- **Rules:**
  - Does not manage individual timers (Spec does).
  - Holds `active_specs` and `unlocked_specs`.
  - Is the **Attribute Owner** (via `AttributeSet`).
  - Is the only entity capable of adding/removing tags on the Actor.
- **Determinism Guarantee:** Must rollback and predict state via `ASStateSnapshot`.
- **State Cache:** Maintains internal buffer of snapshots for net sync.
- **ASEventsHistorical (Event Memory):**
- **Role:** Circular buffer for recent events enabling conditional logic.
- **Queries:** "Did event X happen in the last Y ticks?".
- **Sync:** Tick-sensitive to guarantee consistency on Rollbacks.

---

## 9. DELIVERY & REACTIVITY SYSTEMS

### 9.1 ASDelivery (Payload Injections)

- **Role:** Decouples emitter from target in spatial interactions (AoEs, Projectiles).
- **Rule:** Transports `ASPackage` and injects upon colliding with ASC.

### 9.2 Ability Triggers (Reactive Automation)

- **Role:** Automatic ability activation based on State (Tags) or transient events (AS Events).
- **Rule:** Exclusively based on `ON_TAG_ADDED`, `ON_TAG_REMOVED`, or `ON_EVENT_RECEIVED`.

---

## 10. REPLICATION & PERSISTENCE (DETERMINISM)

Authoritative multiplayer with Prediction/Rollback support.

- **Truth Source (Physics Only):** Only `tick` is a valid temporal identifier. `ASComponent` operates **exclusively** via `physics_process`. Hard block on `_process`.

### 10.1 ASStateSnapshot (Heavy Resource)

- Heap-allocated **Godot Resource** (`.tres`). Supports native serialization for long-term Save/Load.
- Player-exclusive usage.

### 10.2 ASStateCache (Light Structure)

- Pure C++ Struct allocated on inline stack/vector.
- Fast, zero-overhead sync for non-playables/NPCs.

### 10.3 Net Activation Flow

1. **Request:** Client requests via `request_activate_ability(tag)`.
2. **Predict:** Client executes local prediction, generates `cache_buffer` entry.
3. **Confirm/Correct:** Server validates. If diverging, Server sends authoritative state, Client **Rollbacks** to corresponding tick.
4. **Determinism:** Gameplay logic MUST heavily rely purely on ASC and Spec properties.

---

## 11. THE REACTIVITY & FLOW PROTOCOL (TRICHROMATIC MATRIX)

Ensures the 3 Pillars operate in orchestrated unison without spaghetti coupling.

### 11.1 The Natural Order

1. **INPUT/ACTION:** Interaction, timeout, or physical impact emits an **AS Event** (`Event.Damage`).
2. **LISTEN/PROCESS:** Entity listens via Triggers (`ON_EVENT`).
3. **MUTATION:** Reactive ability meets requirements, invokes and applies modifier (`ASEffect`).
4. **STATE (Cycle End):** Effect mutates attributes or permanently adds an **AS Tag** (`State.Stunned`).

> [!CAUTION]
> **Fatal Errors punished by deep refactoring:**
>
> - Waiting for an ability to start based on "tag loss" (acception coupling; emit an Event instead).
> - If an Ability fails requirements, NEVER apply temporary hack tags. Emit `Event.Ability.Failed` passing the reason (Payload Dict).

### 11.2 Hybrid Era Triggers

- `TRIGGER_ON_EVENT`: Gold standard for combat responsiveness (Instant Reaction).
- `TRIGGER_ON_TAG_ADDED/REMOVED`: Standard for environmental automation (e.g., Slow aura on entering water).

### 11.3 The ASDelivery Pact

- **Obligation:** Every ASDelivery successfully finishing its route MUST emit `target_asc->dispatch_event(package_tag)`. Fills the Historical buffer, enabling clean reactive blocks.

---

## 12. TECHNICAL RIGOR & TEST QUALITY

### 12.1 The 300% Standard (Iron Law)

Every feature proved by at least **3 variations** in the same test:

1. **Happy Path:** Ideal base scenario.
2. **Negative:** Invalid input or expected fail.
3. **Edge Case:** Complex combinations (multi-tags, bounds).

### 12.2 Test Suites

- **Core:** Atomic, no side-effects.
- **Advanced:** Periodic DoT/HoT, complex RPG flows.
- **Multiplayer:** Executed via `runner.py` with injected latency.

---

## 13. API NAMING PATTERNS

### 13.1 Method Prefixes

- **🎮 Gameplay Layer (Game Logic Usage)**
  - `try_activate_...`: **Safe Execution.** Validates requirements then invokes.
  - `can_...`: **Pre-authorization.** Answers if feasible.
  - `is_...`, `has_...`: Query statuses mapping.
  - `get_...`, `cancel_...`, `request_...` (Net intention).
- **🏗️ Internal/Infrastructure Layer**
  - `apply_...`: Forced application ignoring capabilities (for Init or ASDelivery).
  - `add_...` / `remove_...`: Low-level collections mutator. (Do not use for gameplay logic).
  - `unlock_...` / `lock_...`, `register_...`, `rename_...`, `set_...`, `clear_...`, `capture_...`.

> [!IMPORTANT]
> **There is no direct `activate` in the public API.** `try_...` is the only safe gameplay path.

### 13.2 View Suffixes

- `..._by_tag` vs `..._by_resource`
- `..._to_self` vs `..._to_target`
- `..._debug`, `..._preview`, `..._all`.

### 13.3 Types & Parameters

- `StringName` (tags), `float` (magnitude/levels), `Dictionary` (Custom Payloads).
- Order: Identifier (`tag`) > Magnitude (`level`) > Context (`target_node`).

### 13.4 Signals and C++ Properties

- **Signals:** Passive voice past-tense (`ability_activated`, NOT `on_ability_activate`).
- **Internal C++:** Privates prefixed with `_`. Reactive Setters required over direct modifications.

---

## 14. DESIGN PATTERNS

- **Spec Pattern:** Separation between Data Blueprint (Resource) and Runtime Instance (Spec).
- **Flyweight:** ASC holds pointers to Resources; heavy static data is never duplicated in RAM.
- **Command:** Each ability is a self-contained Command.
- **Data-Driven:** Configured via `.tres`, zero hardcoded values.

---

## 15. TEST PATTERNS

- **Deterministic Ticking:** Tested strictly via `tick(delta)`.
- **Isolation/Mocking:** Temporary resources created in memory.
- **State Injection:** Injecting an expiring Spec instead of waiting for full timer loops.
- **Signal Auditing:** Every activation test must audit if the canonical Signal accurately fired.
