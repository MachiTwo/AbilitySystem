# 🛠️ API Reference

> [!TIP]
> **Read this in other languages:**
> [**English**](API.md) | [**Português**](API.pt.md)

---

## 1. Core Systems

### `AbilitySystem` (Global Singleton)

Central authority for registering Tags and unique resource names.

- **Enums:**
  - `TagType`:
    - `TAG_TYPE_NAME` (0): Identity tags (Ability, Effect, Cue, Attribute).
    - `TAG_TYPE_CONDITIONAL` (1): State/status tags (state.stunned, state.dead, cooldown.attack).

- **Methods:**
  - `register_tag(tag, type, owner_id)`: Registers a new tag globally with a classification.
  - `unregister_tag(tag)`: Unregisters a tag from the global registry.
  - `is_tag_registered(tag) -> bool`: Checks if a tag exists in the central registry.
  - `get_tag_type(tag) -> TagType`: Returns the classification of a tag.
  - `get_registered_tags() -> StringName[]`: Lists all registered tags.
  - `get_registered_tags_of_type(type) -> StringName[]`: Lists tags of a specific classification.
  - `get_tag_owner(tag) -> int`: Returns the instance ID of the resource that registered this tag.
  - `register_resource_name(name, owner_id) -> bool`: Validates and registers resource name uniqueness.
  - `unregister_resource_name(name)`: Removes a resource name from the global registry.
  - `get_resource_name_owner(name) -> int`: Returns the instance ID of the owner of a specific name.
  - `tag_matches(tag, match_against, exact) -> bool` (static): Hierarchical tag comparison utility.
- **Signals:**
  - `tags_changed`: Emitted when the global tag list is modified.

---

### `AbilitySystemComponent` (ASC)

The heart of the system. Must be a direct child of `CharacterBody2D` or `CharacterBody3D`.

#### Abilities

| Method | Return | Description |
| :--- | :--- | :--- |
| `give_ability_by_resource(ability, level)` | `void` | Grants an ability to the actor via resource. |
| `give_ability_by_tag(tag, level)` | `void` | Grants an ability from the container catalog via tag. |
| `remove_ability_by_resource(ability)` | `void` | Revokes an ability via resource. |
| `remove_ability_by_tag(tag)` | `void` | Revokes all abilities matching the tag. |
| `remove_granted_ability_spec(spec)` | `void` | Removes a specific ability instance. |
| `can_activate_ability_by_tag(tag)` | `bool` | Checks if an ability can be activated **without** activating it. Emits `ability_failed` if it cannot. |
| `can_activate_ability_by_resource(ability)` | `bool` | Same, by resource. |
| `try_activate_ability_by_tag(tag)` | `bool` | Attempts to activate an ability. Returns `true` if successful. |
| `try_activate_ability_by_resource(ability)` | `bool` | Same, by resource. |
| `cancel_ability_by_tag(tag)` | `void` | Cancels all active abilities matching the tag. |
| `cancel_ability_by_resource(ability)` | `void` | Cancels the active ability via resource. |
| `cancel_all_abilities()` | `void` | Forcefully cancels all currently active abilities. |
| `get_granted_abilities_debug()` | `AbilitySystemAbilitySpec[]` | Returns all granted abilities (for debugging). |

#### Effects

