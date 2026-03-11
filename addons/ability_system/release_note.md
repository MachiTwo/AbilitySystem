# Ability System

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**Português**](release_note.pt.md) | [**English**](release_note.md)

## 1. Context and Philosophy

The **Ability System** for Godot Engine is a high-performance native framework designed to handle combat logic, abilities, and attributes in a modular and data-driven way.

Unlike pure script implementations, the Ability System focuses on:

- **Raw Performance:** Modifier calculations and tag processing in C++.
- **Data-Driven:** Abilities and effects are `Resource` types, allowing designers to create mechanics without touching code.
- **Decoupling:** The logic of "how damage is calculated" is separated from "who causes the damage".

## 2. Core Architecture (C++)

The system is organized into subfolders within `src/` (or `modules/ability_system/` in engine builds) for better maintainability:

- `core/`: Central Singleton, Tags, and Specs.
- `resources/`: Attribute, Ability, and Effect definitions.
- `scene/`: Scene Components.

### 2.0 AbilitySystem (Singleton)

The true central point of the system. Managed as an Engine Singleton [Object].

- **Global Tag Registry:** Stores all tags registered in the project.
- **Project Settings:** Integrated into project settings for persistent tag definitions.

### 2.1 AbilitySystemComponent (ASC)

The actor's "brain." Must be attached to a `CharacterBody2D/3D` [Node].

- **Registration API (Manual Setup):**
  - `set_animation_player(Node)`: Sets the default slot for animations (accepts `AnimationPlayer` or `AnimatedSprite`).
  - `set_audio_player(Node)`: Sets the default slot for audio (accepts `AudioStreamPlayer`, `2D` or `3D`).
  - `register_node(StringName name, Node)`: Registers a named node for specific Cues.
- **Feedback API (Execution):**
  - `play_montage(StringName anim, Node target)`: Plays an animation. If `target` is null, uses the default slot.
  - `play_sound(AudioStream sound, Node target)`: Plays audio. If `target` is null, uses the default slot.
- **Main API (Execution):**
  - `give_ability(AbilitySystemAbility ability)`: Registers a new ability.
  - `try_activate_ability(Variant ability_or_tag)`: Attempts to execute the passed ability (accepts resource, tag, or name).
  - `cancel_ability(Variant ability_or_tag)`: Ends the execution of an active ability.
  - `apply_effect_spec_to_self(AbilitySystemEffectSpec spec)`: Applies modifiers.
- **Position Support:** All position parameters (hits, cues) now use `Variant`, supporting both `Vector2` and `Vector3`.
- **Reactive Signals:**
  - `attribute_changed(name, old, new)`: For UI and Feedback.
  - `tag_changed(name, present)`: For visual state changes.

> [!NOTE]
> The `play_montage` and `play_sound` methods are public and can be used via script, but the recommended flow is **Data-Driven**, where Abilities and Effects trigger these methods internally through Cues.

### 2.2 AbilitySystemAbilityContainer (Resource)

Defines the **permission contract** (archetype) of an entity, cataloging everything it is capable of possessing or performing.

- **Purpose:** Acts as a "blueprint" or "fixed template" for races, classes, or enemy types. It defines the scope of possibilities, allowing the game to manage progressive unlocking (via level or class) safely.
- **Content (Catalog):**
  - `abilities`: List of abilities the entity **can** learn or equip (access catalog).
  - `effects`: Catalog of effects, passives, and **attack instances** allowed (e.g., poison, shields, buffs), which the archetype is authorized to carry or instantiate.
  - `attributes`: Allowed attributes and their definitions for this archetype.
  - `tags`: Identity and permission tags inherent to the archetype.
  - `cues`: Mapping of audiovisual feedback available to the archetype.

### 2.3 AbilitySystemAttribute

Defines an individual attribute with a schema, limits, and validation.

- **Purpose:** Definition Resource for statistics (HP, Mana, Stamina).
- **Properties:**
  - `name`: Unique identifier for the attribute.
  - `base_value`: Initial base value.
  - `min_value`/`max_value`: Limits for automatic validation.
- **Validation:** `is_valid_value()` and `clamp_value()` methods to ensure consistency.
- **Usage:** Configurable in the Editor, reusable across multiple characters.

