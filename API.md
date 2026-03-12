# 🛠️ API Reference (v0.1.0 Stable)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](API.md) | [**Português**](API.pt.md)

---

## 1. Core Systems

### `AbilitySystem` (Global Singleton)

Central authority for Tag registration and unique resource names.

- **Enums:**
  - `TagType`:
    - `TAG_TYPE_NAME` (0): Identity tags (Ability, Effect, Cue, Attribute).
    - `TAG_TYPE_CONDITIONAL` (1): State/Status tags (state.stunned, state.dead).

- **Methods:**
  - `register_tag(tag, type, owner_id)`: Registers a new tag globally.
  - `unregister_tag(tag)`: Unregisters a tag.
  - `is_tag_registered(tag) -> bool`: Checks registration status.
  - `tag_matches(tag, match_against, exact) -> bool` (static): Hierarchical comparison.
  - `register_resource_name(name, owner_id) -> bool`: Validates uniqueness.

---

### `ASComponent` (ASC)

The central logic hub for any actor.

#### Abilities

| Method                                      | Return | Description                                              |
| :------------------------------------------ | :----- | :------------------------------------------------------- |
| `try_activate_ability_by_tag(tag)`          | `bool` | Attempts to activate via tag. Returns `true` if started. |
| `try_activate_ability_by_resource(ability)` | `bool` | Attempts to activate via resource.                       |
| `cancel_ability_by_tag(tag)`                | `void` | Stops active executions of the ability.                  |
| `unlock_ability_by_tag(tag)`                | `void` | Unlocks an ability from the catalog for use.             |
| `lock_ability_by_tag(tag)`                  | `void` | Blocks an ability from being used.                       |
| `add_tag(tag)`                              | `void` | Adds a tag and triggers **Ability Triggers**.            |
| `remove_tag(tag)`                           | `void` | Removes a tag and triggers **Ability Triggers**.         |

#### Effects

| Method                            | Return | Description                                         |
| :-------------------------------- | :----- | :-------------------------------------------------- |
| `apply_effect_spec_to_self(spec)` | `void` | Applies a payload directly to the actor.            |
| `has_active_effect_by_tag(tag)`   | `bool` | Checks for active modifier presence.                |
| `remove_effect_by_tag(tag)`       | `void` | Removes the effect and clears associated modifiers. |

#### Configuration

| Method                       | Return | Description                                                       |
| :--------------------------- | :----- | :---------------------------------------------------------------- |
| `apply_container(container)` | `void` | Initializes the ASC with the blueprint (attributes/abilities).    |
| `register_node(alias, node)` | `void` | Registers internal nodes for Cue lookup (e.g., "Muzzle", "Head"). |

---

### `ASDelivery` (New in v0.1.0)

Specialized in transporting effect payloads between components. Ideal for Projectiles and Areas of Effect (AoE).

- **Common Usage:** Attach to projectile scripts to handle collision and effect injection.
- **Methods:**
  - `deliver(target_asc)`: Injects all registered effects into the target ASC.
  - `add_effect(effect)`: Adds an effect resource to the delivery payload.
  - `set_source_component(asc)`: Defines the source author (for source-relative attribute calculations).

---

## 2. Resources (Blueprints)

### `ASAbility`

- **Triggers:** Reactive activation based on Tags.
  - `TRIGGER_ON_TAG_ADDED`: Activates when tag enters the ASC.
  - `TRIGGER_ON_TAG_REMOVED`: Activates when tag leaves (e.g., death ability when losing `state.alive`).
- **Cooldowns & Costs:** Automatically managed by the Core.

---

## 3. Runtime Objects (Specs)

### `ASAbilitySpec` / `ASEffectSpec`

Represent active instances. Note that the `target_node` parameter has been removed to prioritize **Node Registration** in the ASC and the **ASDelivery** system.

---

## 4. LimboAI Integration

`ASComponent` exposes simplified triggers for `BTAction`. In the Behavior Tree, use tags to manage flow:

- `BTCheckTag`: Verify state.
- `BTActivateAbility`: Trigger action via tag.