| Method | Return | Description |
| :--- | :--- | :--- |
| `make_outgoing_spec(effect, level, target_node)` | `AbilitySystemEffectSpec` | Creates a validated effect instance from a resource. |
| `apply_effect_spec_to_self(spec)` | `void` | Applies a spec to the actor itself. |
| `apply_effect_spec_to_target(spec, target)` | `void` | Applies a spec to another actor's ASC. |
| `apply_effect_by_resource(effect, level, target_node)` | `void` | Applies directly, without checking requirements. |
| `apply_effect_by_tag(tag, level, target_node)` | `void` | Same, looking up the effect in the catalog by tag. |
| `can_activate_effect_by_resource(effect)` | `bool` | Checks activation requirements of an effect. |
| `can_activate_effect_by_tag(tag)` | `bool` | Same, by tag. |
| `try_activate_effect_by_resource(effect, level, target_node)` | `bool` | Applies if requirements are met. |
| `try_activate_effect_by_tag(tag, level, target_node)` | `bool` | Same, by tag. |
| `cancel_effect_by_tag(tag)` | `void` | Removes all active effects matching the tag. |
| `cancel_effect_by_resource(effect)` | `void` | Removes all active effects of the resource. |
| `remove_active_effect(spec)` | `void` | Explicitly removes an active effect instance. |
| `remove_effect_by_tag(tag)` | `void` | Removes all active effects matching the tag. |
| `remove_effect_by_resource(effect)` | `void` | Removes all active instances of the effect. |
| `has_active_effect_by_tag(tag)` | `bool` | Checks if there is any active effect with the tag. |
| `has_active_effect_by_resource(effect)` | `bool` | Checks if there is any active instance of the effect. |
| `get_active_effects_debug()` | `AbilitySystemEffectSpec[]` | Returns all active effects (for debugging). |

#### Attributes

| Method | Return | Description |
| :--- | :--- | :--- |
| `add_attribute_set(set)` | `void` | Adds a unique AttributeSet to the component (creates internal copy). |
| `get_attribute_value_by_tag(tag)` | `float` | Returns the **current** value (base + modifiers). |
| `get_attribute_value_by_resource(attribute)` | `float` | Same, by resource. |
| `get_attribute_base_value_by_tag(tag)` | `float` | Returns the **base** value without modifiers. |
| `get_attribute_base_value_by_resource(attribute)` | `float` | Same, by resource. |
| `set_attribute_base_value_by_tag(tag, value)` | `void` | Sets the base value directly (permanent change). |
| `set_attribute_base_value_by_resource(attribute, value)` | `void` | Same, by resource. |
| `has_attribute_by_tag(tag)` | `bool` | Checks if the component has the attribute. |
| `has_attribute_by_resource(attribute)` | `bool` | Same, by resource. |

#### Tags

| Method | Return | Description |
| :--- | :--- | :--- |
| `add_tag(tag)` | `void` | Adds a tag to the actor. |
| `remove_tag(tag)` | `void` | Removes a tag from the actor. |
| `has_tag(tag)` | `bool` | Checks for tag presence (with hierarchical support). |
| `get_owned_tags()` | `AbilitySystemTagSpec` | Returns the high-performance tag container. |

#### Cooldowns

| Method | Return | Description |
| :--- | :--- | :--- |
| `start_cooldown(ability_tag, duration, tags)` | `void` | Manually starts a cooldown with associated tags. |
| `is_on_cooldown(ability_tag)` | `bool` | Checks if a tag is on cooldown. |
| `get_cooldown_remaining(ability_tag)` | `float` | Returns the remaining time of a cooldown. |

#### Cues (Visual/Audio Feedback)

| Method | Return | Description |
| :--- | :--- | :--- |
| `register_cue_resource(cue)` | `void` | Registers a cue resource for runtime lookup. |
| `unregister_cue_resource(tag)` | `void` | Removes the registration of a cue by its tag. |
| `get_cue_resource(tag)` | `AbilitySystemCue` | Returns the cue associated with the tag. |
| `can_activate_cue_by_tag(tag)` | `bool` | Checks activation requirements of a cue by tag. |
| `can_activate_cue_by_resource(cue)` | `bool` | Same, by resource. |
| `try_activate_cue_by_tag(tag, data, target_node)` | `bool` | Activates a cue if requirements are met. |
| `try_activate_cue_by_resource(cue, data, target_node)` | `bool` | Same, by resource. |
| `cancel_cue_by_tag(tag)` | `void` | Cancels active cues with the tag. |
| `cancel_cue_by_resource(cue)` | `void` | Cancels the active cue. |
| `play_montage(name, target)` | `void` | Plays an animation on the target node or default node. |
| `is_montage_playing(name, target)` | `bool` | Checks if an animation is playing. |
| `play_sound(sound, target)` | `void` | Plays audio on the target node or default node. |