### 2.4 AbilitySystemAttributeSet

Container that manages collections of AbilitySystemAttribute Resources.

- **Modularity:** Supports multiple sets per AbilitySystemComponent.
- **Mandatory Validation:** Requires attributes to be defined as Resources before use.
- **Resource-based API:**
  - `add_attribute_definition(AbilitySystemAttribute)`: Registers definition.
  - `get_attribute_definition(name)`: Gets definition Resource.
  - `set_attribute_base_value(name, value)`: Manipulates with automatic validation.
- **Runtime Performance:** Maintains an internal HashMap for values (base/current) with Resource-based validation.
- **Signals:** `attribute_changed`, `attribute_pre_change` for reactivity.

### 2.5 AbilitySystemAbility (Resource)

Defines the logic of "what happens" and primarily "what this ability is".

- **Identity vs Behavior:** An ability in the Ability System is defined primarily by its unique `Ability Tag`, which acts as its "ID Card". The other Tag fields (Activation Arrays) do not dictate _who_ the Ability is, but rather its _behavior_ regarding the character and scenario.
- **Polymorphism Hierarchy:** Due to the Tag format (e.g., `ability.spell.fire.fireball`), the Engine easily handles groups or classifiers without using a Tag Array in the Identity. if the player tries to activate `ability.spell` on the ASC, all sub-magic skills respond. If specificity is needed, call it fully: `ability.spell.fire`.

**State Control and Condition Properties (The "Activation Tags"):**

- **`ability_tag` (Unique Registry):** Main and singular tag forming identity and filtering (`StringName`). Example: `ability.movement.dodge`
- **`activation_owned_tags` (Temporary Buff):** Tag Array. When the ability starts, it stamps this entire group on the Character and removes them once the lifecycle ends. Dictates what your Character "becomes" (e.g., during Dodge, gains _state.immune.damage_ and _state.animation.rolling_).
- **`activation_required_tags` (Requirements):** Tag Array. The ability refuses to activate if invoked by a Player who does not possess all these listed tags (Can require _state.combat.advantage_ or something native like _state.in_air_).
- **`activation_blocked_tags` (CC/Block):** Tag Array. The ability refuses to activate if the user tries to use it while suffering from any of these Tags (The famous Stun, Silence, and Root windows - _state.debuff.stun_).

**Other Core Structures:**

- **Lifecycle:** `can_activate` -> `activate_ability` -> `end_ability`.
- **Audiovisual Encapsulation:** Contains a list of `cues` executed automatically.
- **Physical and Logical Restrictors:**
  - **Native Costs:** Array of Dictionaries `{attribute: StringName, amount: float}` applied directly to the owner's attributes.
  - **Native Cooldown:** `cooldown_duration` and `cooldown_tags` managed automatically by the ASC.
  - **Custom Logic:** `cooldown_use_custom` and `costs_use_custom` flags to trigger script calculations in the ASC via `_on_calculate_custom_magnitude` (Index -2 for cooldown, -3 and below for costs).
- **Virtual Methods:** `_can_activate_ability()`, `_activate_ability()`, `_on_end_ability()`.

### 2.6 AbilitySystemEffect (Resource)

Defines the rules and the "package" for altering a character:

- **Origin Logic:** Registered in the `AbilitySystemAbilityContainer` of who **causes** the effect. A Sniper has the "shot effect" in their catalog; the enemy just receives the impact calculation via code.
- **Variety:**
  - **Single Damage** (Sniper) vs **Continuous Damage/DOT** (Poison, Fire Area).
  - **Status Buffs** (Strength, Protection Shields).
- **Duration Policies:**
  - `INSTANT` — e.g., A sniper shot (single damage registered on the weapon/character).
  - `DURATION` — e.g., A poisoned arrow or a temporary protective shield.
  - `INFINITE` — e.g., A strength enchantment or permanent aura.
