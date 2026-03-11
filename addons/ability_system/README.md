# Ability System - Data-Driven Abilities for Godot 4.x

[![Godot Engine](https://img.shields.io/badge/Godot-4.6-blue?logo=godot-engine&logoColor=white)](https://godotengine.org)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> [!TIP]
> **Leia isto em outros idiomas / Read this in other languages:**
> [**Português**](README.pt.md) | [**English**](README.md)

**Ability System** is a powerful framework for creating modular combat, abilities, and attributes. Designed to scale from simple mechanics (jump, dash) to complex RPG systems with buffs, debuffs, and advanced damage calculations—all with high-performance C++ at its core.

---

## 📦 Installation

1. Download the latest `ability-system-plugin.zip` from [Releases](https://github.com/MachiTwo/AbilitySystemPlugin/releases/download/0.1.0-dev/ability-system-plugin.zip).
2. Extract and copy the `addons/ability_system` folder to your project's `addons/` directory.
3. Restart Godot and go to **Project > Project Settings > Plugins** to enable the "Ability System" plugin. This activates the Tag Editor and custom Inspector features.

---

## 🛠️ Dual Build Architecture

This project is uniquely engineered to support **Dual Compilation**, serving both the core engine development and the plugin ecosystem:

- **Zyris Engine (Native):** This framework is a core native component of the **Zyris Engine**. Development in this repository allows for centralized automation and rigorous validation. Once stabilized, the code is officially integrated into the Zyris Engine's `master` branch.
- **GDExtension (Plugin):** A dynamic library version for standard Godot 4.x projects. It provides 100% logic parity without requiring engine recompilation, ideal for projects using the official Godot distribution.

A unified C++ codebase powers both versions, using a robust preprocessor system (`#ifdef ABILITY_SYSTEM_GDEXTENSION`) to manage integration differences while maintaining a single source of truth for the logic.

---

## 🏗️ Quick Start Guide

Transform your gameplay into data with these core steps. We will build a system where social interactions and combat coexist.

### 1. Access the Tag Manager

The Tag Manager is the heart of your project's vocabulary. Its location depends on your version:

- **GDExtension (Plugin):** Look for the **Ability System Tags** tab in the **Bottom Panel** (next to Output/Debugger).
- **Zyris Engine (Native):** Go to **Project > Project Settings** and look for the **Ability System Tags** tab after the Input Map.

### 2. Define the Vocabulary (Tags)

Define hierarchical identifiers. Dots create branches that the logic can query:

- `ability.social.talk`: The base for all conversations.
- `state.emotional.angry`: A state that might block social interactions.
- `state.stun.frozen`: A combat state.

> [!TIP]
> **Broad Logic:** Checking for `state.emotional` will return true if the actor has `state.emotional.angry` or `state.emotional.happy`.

### 3. Create the Stat Schema (AttributeSet)

Create an **AttributeSet** resource (e.g., `RPGAttributes.tres`).

- Add combat stats: `Health`, `Mana`.
- Add social stats: `Charisma`, `Patience`.
- Set Min (0), Max (100), and Base (100) values.
- The `AbilitySystemComponent` (ASC) will **deep-duplicate** this set on spawn, ensuring unique health/patience pools for every NPC.

### 4. Design Social Abilities (Dialogues)

Create an **Ability** (e.g., `TalkAbility.tres`):

- **Ability Tag:** `ability.social.talk`.
- **Activation Blocked Tags:** Add `state.emotional.angry`. An angry character cannot start a polite conversation!
- **Owned Tags:** Add `state.busy.talking`. This tag is granted *while* the ability is active and can be used to block other actions like "Run".

### 5. Create Emotional Effects

Create an **Effect** (e.g., `EffectAngry.tres`):

- **Duration Policy:** `Infinite` (Passive) or `Duration` (Temporary).
- **Modifiers:** Add a modifier to `Charisma` (Multiply by 0.5) and `Attack` (Add 20).
- **Granted Tags:** `state.emotional.angry`.

### 6. Build the Archetype (AbilityContainer)

Create an **AbilityContainer** (e.g., `VillagerArchetype.tres`). This is your "NPC Blueprint":

- Assign the `AttributeSet`.
- Add the `Talk` ability to the catalog.
- Add default effects (e.g., a "Neutral" state).

### 7. Initialize the Actor (ASC)

Add the `AbilitySystemComponent` node to your CharacterBody and assign the **AbilityContainer** in the Inspector.
Register your feedback nodes (AnimationPlayer/Audio) in script so the system can trigger **Cues** automatically.

### 8. Handling Complex Dialogue Logic

Use **Tasks** to handle async timing or waiting for events:

```gdscript
# Inside a custom Ability script or triggered via ASC
func _on_activate_ability(owner, spec):
    # 1. Play a dialogue animation
    owner.play_montage("greet")
    
    # 2. Wait for a delay or an external "DialogueFinished" tag event
    var task = AbilitySystemTask.wait_delay(owner, 2.0)
    await task.completed
    
    # 3. End ability manually if needed
    owner.end_ability(spec)
```

### 9. Combat vs Social Interaction

The system handles both seamlessly. Applying a "Frozen" effect will automatically block the "Talk" ability if you add `state.stun` to the ability's blocked tags list.

```gdscript
func interact_with_npc(npc: AbilitySystemComponent):
 if npc.can_activate_ability_by_tag(&"ability.social.talk"):
  npc.try_activate_ability_by_tag(&"ability.social.talk")
 else:
  print("The NPC is too angry or stunned to talk!")
```

### 10. Handling State Changes (Signals)

The ASC notifies your game logic when significant changes occur:

```gdscript
func _ready():
 asc.tag_changed.connect(_on_tag_changed)

func _on_tag_changed(tag: StringName, is_present: bool):
 if tag == &"state.emotional.angry" and is_present:
  $Sprite.modulate = Color.RED # Visual feedback for emotion change
```

---

## 📚 Reference

### 🧠 Core Components

| Component | Purpose | Key Features |
| :--- | :--- | :--- |
| **AbilitySystem** | Global Coordinator. | Central Tag Registry, Project Settings integration. |
| **AbilitySystemComponent** | Actor's Processor (ASC). | Grants/Activates abilities, manages effects, triggers Cues. |

### ⚙️ Resources

| Resource | Purpose | Key Features |
| :--- | :--- | :--- |
| **Ability** | Logic of an action. | Native Costs, Cooldowns, and Activation Tags. |
| **Effect** | Modification package. | Instant damage, timed buffs, or infinite passives. |
| **AttributeSet** | Stat container. | Manages collections of attributes. Deep-duplicated per actor. |
| **Attribute** | Stat definition. | Individual HP, Mana, XP schema with clamping. |
| **AbilityContainer** | Archetype Blueprint. | Catalog of allowed abilities, effects, and starting stats. |
| **Task** | Async logic. | Handles waits, delays, and animation timing in abilities. |
| **Cue** | Feedback Base. | Event activation logic for audiovisual effects. |
| **CueAnimation** | Animation Feedback. | Specialized for playing montages on actors. |
| **CueAudio** | Audio Feedback. | Specialized for playing spatial or global sound. |

### 🚀 Runtime Objects (Specs)

| Spec | Purpose | Key Data |
| :--- | :--- | :--- |
| **AbilitySpec** | Ability instance. | Level, active state, runtime query methods. |
| **EffectSpec** | Effect instance. | Remaining duration, stack count, hit position (Variant). |
| **CueSpec** | Feedback context. | Triggering magnitude, source/target references, hit data. |
| **TagSpec** | Tag Management. | High-performance container for actor state tags. |

---

Developed with ❤️ by **MachiTwo**
