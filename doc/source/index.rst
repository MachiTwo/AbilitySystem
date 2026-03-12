Ability System Documentation
==========================

**Ability System** is a powerful and flexible C++ module for **Godot Engine 4** designed to handle complex game abilities, attributes, and tags. Inspired by robust architectures, it provides a data-driven approach to character skills, status effects, and reactive logic.

Features
--------

*   **Attribute Sets**: Define and manage numeric values like Health, Mana, or Strength with built-in support for modifiers and clamping.
*   **Tags**: A hierarchical tagging system to categorize and query game state efficiently.
*   **Abilities**: Reusable logic for skills, spells, or any action that actors can perform.
*   **Package System**: Bundle effects and visual cues for easy delivery via projectiles or direct application.
*   **Wait Tasks**: Async-like execution flow for abilities using Godot nodes.

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: Getting Started

   getting-started/introduction
   getting-started/installation

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: Core Concepts

   concepts/tags
   concepts/attributes
   concepts/abilities
   concepts/effects

.. toctree::
   :hidden:
   :maxdepth: 1
   :caption: Class Reference
   :glob:

   classes/class_*