- **Modifiers:** `ADD`, `MULTIPLY`, `DIVIDE`, and `OVERRIDE` operations.
- **Costs:** Activation costs (Mana, Stamina, etc.).
- **Cooldowns:** Recharge time between uses.
- **Tags:** `granted_tags`, `removed_tags`, `blocked_tags`.
- **Cues:** `cue_tags` to trigger visual/audio feedback.
- **Custom Duration:** `use_custom_duration` flag to trigger script calculation in the ASC via `_on_calculate_custom_magnitude` (Index -1).
- **Custom Magnitude:** handled via virtual method `_on_calculate_custom_magnitude` in `AbilitySystemComponent` for any modifier marked as `use_custom_magnitude`.

### 2.7 AbilitySystemCue (Resource)

Event activation and synchronization system (animations, sounds) triggered by effects or abilities.

- **Function:** System for activation and synchronization of gameplay events.
- **Dependency:** Requires specialized classes (CueAnimation, CueAudio, CueParticles [planned]) for actual functionality.
- **Trigger:** Executed when effects are applied/removed or abilities are activated.
- **Types:** `ON_EXECUTE` (instant), `ON_ACTIVE` (while active), `ON_REMOVE` (on removal).
- **Architecture:** Provides base structure but does not implement specific functionality.
- **Virtual Methods:** `_on_execute()`, `_on_active()`, `_on_remove()` for script customization.
- **Usage:** Inheritance to create specialized cues or customization via GDScript/C#.

#### 2.7.1 AbilitySystemCueAnimation (Resource)

Specialized in playing animations.

- **Property:** `animation_name` (StringName) - Name of the animation.
- **Property:** `node_name` (StringName) - Name of the target node registered in the ASC (optional).
- **Execution:** The system internally calls `asc->play_montage(animation_name, target)`. If `node_name` is empty, it uses the ASC's default `AnimationPlayer`.

#### 2.7.2 AbilitySystemCueAudio (Resource)

Specialized in playing audio.

- **Property:** `audio_stream` (Ref AudioStream) - Audio resource.
- **Property:** `node_name` (StringName) - Name of the target node registered in the ASC (optional).
- **Execution:** The system internally calls `asc->play_sound(audio_stream, target)`. If `node_name` is empty, it uses the ASC's default `AudioStreamPlayer`.

**Flow Note:** Play Montage and Play Sound are the "final point" of the system's feedback execution. Although public in the ASC, their use via Cues (Data-Driven) is preferred to keep logic decoupled from animation/sound code.

### 2.8 AbilitySystemTag

Unique identifier for game states [RefCounted].

- **Hierarchical:** e.g., `state.debuff.poison`, `ability.fireball`.
- **Optimized:** Pure `StringName` with direct storage in `HashSet` for maximum performance.
- **Global:** Centrally managed by the `AbilitySystem` singleton.
- **Strict-Mode:** in the editor, selectors ensure only tags registered in the `AbilitySystemTagsPanel` are used.

### 2.9 AbilitySystemTagSpec

High-performance container for tag operations and queries [RefCounted].

- **Management:** `add_tag()`, `remove_tag()` with reference counting.
- **Queries:** `has_tag()`, `has_any_tags()`, `has_all_tags()`.
- **Matching:** Support for hierarchical matching (`exact` parameter).
- **Performance:** Optimized structure for high-frequency operations.

### 2.10 AbilitySystemTask

Base for asynchronous tasks in abilities [RefCounted].

- **Types:** `TASK_WAIT_DELAY`, `TASK_PLAY_MONTAGE`, `TASK_WAIT_EVENT`.
- **Static Methods:** `wait_delay()`, `play_montage()`.
- **Signals:** `completed` for asynchronous coordination.
- **Extensible:** Inheritance for custom tasks.

## 3. Runtime System (Specs)

### 3.1 AbilitySystemAbilitySpec

Runtime instance of an `AbilitySystemAbility` granted to the ASC.

- **Data:** Level, active state, reference to Resource.
- **Methods:** `get_ability()`, `get_is_active()`, `get_level()`.
- **Cycle:** Managed by the `AbilitySystemComponent`.

### 3.2 AbilitySystemEffectSpec

Runtime instance of an applied `AbilitySystemEffect`.

- **Data:** Remaining duration, dynamic magnitudes, level.
- **Context:** Source/target ASCs, magnitude calculations.
- **Methods:** `get_duration_remaining()`, `set_magnitude()`, `calculate_modifier_magnitude()`.

