# BUSINESS RULES: LIMBOAI - GOVERNANCE CONTRACT

This document establishes the mandatory architectural boundaries and business rules for the integration and use of the **LimboAI** plugin (Behavior Trees and Hierarchical State Machines) within the project. Any implementation that violates these boundaries must be refactored immediately.

---

## 1. PHILOSOPHY AND RIGOROUS ENGINEERING

The project rejects **"Vibe-Coding"** (programming by intuition, luck, or blind AI suggestion). Every line of AI behavior logic must be intentional, deterministic, and testable.

### 1.1 Pair Programming and Governance

- **Radical Detachment from Code:** A behavior tree or state machine that fails indicates a poorly defined AI architecture. Correction occurs via context re-evaluation and documentation adjustment, never through "patching" tasks to skip logical steps.
- **SSOT (Single Source of Truth):** This file is the Iron Law of intelligent behavior. All new guidelines regarding AI and BTs must be documented here.
- **Language Policy:** Technical code, Task documentation, and comments MUST be in **English**.

---

## 2. THE AGENT'S MIND: BLACKBOARD

The Blackboard is the Agent's Memory mechanism. It is fundamental for sharing context between disconnected Tasks without forcing Strong Coupling.

### 2.1 Blackboard & BlackboardPlan (Data and Scope)

- **Role:** Storing and sharing global and local AI data during the mental cycle (`tick`).
- **Rule (Strict Typing):** Whenever possible, define a `BlackboardPlan` to delimit and type the options available to the agent, exporting the appropriate variables to the editor.
- **Rule (Variable Semantics):** Variables defined in the Blackboard must be treated as "Facts" perceived by the agent from the world.
- **Rule (Multi-Scope):** Leverage Blackboard scopes to share "faction data" or global environment info between different agents without using heavy Singletons, creating the illusion of a *Hive Mind* (Connected Minds).

---

## 3. THE EXECUTOR: BTPLAYER (BT ORCHESTRATOR)

The `BTPlayer` is the root Node that binds the `BehaviorTree` resource to the physical world (Scene Node).

- **Role:** It is the brain running the temporal execution (Tick).
- **Update Rules:**
  - The `update_mode` must align with the Ability System to simulate direct control. `UpdateMode::PHYSICS` ensures network synchronization and proper Rollbacks.
  - For deterministic multiplayer cases, `MANUAL` is preferred to granularly control when the AI's mental "Tick" happens within the native physics handler.
- **Sovereignty of the BehaviorTree:** The `BTPlayer` only processes the static decisions of a `.tres` Resource (`BehaviorTree`).

---

## 4. BEHAVIOR TREES (BT)

LimboAI models decisions through a hierarchy of actions with strict status codes.

### 4.1 The `Status` Cycle

Tasks must strictly return the following promise states:

- `FRESH`: Initializing.
- `RUNNING`: Pending, requires continuation in the next frame. Promotes `Blocking`.
- `SUCCESS`: Action finalized according to design.
- `FAILURE`: Condition not met or action interrupted/invalid.

### 4.2 BT Task Categories (Structuring)

1. **BTComposite (Routers):** `Selector`, `Sequence`, etc. **Rule:** They must never execute real game logic; they only manipulate tree flow.
2. **BTDecorator (Modifiers):** Inverters, Time Limiters. **Rule:** They operate only on the returns or pre-conditions of a *single* child node.
3. **BTCondition (Observers):** Querying Facts. **Rule:** Pure, fast nodes with "O(1)" complexity. Never modify game state here. Must return `SUCCESS` or `FAILURE` instantly. They primarily read from the `Blackboard`.
4. **BTAction (Active Action - Leaf):** Where the "dirty work" resides. Interacts with `ASComponent`, triggers animations, moves characters. **Rule:** Actions can be instantaneous or `RUNNING`. They are the only nodes authorized to modify the environment.

---

## 5. STATE MACHINES: LIMBOHSM and LIMBOSTATE

When logic requires event-driven persistent transitions instead of tick-by-tick evaluations, use Hierarchical State Machines (HSM).

- **LimboHSM:** The container for active states and the transition manager (Dispatching).
- **LimboState (States):** Can house `enter`, `exit`, and `update` logic. **Rule:** Use primarily for closed agent mechanics (e.g., being "Stunned" or managing contiguous primary Animations).
- **Hybridism (`BTState`):** When macro-logic needs rigidity (HSM) but micro-decisions require fluidity (BT). **Rule:** Wrap Behavior Trees inside States for maximum flexibility.

---

## 6. LIMBOAI + ABILITY SYSTEM INTEGRATION

To avoid "God Objects" in Actions and ensure correct usage of the Abilities and Tags machine, the following architectural bridges are established:

### 6.1 Conditions via Tags (BTCondition)

AIs must react to context by applying and reading `Tags`, never by querying arbitrary boolean variables on the character.

- Use Observer tasks that access the `ASComponent` and verify status via: `has_tag(...)`.
- *Example:* An AI should retreat if it possesses the `Debuff.Fear` tag.

### 6.2 Casting Abilities (BTAction)

The BTAction **must not** produce direct damage or projectiles unless it is itself the "trigger" logic. It must access the ASC and command:

- `try_activate_ability_by_tag(...)`.
  The task remains in `RUNNING` as long as the ability does not report its completion via signal (this promotes actions that wait for animations via the Ability System).

### 6.3 The Subordinate Agent

The AI is merely a remote "Controller" simulating a local player operating the levers of the `ASComponent`. LimboAI dictates *when to act*, and the Ability System dictates *if it is possible* and the resulting *effects*.

---

## 7. DEVELOPMENT CRITERIA (CUSTOM TASKS)

Creating new Tasks in C++ or GDScript requires strict code obedience:

1. **Use of Godot Virtuals:** Overriding must occur in the virtuals `_enter`, `_tick`, `_exit`, and `_setup`.
2. **Exported Properties:** When using variable attributes in tasks, prioritize Blackboard variables (`BlackboardPlan`) using macros/suffixes that bind with native editor variables (`_var`).
3. **Idempotency in Tick:** If logic in `update(delta)` returns `RUNNING`, ensure it does not initialize duplicate events every frame unless the intention is a continuity tick.
4. **Error Isolation:** Returning `FAILURE` in the log is not a fatal Godot error; it is tree flow control. Embrace `FAILURE` as a normal control path (e.g., Target Not Found).

---

## 8. VISUAL DEBUGGER

- It is mandatory to maintain tree execution in compliance so that the native LimboAI Visual Debugger tool remains traceable for detecting bottlenecks in AI choices. Create BTTasks with clear `get_custom_name()` results if the logic involving heavy dynamic instances.

---

Developed with ❤️ by **MachiTwo**