#### Configuration & Nodes

| Method | Return | Description |
| :--- | :--- | :--- |
| `apply_ability_container(container, level)` | `void` | Initializes the component with a full blueprint (AbilityContainer). |
| `set_animation_player(node)` / `get_animation_player()` | `void / Node` | Sets/gets the default node for animations (AnimationPlayer or AnimatedSprite). |
| `set_audio_player(node)` / `get_audio_player()` | `void / Node` | Sets/gets the default node for sounds. |
| `register_node(name, node)` | `void` | Registers a node with an alias for use by cues and effects. |
| `unregister_node(name)` | `void` | Removes the registration of a node alias. |
| `get_node_ptr(name)` | `Node` | Returns a registered node by its alias. |
| `register_task(task)` | `void` | Registers an active asynchronous task. |
| `tick(delta)` | `void` | Manually advances the internal simulation. Called automatically by `_process`. |
| `calculate_modifier_magnitude(spec, index)` | `float` | Calculates the magnitude of a modifier in a spec. |

#### Virtual Method

```gdscript
func _on_calculate_custom_magnitude(spec: AbilitySystemEffectSpec, index: int) -> float:
    pass
```

- `index = -1`: Effect duration.
- `index = -2`: Native ability cooldown.
- `index = -3` and below: Native ability costs (one per cost).

#### Signals

| Signal | Parameters | Emitted when |
| :--- | :--- | :--- |
| `ability_activated(spec)` | `AbilitySystemAbilitySpec` | An ability is successfully activated. |
| `ability_ended(spec, was_cancelled)` | `AbilitySystemAbilitySpec, bool` | An ability ends or is cancelled. |
| `ability_failed(ability_name, reason)` | `StringName, String` | An ability fails to activate, with a reason. |
| `attribute_changed(name, old, new)` | `StringName, float, float` | An attribute value changes. |
| `tag_changed(tag_name, is_present)` | `StringName, bool` | A tag is added or removed. |
| `tag_event_received(event_tag, data)` | `StringName, Dictionary` | A specific tag event is triggered. |
| `effect_applied(spec)` | `AbilitySystemEffectSpec` | An effect is successfully applied. |
| `effect_removed(spec)` | `AbilitySystemEffectSpec` | An active effect is removed. |
| `effect_failed(effect_name, reason)` | `StringName, String` | An effect fails to be applied. |
| `cue_failed(cue_name, reason)` | `StringName, String` | A cue fails to be activated. |
| `cooldown_started(ability_tag, duration)` | `StringName, float` | A cooldown starts. |
| `cooldown_ended(ability_tag)` | `StringName` | A cooldown reaches zero. |

---

## 2. Resources (Data Blueprints)

### `AbilitySystemAbility`

Defines the logic and data for a specific action.

- **Properties:** `ability_name`, `ability_tag`, `activation_required_tags`, `activation_blocked_tags`, `activation_owned_tags`, `effect`, `costs`, `cooldown_duration`, `cooldown_tags`, `cooldown_use_custom`, `costs_use_custom`, `cues`.
- **Virtual Methods (override in GDScript):**
  - `_on_can_activate_ability(owner, spec) -> bool`: Custom activation requirements.
  - `_on_activate_ability(owner, spec)`: Main ability logic.
  - `_on_end_ability(owner, spec)`: Cleanup on end.