### 3.3 AbilitySystemCueSpec

Runtime execution context for an `AbilitySystemCue`.

- **Data:** Cue resource, triggering effect spec, calculated magnitude.
- **Context:** Source/target ASCs, magnitude calculations.
- **Methods:** `get_source_asc()`, `get_magnitude()`, `get_effect_spec()`.
- **Usage:** Passed to Cue's GDVirtual callbacks (`_on_execute`, `_on_active`, `_on_remove`).

**Example (GDScript):**

```gdscript
extends AbilitySystemCue

func _on_execute(spec: AbilitySystemCueSpec):
  var damage = spec.magnitude
  var attacker = spec.source_asc
  # Spawn floating damage number at attacker's position, etc.
  print("Cue triggered! Damage = ", damage)
```

- **Context:** Source ASC, target ASC, extra data.
- **Usage:** Passed to Cue callbacks with full context.

## 4. Ability System Tags

**Ability System Tags** are optimized hierarchical `StringName` IDs.

- **Example:** `state.buff.speed`, `state.debuff.stun`.
- **Registry:** Managed by the `AbilitySystem` singleton.
- **Matching:** Hierarchical matching support via `AbilitySystem.tag_matches()`.
- **Performance:** Optimized for high-frequency queries without class overhead.
- **Workflow:** Register tags in Project Settings and use automatic selectors in Resources.

## 5. Editor Tools

### 5.1 AbilitySystemEditorPlugin

Native integration with the Godot **Inspector** for UI automation:

- **Unified Selectors:** Any property ending in `_tag` (StringName) or `_tags` (Array of StringName) automatically receives a dropdown selector.
- **Real-time Sync:** The selector connects to the `AbilitySystem`'s `tags_changed` signal, ensuring new entries in the Tag Editor appear instantly in the Inspector without restarting the engine.
- **Visual Validation:** Selected tags that no longer exist in the global registry are marked with an error icon and "(Missing)" warning.

### 5.2 AbilitySystemTagsPanel (Tag Registry)

Located at **Bottom Panel > Ability System Tags**, it is the system's central source of truth:

- **Centralized Management:** Add or remove global tags to be used throughout the project.
- **Secure Persistence:** Tags are saved in project settings (`project.godot`), ensuring the design team has the same validated catalog.
- **Error Prevention:** Avoids "Magic Strings" scattered throughout the project, forcing a registry-based workflow.

### 5.3 AbilitySystemEditorPropertySelector (Array Property Integration)

Pure C++ custom UI component (inheriting from `EditorProperty`) that intercedes in the Engine's inspector, replacing the default Array editor for properties ending in `_tags`.

- **Dynamic Multi-Tag Selection:** Offers a button that opens a floating dialog. The layout features a `Tree` listing all registered tags and native checkboxes, supporting multiple insertions at once.
- **Real-time Filter:** Has a dynamic search field at the top of the list that masks and filters tags (e.g., typing "poison" brings up all associated sub-tags for quick tagging).

### 5.4 AbilitySystemEditorPropertyTagSelector (Single Tag Integration)

Pure C++ custom UI component (inheriting from `EditorProperty`) for single tag properties ending in `_tag`.

- **Dropdown Selection:** Provides an `OptionButton` with all registered tags plus a "(None)" option.
- **Real-time Updates:** Automatically refreshes when tags are added/removed from the global registry.

### 5.5 AbilitySystemEditorPropertyName (Name Validation)

Custom editor for properties ending in `_name` that ensures uniqueness across the project.

- **Uniqueness Check:** Validates names against existing resources of the same type.
- **Visual Feedback:** Shows warning icons for duplicate names.

### 5.6 AbilitySystemInspectorPlugin (Property Parser)

Core plugin that intercepts property parsing and routes to appropriate custom editors.

- **Pattern Matching:** Detects `_tag`, `_tags`, and `_name` property patterns.
- **Editor Registration:** Automatically registers the correct custom editor for each property type.
- **Constant Visual Summary:** keeps its interface clean when collapsed, directly displaying selection stats (e.g., "4 Tags Selected" or "(Empty Tags)").

