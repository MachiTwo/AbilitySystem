# Release Notes - v0.1.0 (Stable)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](release_note.md) | [**Português**](release_note.pt.md)

This version marks the stable release (v0.1.0) of the Ability System, consolidating the transition from the development version (v0.1.0.dev). The main focus of this release is API simplification, expansion of reactive features, and absolute stability guaranteed by testing and build infrastructure.

## 🚀 New Features and Changes (v0.1.0.dev -> v0.1.0)

- **Class Renaming (AS Prefix)**: All system classes now use the short `AS` prefix instead of the full `AbilitySystem` (e.g., `ASAbility`, `ASComponent`, `ASEffect`).
  - _Note: The main Singleton remains `AbilitySystem` for clarity and global access._
- **ASDelivery**: Introduction of a lightweight system for effect delivery, ideal for projectiles and Areas of Effect (AoE).
- **ASPackage**: A new reusable data container that bundles multiple effects and cues. It serves as the official payload for `ASDelivery`, allowing for complex effect transmission in a single resource.
- **Ability Triggers**: Abilities can now be triggered automatically through Tag changes (e.g., triggering "Hurt" when receiving the `state.hurt` tag).
- **LimboAI Integration**: Native compatibility with Behavior Trees to control agents using the Ability System.
- **Test Suite (100% Core)**: Complete coverage of core logic through unit tests (`doctest`), ensuring refactoring safety.

## 🛠️ Infrastructure & Polishing

- **SCons Build Optimization**: Transitioned to **MD5-based Cache Decider**, ensuring reliable incremental builds and CI/CD consistency by ignoring irrelevant timestamps.
- **Documentation Overhaul**: Completely rewritten `API.md` and `Tests.md` to provide comprehensive, action-oriented guidance for developers.
- **`target_node` Removal**: Drastic simplification of activation method signatures. The system now focuses on reactive flow, leaving spatial "target" logic to `ASDelivery`.
- **Bug Fix**: Resolved an edge-case where `Ability Triggers` could fail when using duplicated containers within the `ASComponent`.

## 📦 Installation

See the [README.md](README.md) for installation instructions.
