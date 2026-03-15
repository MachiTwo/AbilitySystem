# 🛠️ API Reference (v0.1.0 Stable)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](API.md) | [**Português**](API.pt.md)

---

## 1. Core Systems

### `AbilitySystem` (Global Singleton)

The central authority for the Tag system and resource naming registration.

- **Constants (TagType):**
  - `TAG_TYPE_NAME` (0): Identity tags for permanent resources.
  - `TAG_TYPE_CONDITIONAL` (1): Dynamic state tags (e.g., `state.stunned`).

- **Methods:**
  - `register_tag(tag, type=0, owner_id=0)`: Registers a tag globally.
  - `unregister_tag(tag)`: Removes a tag.
  - `rename_tag(old, new)`: Renames a tag and all its children.
  - `remove_tag_branch(tag)`: Recursively removes a tag and its sub-hierarchy.
  - `tag_matches(tag, match_against, exact=false)` (static): Hierarchical comparison logic.
  - `get_registered_tags()`: Returns an array of all global tags.

---

### `ASComponent` (ASC)

The logic hub for any actor. Manages attributes, abilities, effects, and tags.

#### 🔖 Tags & State

- `add_tag(tag)`: Adds a tag and triggers re-evaluations.
- `remove_tag(tag)`: Removes a tag.
- `remove_all_tags()`: Clears all owned tags and triggers removal events.
- `has_tag(tag)`: Checks for tag dominance (supports hierarchy).
- `get_tags()`: Returns all currently active tags.
- `get_attribute_sets()`: Returns all attached attribute sets.

#### ⚔️ Abilities

- `try_activate_ability_by_tag(tag)`: Attempts activation via tag.
- `try_activate_ability_by_resource(ability)`: Attempts activation via resource.
- `cancel_ability_by_tag(tag)`: Aborts active instances matching the tag.
- `unlock_ability_by_tag(tag)`: Makes an ability available for the actor.
- `is_ability_active(tag)`: Checks if an ability is currently running.
- `get_active_abilities()`: Returns current `ASAbilitySpec` instances.

#### 📊 Attributes

- `get_attribute_value_by_tag(tag)`: Gets the current calculated value.
- `get_attribute_base_value_by_tag(tag)`: Gets the unmodified base value.
- `set_attribute_base_value_by_tag(tag, value)`: Updates the base value.
- `has_attribute_by_tag(tag)`: Checks if the attribute exists on this actor.

#### ✨ Effects

- `apply_effect_spec_to_self(spec)`: Directly injects an effect payload.
- `has_active_effect_by_tag(tag)`: Checks if a specific effect is active.
- `remove_effect_by_tag(tag)`: Removes instances of the effect.

#### 🎭 Cues & Notifications

- `register_node(alias, node)`: Registers a node for cue lookups (e.g., "Muzzle").
- `try_activate_cue_by_tag(tag, data={})`: Triggers visual/audio feedback.
- `set_animation_player(node)`: Registers the primary animation controller.

---

## 2. Resources (Blueprints)

### `ASAbility`

Configuration for actions with costs, cooldowns, and requirements.

- **Duration Policies:** `INSTANT`, `DURATION`, `INFINITE`.
- **Triggers:** `ON_TAG_ADDED`, `ON_TAG_REMOVED`.
- **Tag Logic (Activation):**
  - `required_all`: Owner must have **every** tag.
  - `required_any`: Owner must have **at least one** tag.
  - `blocked_any`: Activate fails if owner has **any** of these.
  - `blocked_all`: Activate fails only if owner has **all** of these.
- **Key Properties:** `costs`, `requirements`, `cooldown_duration`, `activation_owned_tags`.

### `ASEffect`

Blueprints for attribute modifications and tag injections.

- **Modifier Ops:** `ADD`, `MULTIPLY`, `DIVIDE`, `OVERRIDE`.
- **Stacking:** `NEW_INSTANCE`, `OVERRIDE`, `INTENSITY`, `DURATION`.
- **Periodic:** Supports `period` for tick-based execution (DoT/HoT).

### `ASContainer`

An archetype database (AttributeSet + Default Abilities + Initial Effects). Use `asc.apply_container(resource)` to initialize an actor.

### `ASAttributeSet`

Container for numerical attributes and derivation logic.

- **Attribute Drivers:** Allows one attribute to drive another (e.g., `stat.str` -> `stat.atk`) via a ratio. Automatically recalculates on base value changes.
- **Modifiers:** Can receive temporary or permanent `ADD`, `MULTIPLY`, or `OVERRIDE` modifications from Effects.

### `ASPackage`

A portable bundle of Effects and Cues. Ideal for items or multi-step logic.

---

## 3. Specialized Systems

### `ASDelivery`

Handles payload transportation for bullets, traps, and AoEs.

- `add_effect(effect)` / `add_cue(cue)`: Prepare the payload.
- `deliver(target_asc)`: Injects the payload into a destination ASC.

---

## 4. Runtime Objects

### `ASAbilitySpec` / `ASEffectSpec`

Active instances of their respective resources. They hold variable state (remaining duration, level, etc.).

---

## 5. LimboAI Integration

The `ASComponent` is natively compatible with LimboAI via:

- **Triggers:** React to tag changes in the Behavior Tree.
- **BT Nodes:** (Recommended) Use `BTCheckTag` and `BTActivateAbility` for state-driven AI logic.

---

> [!NOTE]
> For the complete C++ class reference, refer to the in-engine help or the `doc/source/classes` directory.