## 6. Signals and Events

### 6.1 AbilitySystemComponent Signals

- **`ability_activated(spec)`**: Ability started.
- **`ability_ended(spec, was_cancelled)`**: Ability ended.
- **`ability_failed(name, reason)`**: Failed to activate.
- **`attribute_changed(name, old, new)`**: Attribute changed.
- **`tag_changed(name, present)`**: Tag added/removed.
- **`effect_applied(spec)`**: Effect applied.
- **`effect_removed(spec)`**: Effect removed.
- **`tag_event_received(tag, data)`**: Generic event triggered by Cues or manually.
- **`cooldown_started(tag, duration)`**: Cooldown started.
- **`cooldown_ended(tag)`**: Cooldown ended.

### 6.2 AbilitySystemAttributeSet Signals

- **`attribute_changed(attribute_name, old_value, new_value)`**: Triggered on any change.
- **`attribute_pre_change(attribute_name, old_value, new_value)`**: Before the change (can be cancelled).
- **Usage:** Update UI (health bars), logic triggers (death when HP = 0).

---

## 🎮 Complete Tutorial: Building a 2D Action RPG Player

This tutorial walks through creating a complete 2D player character using the Ability System Plugin, based on our demo project.

### Step 1: Create Player Attributes

Create `player_attribute_set.tres`:

```gdscript
extends AbilitySystemAttributeSet

func _init():
  # Health attribute
  var health = AbilitySystemAttribute.new()
  health.attribute_name = &"Health"
  health.base_value = 100.0
  health.min_value = 0.0
  health.max_value = 100.0
  add_attribute(health)

  # Stamina attribute (for abilities)
  var stamina = AbilitySystemAttribute.new()
  stamina.attribute_name = &"Stamina"
  stamina.base_value = 50.0
  stamina.min_value = 0.0
  stamina.max_value = 50.0
  add_attribute(stamina)
```

### Step 2: Create Movement Abilities

Create `walk.tres` ability:

```gdscript
extends AbilitySystemAbility

func _init():
  ability_name = "Walk"
  ability_tag = &"ability.skill.walk"

  # Tags applied when activated
  activation_owned_tags = [&"ability.skill.walk"]

  # Block idle while walking
  activation_blocked_tags = [&"ability.skill.idle"]

  # Link to animation cue
  cues = [preload("res://resources/player/cues/walk_cue.tres")]
```

Create `idle.tres` ability:

```gdscript
extends AbilitySystemAbility

func _init():
  ability_name = "Idle"
  ability_tag = &"ability.skill.idle"

  # Tags applied when activated
  activation_owned_tags = [&"ability.skill.idle"]

  # Block walk while idling
  activation_blocked_tags = [&"ability.skill.walk"]

  # Link to animation cue
  cues = [preload("res://resources/player/cues/idle_cue.tres")]
```

### Step 3: Create Combat Abilities

Create `attack.tres` ability:

```gdscript
extends AbilitySystemAbility

func _init():
 ability_name = "Attack"
 ability_tag = &"ability.skill.attack"

 # Native cost (mana/stamina)
 add_cost(&"Stamina", -10.0)

 # Native cooldown
 cooldown_duration = 0.5
 cooldown_tags = [&"cooldown.attack"]

 # Damage effect for enemies
 effect = preload("res://resources/player/effects/attack_damage.tres")

 # Tags for state management
 activation_owned_tags = [&"state.attacking"]
 activation_blocked_tags = [&"state.stunned"]

 # Visual/audio feedback
 cues = [preload("res://resources/player/cues/attack_cue.tres")]
```

### Step 4: Create Animation Cues

Create `walk_cue.tres`:

```gdscript
extends AbilitySystemCueAnimation

func _init():
  cue_tag = &"cue.animation.walk"
  animation_name = "walk"
```

Create `attack_cue.tres`:

```gdscript
extends AbilitySystemCueAnimation

func _init():
  cue_tag = &"cue.animation.attack"
  animation_name = "attack"
```

### Step 5: Create Main Effects

Create `attack_damage.tres`:

```gdscript
extends AbilitySystemEffect

func _init():
 # Instant damage
 add_modifier(&"Health", MODIFIER_OP.ADD, -20.0)
```

### Step 6: Create Player Container

Create `player_ability_container.tres`:

```gdscript
extends AbilitySystemAbilityContainer

func _init():
  # Set attributes
  attribute_set = preload("res://resources/player/player_attribute_set.tres")

  # Add abilities
  abilities = [
    preload("res://resources/player/abilities/walk.tres"),
    preload("res://resources/player/abilities/idle.tres"),
    preload("res://resources/player/abilities/attack.tres")
  ]
```

### Step 7: Setup Player Scene

Create player script:

```gdscript
extends CharacterBody2D

const SPEED = 200.0

@onready var asc: AbilitySystemComponent = $AbilitySystemComponent
@onready var sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var attack_area: Area2D = $Area2D
@onready var audio_player: AudioStreamPlayer2D = $AudioStreamPlayer2D

var is_attacking = false
var is_dead = false

func _ready() -> void:
  # Setup audio feedback
  asc.set_audio_player(audio_player)

  # Apply player container (attributes + abilities)
  var container = preload("res://resources/player/player_ability_container.tres")
  asc.apply_ability_container(container)

  # Connect signals
  asc.attribute_changed.connect(_on_attribute_changed)
  asc.ability_activated.connect(_on_ability_activated)

func _physics_process(delta: float) -> void:
  if is_dead: return

  # Handle attack input
  if Input.is_action_just_pressed("attack") and not is_attacking:
    asc.try_activate_ability(&"ability.skill.attack")

  # Handle movement with ability system
  var direction := Input.get_vector("move_left", "move_right", "move_up", "move_down")

  if direction:
    # Try to activate walk ability if not already active
    if not asc.has_tag(&"ability.skill.walk"):
      asc.try_activate_ability(&"ability.skill.walk")

    # Move only if walk ability is active and not attacking
    if asc.has_tag(&"ability.skill.walk") and not is_attacking:
      velocity.x = direction.x * SPEED
      velocity.y = direction.y * SPEED
      sprite.flip_h = direction.x < 0
  else:
    # Try to activate idle when not moving
    if not asc.has_tag(&"ability.skill.idle"):
      asc.try_activate_ability(&"ability.skill.idle")

    velocity.x = move_toward(velocity.x, 0, SPEED)
    velocity.y = move_toward(velocity.y, 0, SPEED)

  move_and_slide()

func _on_attribute_changed(attr: StringName, old_val: float, new_val: float) -> void:
 match attr:
  &"Health":
   if new_val <= 0:
    die()
   update_health_bar(new_val)
  &"Stamina":
   update_stamina_bar(new_val)

func _on_ability_activated(spec: AbilitySystemAbilitySpec) -> void:
 match spec.get_ability().ability_tag:
  &"ability.skill.attack":
   is_attacking = true
   # Handle attack logic (damage enemies, etc.)
   handle_attack()

func die():
 is_dead = true
 # Play death animation, disable input, etc.

func handle_attack():
 # Check for enemies in attack area
 for body in attack_area.get_overlapping_bodies():
  if body.has_method("take_damage"):
   # Create damage effect spec (Supports Vector2 or Vector3 position)
   var damage_effect = preload("res://resources/player/effects/attack_damage.tres")
   var damage_spec = asc.make_outgoing_spec(damage_effect, 1.0, global_position)

   # Apply damage to enemy
   body.take_damage(damage_spec)
```

### Step 8: Setup Input Map

In Project Settings > Input Map, add:

- `move_left`: A key, Left arrow
- `move_right`: D key, Right arrow
- `move_up`: W key, Up arrow
- `move_down`: S key, Down arrow
- `attack`: Space key

### Result

You now have a complete 2D player character with:

- **Attribute-based health and stamina system**
- **Ability-driven movement** (walk/idle states)
- **Combat system** with stamina costs and cooldowns
- **Animation feedback** via Cue system
- **Extensible architecture** for adding more abilities

This demonstrates the core power of the Ability System: data-driven gameplay where mechanics are defined as resources rather than hardcoded logic.

---

_This document defines the official technical specification of the Ability System module._