- **Lifecycle Methods:**
  - `can_activate_ability(owner, spec) -> bool`: Validates tags, costs, and cooldown.
  - `activate_ability(owner, spec, target_node)`: Starts execution (applies costs and cooldown automatically).
  - `end_ability(owner, spec)`: Ends execution (clears `activation_owned_tags`).
  - `can_afford_costs(owner, spec) -> bool`: Checks if the actor has enough resources for the costs.
  - `apply_costs(owner, spec)`: Applies costs to the actor.
  - `add_cost(attribute, amount)`: Adds a native cost at runtime.
  - `remove_cost(attribute) -> bool`: Removes a native cost at runtime.

> [!NOTE]
> `activation_owned_tags` are automatically added to the ASC when the ability activates and removed when it ends. Use them to represent the ability's active state (e.g., `ability.skill.walk`).

---

### `AbilitySystemEffect`

Defines a package of attribute modifiers and associated tags.

- **Duration Policies** (`DurationPolicy`):
  - `POLICY_INSTANT` (0): Applied once and removed immediately.
  - `POLICY_DURATION` (1): Lasts for a defined duration.
  - `POLICY_INFINITE` (2): Lasts indefinitely until removed.
- **Stacking Policies** (`StackingPolicy`):
  - `STACK_NEW_INSTANCE` (0): Each application is independent.
  - `STACK_OVERRIDE` (1): New application replaces the old one (resets timer).
  - `STACK_INTENSITY` (2): Increases magnitude (adds to stack count).
  - `STACK_DURATION` (3): Extends the remaining duration.
- **Modifier Operations** (`ModifierOp`): `OP_ADD`, `OP_MULTIPLY`, `OP_DIVIDE`, `OP_OVERRIDE`.
- **Key Properties:** `effect_name`, `effect_tag`, `duration_policy`, `duration_magnitude`, `granted_tags`, `removed_tags`, `activation_required_tags`, `activation_blocked_tags`, `blocked_tags`, `use_custom_duration`, `period`, `execute_periodic_tick_on_application`, `stacking_policy`, `cues`.
- **Method:** `add_modifier(attribute, operation, magnitude, use_custom_magnitude)`.

---

### `AbilitySystemAttributeSet`

Attribute container for an actor. A unique instance is created per actor in `apply_ability_container`.

- **Properties:** `attribute_definitions`, `granted_abilities`.
- **Methods:** `add_attribute_definition(attribute)`, `remove_attribute_definition(name)`, `get_attribute_definition(name)`, `get_attribute_list()`, `add_modifier(name, value, type)`, `remove_modifier(name, value, type)`, `get_attribute_current_value(name)`, `get_attribute_base_value(name)`, `set_attribute_base_value(name, value)`, `set_attribute_current_value(name, value)`, `has_attribute(name)`, `reset_current_values()`.
- **Signals:** `attribute_changed(name, old, new)`, `attribute_pre_change(name, old, new)`.

---

### `AbilitySystemAbilityContainer` (Archetype Blueprint)

Full blueprint for an actor archetype. Assigned to the ASC's `ability_container` property.

- **Properties:** `abilities`, `attribute_set`, `effects`, `cues`.

> [!IMPORTANT]
> All effects and cues the ASC can emit **must** be registered in its `AbilityContainer`. The system validates this in `make_outgoing_spec`.

---

## 3. Runtime Objects (Specs)

### `AbilitySystemAbilitySpec`

A granted ability instance. Stores runtime state.

| Method | Return | Description |
| :--- | :--- | :--- |
| `get_ability()` | `AbilitySystemAbility` | Returns the ability resource. |
| `get_level()` | `int` | Returns the level of this instance. |
| `get_is_active()` | `bool` | Returns `true` if the ability is running. |
| `get_owner()` | `AbilitySystemComponent` | Returns the ASC that owns this spec. |
| `get_cooldown_duration()` | `float` | Returns the base cooldown duration. |
| `get_cooldown_remaining()` | `float` | Returns the remaining cooldown time on the owner ASC. |
| `is_on_cooldown()` | `bool` | Checks if the ability tag is currently on cooldown. |
| `get_cost_amount(attribute)` | `float` | Returns the cost for a specific attribute. |

