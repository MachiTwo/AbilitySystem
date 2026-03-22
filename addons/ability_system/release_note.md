# 🚀 Release Notes - v0.1.0 (Official Release)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](release_note.md) | [**Português**](release_note.pt.md)

We are proud to announce the stable release (v0.1.0) of the **Ability System Framework**, a mature, robust, and data-driven solution for Godot. Available as both a **GDExtension** plugin and a **Native Module** (C++), the system brings rigorous engineering of complex features to your game's architecture.

---

## 1. Orchestration and Base Entities

- **`ASComponent` (The Brain)**: Attached to your `CharacterBody`, it operates exclusively on `physics_process` (ensuring determinism). It owns attributes and manages the list of active Specs.

- **`ASContainer` (Class Template)**: Eliminates manual setup. It acts as an archetype to fully initialize the component from a single `.tres`, making it easy to structure a "Mage" or "Warrior" based on pre-selected inventories of abilities and native attributes.

- **`ASDelivery` & `ASPackage`**: Pure logic injectors! `ASDelivery` is an agnostic `Node` (not dependent on 2D or 3D) that carries an `ASPackage`. It can be attached to any entity or projectile to inject collections of effects and cues into a target, decoupling gameplay logic from the physical collision infrastructure.

## 2. Reusable Resources (Immutable Blueprints)

- **`ASAbility`**: The core of an ability, validating level requirements, tags, and costs.

- **`ASEffect`**: Isolated state modifiers with native support for duration, individual cooldowns, and precise stacking policies.

- **`ASCue`**: Pure visual reactivity layer. Extensions (like `ASCueAudio`) operate independently without affecting game server data.

- **`ASAttribute`**: Fundamental stats that react dynamically via **Attribute Drivers**. Changing a base strength reflexively alters the total calculated damage.

## 3. The Sovereignty of "Specs" (Runtime)

While `ASAbility` is static in the project folder, **`ASAbilitySpec`**, **`ASEffectSpec`**, **`ASCueSpec`**, and **`ASTagSpec`** are active instances based on `RefCounted`. This solves the overlapping problem! Opponents of different levels can use the exact same base ability — the Specs retain and calculate the current state based on the caster's isolated status, clearing from memory at the end of their utility.

---

## 🏷️ The Global Tag Engine

We do not manage booleans spread across code; we manage **Tags**.

- **`AbilitySystem` (Singleton)**: The global server that extends _Project Settings_. It registers tags in the engine. It is the only entity that loads and saves the global list in `project.godot`.

- **NameTag & ConditionalTag**: Tags identify the source (`NameTag` e.g., `Ability.Skill.Fireball`) and the logical semantics (`ConditionalTag` e.g., `Damage.Element.Fire`). This makes global resistance logic incredibly simple compared to matching raw ability names.

- **Logical Rules**: Built-in conditions such as `Required All`, `Required Any`, `Blocked Any`, and `Blocked All` are natively parsed in C++.

- **Automation via Ability Triggers**: Abilities now react autonomously to global Tags, without extra _scripting_. You can trigger reflex shields or counter-attacks purely by linking the `ON_TAG_ADDED` or `ON_TAG_REMOVED` of a received debuff or damage.

## API Patterns and Safe Access

The Ability System interface encourages fetching resources securely and predictably, focusing on preventing stealth mutations:

- **Dual Instantiation (`by_tag` vs `by_resource`)**: Every method features two signatures. Use `by_tag` to invoke dynamic actions natively driven by the Tag Engine, or `by_resource` for hard references (Path/UID).

- **Safe Execution (`try_activate`)**: The API does not expose a direct `activate()` for functional abilities. The mandatory use of `try_...` ensures that Blueprint rules (mana cost, cooldown, logical requirements) are irrevocably respected before any execution.

- **Authorization (`can_`)**: Evaluates whether an action is theoretically permitted to execute, without processing it.

- **Status Query (`is_` / `has_`)**: Clean and safe checks to validate Tag possession (`has_tag`) or operational states (`is_active`), discouraging the use of manual boolean checks.

- **Interruption (`cancel_`)**: Actively terminates ongoing flows of abilities or active effects.

- **Network Intent (`request_`)**: Formal methodology to request executions via RPC, delegating authority to the server.

- **Safe Extraction (`get_`)**: Retrieves values, instances, and calculated data cleanly without the risk of exposing unintended mutability.

- **Inventory Management (`unlock_` / `lock_`)**: Defines the availability and the active catalog of equipable abilities for an actor at runtime.

---

## 🌐 Multiplayer Engine

For online instances and competitive games, we added practical superpowers:

- **Multiplayer and Prediction (Rollback)**:

- **`ASStateCache`**: Lightweight C++ structure (inline struct) designed to retain recent tick history in memory for optimized local prediction.

- **`ASStateSnapshot`**: Literal capture (photo) of all fundamental status. Allows saving to disk with base Godot methods for a real **Save/Load** flow and acts as the central authority source (True State) when the Server signals corrections and forces a Rollback of the Frame.

---

## 🧬 Interactions and Intelligence (LimboAI Bridge)

Game intelligence was fundamentally redesigned to operate natively with our framework:

- **Native Bridge with LimboAI**: Behavior trees can formally control subordinate agents carrying an `ASComponent`, activating commands and consuming `BTAction` nodes with precise physical latency natively synchronized by Godot.