---

### `AbilitySystemEffectSpec`

An active effect instance. Stores calculated duration, stacks, and magnitudes.

| Method | Return | Description |
| :--- | :--- | :--- |
| `get_effect()` | `AbilitySystemEffect` | Returns the effect resource. |
| `get_duration_remaining()` | `float` | Time remaining before the effect ends. |
| `get_total_duration()` | `float` | Initial calculated duration. |
| `get_stack_count()` | `int` | Current number of stacks. |
| `get_level()` | `float` | Power level of this instance. |
| `get_magnitude(name)` | `float` | Calculated modifier value for an attribute. |
| `get_source_component()` | `AbilitySystemComponent` | The ASC that applied this effect. |
| `get_target_component()` | `AbilitySystemComponent` | The destination ASC where the effect is applied. |
| `get_source_attribute_value(attribute)` | `float` | Value of an attribute on the source (for relative calculations). |
| `get_target_attribute_value(attribute)` | `float` | Value of an attribute on the target. |
| `get_period_timer()` | `float` | Time remaining until the next periodic tick. |
| `get_hit_position()` | `Variant` | Impact position (Vector2 or Vector3). |

---

### `AbilitySystemTagSpec`

High-performance container for actor state tags. Tags are reference-counted — a tag is only removed when its count reaches zero.

| Method | Return | Description |
| :--- | :--- | :--- |
| `add_tag(tag)` | `bool` | Adds the tag (or increments its count). Returns `true` on the first reference. |
| `remove_tag(tag)` | `bool` | Decrements the count. Returns `true` when the tag is actually removed. |
| `has_tag(tag, exact)` | `bool` | Checks for presence (hierarchical by default). |
| `has_all_tags(tags, exact)` | `bool` | Checks if all specified tags are present. |
| `has_any_tags(tags, exact)` | `bool` | Checks if any of the specified tags are present. |
| `get_all_tags()` | `StringName[]` | Returns all tags in the container. |
| `clear()` | `void` | Removes all tags. |

---

### `AbilitySystemCueSpec` (Cue Context)

Context package passed to cue virtual methods.

- **Properties:** `cue`, `effect_spec`, `extra_data`, `hit_position`, `magnitude`, `source_asc`, `target_asc`, `target_node`.
- **Methods:** `get_level()`, `set_level(level)`.

---

## 4. Extensibility

### `AbilitySystemTask` (Async Logic)

Used internally by abilities to manage time-based operations (waits, animations).

- **Static Methods:** `wait_delay(owner, delay) -> Task`, `play_montage(owner, animation_name) -> Task`.

---

### `AbilitySystemCue` (Feedback Base)

Base resource for audiovisual feedback. Use `AbilitySystemCueAnimation` or `AbilitySystemCueAudio` for common cases. Extend in GDScript for fully custom behavior.

- **Properties:** `cue_name`, `cue_tag`, `event_type` (`ON_EXECUTE`, `ON_ACTIVE`, `ON_REMOVE`), `node_name`.
- **Virtual Methods:** `_on_execute(spec)`, `_on_active(spec)`, `_on_remove(spec)`.

---

## 5. Editor Tools (Internal API)

> [!IMPORTANT]
> These tools require the plugin to be enabled in **Project Settings > Plugins**.

| Tool | Purpose |
| :--- | :--- |
| **`AbilitySystemTagsPanel`** | Panel in Project Settings to manage all hierarchical tags (SSOT). |
| **`AbilitySystemInspectorPlugin`** | Registers custom editors in the Godot Inspector. |
| **`AbilitySystemEditorPropertyName`** | Name field with real-time uniqueness validation. |
| **`AbilitySystemEditorPropertySelector`** | Tree selector for multi-tag properties. |
| **`AbilitySystemEditorPropertyTagSelector`** | Dropdown for single-tag selection. |
