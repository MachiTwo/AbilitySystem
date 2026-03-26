# BUSINESS RULES: ABILITY SYSTEM - GOVERNANCE CONTRACT

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](BUSINESS_RULES.md) | [**Português**](BUSINESS_RULES.pt.md)

This document establishes the architectural boundaries and mandatory business rules. Any implementation violating these limits must be refactored immediately.

---

## 1. PHILOSOPHY AND RIGOROUS ENGINEERING

The project rejects **"Vibe-Coding"** (programming by intuition or luck). Every line of business logic is treated as an industrial engineering commitment.

### 1.1 Pair Programming and Governance

- **Radical Code Detachment:** If code fails, the flaw lies in communication or architecture. Correction is made via dialogue and documentation adjustment, never manual patches.
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

### 2.1 Tags Golden Rules (v0.2.0 Security)

- **CONDITIONAL Type (Logic Requirement):** Only tags marked as `CONDITIONAL` are valid in requirement lists (`Required/Blocked`) for Abilities, Effects, or Cues. Attempting to use a Name or Event tag for logical blocking is a security violation.
- **NAME Type (Resource Identity):** Only `NAME` tags are valid for identifying and naming `ASAbility` or `ASEffect` resources. This ensures a state (e.g., `state.stunned`) is never confused with an ability (e.g., `ability.warrior.powerhit`).
- **EVENT Type (Frame Occurrence):** `EVENT` tags are transient and used exclusively for dispatching occurrences via `ASTagUtils`. They do not persist in the Singleton's RefCount.

### 2.2 The 3 Canonical Tag Types (Tag Types / ASTagTypes)

| Type          | Semantic Role                            | Usage Restriction                                                    |
| ------------- | ---------------------------------------- | -------------------------------------------------------------------- |
| `NAME`        | Static identity (Who I Am/What this is)  | **EXCLUSIVE** for Resource Identification (`ASAbility`/`ASEffect`).  |
| `CONDITIONAL` | Persistent gameplay state (How I Am)     | **EXCLUSIVE** for Logical Requirements (`Required`/`Blocked`).       |
| `EVENT`       | Instantaneous occurrence (What Happened) | **EXCLUSIVE** for event dispatching via `ASTagUtils` and `ASBridge`. |

---

## 3. THE SINGLETON: ABILITY SYSTEM & ASUTILS

- **Role:** The **Global Configuration API** and repository for atomic static logic.
- **ASUtils:** C++ namespace centralizing high-performance utility functions for tag manipulation and attribute calculations.
- **Business Rules:**
  - The Singleton is the only entity authorized to persist the tag list in `project.godot`.
  - Acts as a **Typing Validator**: Prevents wrong tag types from being used in restricted fields at edit-time (via Inspector).

### 2.3 Tag Type Structures and Implementation

The unified tag system is implemented through three main structures in `as_tag_types.h`, each providing type-safe tag manipulation with integrated validation and convenience methods.

#### 2.3.1 ASNameTag - Persistent Identity Tags

**Purpose**: Long-duration state identifiers that persist until explicitly removed
**Duration**: Until manual removal via `remove_tag()`
**Usage**: Character classes, persistent states, team affiliations

**Structure Definition:**

```cpp
struct ASNameTag : public ASTagBase {
    // Constructor
    ASNameTag(const StringName &p_name);

    // Factory Methods
    static ASNameTag create(const StringName &p_name);

    // Predefined Common Tags
    static ASNameTag STUNNED();        // "state.stunned"
    static ASNameTag DEAD();           // "state.dead"
    static ASNameTag INVISIBLE();      // "state.invisible"
    static ASNameTag WARRIOR();        // "class.warrior"
    static ASNameTag MAGE();           // "class.mage"
    static ASNameTag ARCHER();         // "class.archer"
    static ASNameTag TEAM_BLUE();      // "team.blue"
    static ASNameTag TEAM_RED();       // "team.red"
};
```

**Key Features:**

- **Type Safety**: Compile-time guarantee of tag type
- **Validation**: Automatic validation against AbilitySystem registry
- **Convenience**: Predefined common tags for frequent use
- **Inheritance**: Extends `ASTagBase` with common functionality

#### 2.3.2 ASConditionalTag - Requirement/Blocking Tags

**Purpose**: Ability/Effect requirements and blockers controlling permissions
**Duration**: Typically short-term, tied to specific conditions or effects
**Usage**: Ability prerequisites, damage immunity, permission checks

**Structure Definition:**

```cpp
struct ASConditionalTag : public ASTagBase {
    // Constructor
    ASConditionalTag(const StringName &p_name);

    // Factory Methods
    static ASConditionalTag create(const StringName &p_name);

    // Predefined Permission Tags
    static ASConditionalTag CAN_PARRY();      // "can.parried"
    static ASConditionalTag CAN_DODGE();       // "can.dodged"
    static ASConditionalTag CAN_INTERRUPT();   // "can.interrupted"

    // Predefined Immunity Tags
    static ASConditionalTag IMMUNE_FIRE();     // "immune.fire"
    static ASConditionalTag IMMUNE_POISON();   // "immune.poison"
    static ASConditionalTag IMMUNE_PHYSICAL(); // "immune.physical"

    // Predefined State Condition Tags
    static ASConditionalTag GROUNDED();        // "state.grounded"
    static ASConditionalTag FLYING();          // "state.flying"
    static ASConditionalTag STEALTHED();       // "state.stealthed"
};
```

**Key Features:**

- **Permission Control**: Fine-grained ability activation control
- **Immunity System**: Centralized handling of damage type immunity
- **State Conditions**: Environmental and positional state tracking
- **Runtime Validation**: Automatic check against current actor state

#### 2.3.3 ASEventTagTag - Event Dispatch Tags

**Purpose**: Event identifiers for the dispatch system with integrated helper methods
**Duration**: Instantaneous (events are transient, but history persists briefly)
**Usage**: Combat events, ability lifecycle, state transitions, weapon interactions

**Structure Definition:**

```cpp
struct ASEventTagTag : public ASTagBase {
    // Constructor
    ASEventTagTag(const StringName &p_name);

    // Factory Methods
    static ASEventTagTag create(const StringName &p_name);

    // Predetermined Combat Event Tags
    static ASEventTagTag DAMAGE_DEALT();        // "event.damage.dealt"
    static ASEventTagTag DAMAGE_TAKEN();        // "event.damage.taken"
    static ASEventTagTag DAMAGE_BLOCKED();      // "event.damage.blocked"
    static ASEventTagTag HEAL_RECEIVED();       // "event.heal.received"

    // Predetermined Ability Event Tags
    static ASEventTagTag ABILITY_ACTIVATED();   // "event.ability.activated"
    static ASEventTagTag ABILITY_FAILED();      // "event.ability.failed"
    static ASEventTagTag ABILITY_COOLDOWN_END(); // "event.ability.cooldown_end"
    static ASEventTagTag ABILITY_INTERRUPTED(); // "event.ability.interrupted"

    // Predetermined State Event Tags
    static ASEventTagTag STUN_BEGIN();          // "event.stun.begin"
    static ASEventTagTag STUN_END();            // "event.stun.end"
    static ASEventTagTag DEATH();               // "event.death"
    static ASEventTagTag RESPAWN();             // "event.respawn"

    // Predetermined Weapon Event Tags
    static ASEventTagTag WEAPON_HIT();          // "event.weapon.hit"
    static ASEventTagTag WEAPON_MISS();         // "event.weapon.miss"
    static ASEventTagTag WEAPON_CRITICAL();     // "event.weapon.critical"

    // Helper Methods
    void dispatch(Node *p_instigator, float p_magnitude = 0.0f,
                  const Dictionary &p_payload = Dictionary()) const;
    bool occurred_recently(Node *p_target, float p_lookback_sec = 1.0f) const;
};
```

**Key Features:**

- **Dispatch Integration**: Built-in capacity to dispatch events
- **Historical Queries**: Direct access to recent event occurrences
- **Comprehensive Coverage**: Predefined tags for all common game events
- **Type Safety**: Event type validation at compile time

### 2.4 ASTagUtils - Tag Type Utilities and Validation

The `ASTagUtils` namespace in `as_tag_types.cpp` provides comprehensive tag validation, type detection, and historical query capabilities.

#### 2.4.1 Validation and Type Detection

```cpp
namespace ASTagUtils {
    // Main Validation Functions
    bool validate_tag_type(const StringName &p_tag, ASTagType p_expected_type);
    ASTagType detect_tag_type(const StringName &p_tag);
    ASTagBase create_tag(const StringName &p_tag);

    // Pattern Recognition Functions
    bool is_state_tag(const StringName &p_tag);     // "state.*"
    bool is_class_tag(const StringName &p_tag);     // "class.*"
    bool is_team_tag(const StringName &p_tag);      // "team.*"
    bool is_event_tag(const StringName &p_tag);     // "event.*"
    bool is_immune_tag(const StringName &p_tag);    // "immune.*"
    bool is_can_tag(const StringName &p_tag);       // "can.*"
}
```

**Type Detection Algorithm:**

1. **Event Tags**: Start with "event." → `ASTagType::EVENT`
2. **Conditional Tags**: Start with "can.", "immune.", "state.grounded", "state.flying", "state.stealthed" → `ASTagType::CONDITIONAL`
3. **Name Tags**: All other patterns (state._, class._, team.\*, etc.) → `ASTagType::NAME`

#### 2.4.2 Historical Tracking Structures

Each tag type maintains its own historical buffer with 128-entry circular buffers:

```cpp
// Historical Entry Structures
struct ASNameTagHistoricalEntry {
    StringName tag_name;
    ObjectID target_id;
    double timestamp = 0.0;
    uint64_t tick_id = 0;
    bool added = true; // true for addition, false for removal

    // Helper Methods
    Node *get_target() const;
    void set_target(Node *p_node);
};

struct ASConditionalTagHistoricalEntry {
    // Same structure as ASNameTagHistoricalEntry
    // Used for CONDITIONAL tag changes
};

struct ASEventTagHistoricalEntry {
    ASEventTagData data;  // Full event payload
    uint64_t tick_id = 0;
};
```

#### 2.4.3 Historical Query APIs

**ASNameTag Historical Query API:**

```cpp
// Basic Queries
ASTagUtils::name_was_tag_added("state.stunned", target, 1.0f);
ASTagUtils::name_was_tag_removed("state.stunned", target, 1.0f);
ASTagUtils::name_had_tag("state.stunned", target, 1.0f);

// Data Retrieval
ASTagUtils::name_get_recent_additions(target, 1.0f);
ASTagUtils::name_get_recent_removals(target, 1.0f);
ASTagUtils::name_get_recent_changes(target, 1.0f);

// Counting Operations
ASTagUtils::name_count_additions("state.stunned", target, 1.0f);
ASTagUtils::name_count_removals("state.stunned", target, 1.0f);
```

**ASConditionalTag Historical Query API:**

```cpp
// Specialized for CONDITIONAL tag changes
ASTagUtils::cond_was_tag_added("immune.fire", target, 1.0f);
ASTagUtils::cond_was_tag_removed("immune.fire", target, 1.0f);
ASTagUtils::cond_had_tag("immune.fire", target, 1.0f);
```

**ASEventTag Historical Query API:**

```cpp
// Event-Specific Queries
ASTagUtils::event_did_occur("event.damage", target, 1.0f);
ASTagUtils::event_get_recent_events("event.damage", target, 1.0f);
ASTagUtils::event_get_all_recent_events(target, 1.0f);

// Event Data Access
ASTagUtils::event_get_last_data("event.damage", target);
ASTagUtils::event_get_last_magnitude("event.damage", target);
ASTagUtils::event_get_last_instigator("event.damage", target);

// Counting Operations
ASTagUtils::event_count_occurrences("event.damage", target, 1.0f);
```

**Unified Historical Query API:**

```cpp
// Cross-Type Queries
ASTagUtils::history_was_tag_present("state.stunned", target, 1.0f);
ASTagUtils::history_get_tag_history("state.stunned", target, 1.0f);
ASTagUtils::history_get_all_changes(target, 1.0f);

// Debug Utilities
ASTagUtils::history_dump(target, 5.0f);
ASTagUtils::history_get_total_size(target);
```

### 2.5 Event System Integration

**Events are now a specialized tag type** managed through the unified tag system. Resource ASEvent was deprecated and replaced by ASTagUtils-based event handling.

#### 2.5.1 Event Tag Data Structure

Events use the `ASEventTagData` struct in `ASUtils` for full payload information:

- `event_tag`: Exact occurrence identifier (ex: `event.interrupt`)
- `instigator`: The Node that caused it (offender)
- `target`: The affected Node (victim)
- `magnitude`: Base event intensity (impact power)
- `custom_payload`: GDScript/Variant dictionary for metadata transport
- `timestamp`: Natively registered in precise milliseconds
- `tick_id`: Tick identifier for multiplayer synchronization

#### 2.5.2 Event Dispatch API

```gdscript
# Signature: dispatch_event(event_tag, instigator, magnitude, custom_payload)
var asc: ASComponent = target.get_node("ASComponent")
asc.dispatch_event(&"event.weapon.hit", self, 35.0, {})
```

#### 2.5.3 Short-Term Memory (Events Historical)

Events die in 1 tick, but their _"memory"_ persists in a lightweight manner:

- The `ASComponent` mantém um `_event_history` (buffer circular C++ super otimizado de até 128 entradas).
- **Practical use:** Reactive components (como _Parry_ ou _Counter-Attack_) não precisam estar no estado eterno de "parrying". Podem checar o passado recente: `has_event_occurred(&"event.damage.block", 0.4)`. Se o bloqueio aconteceu no último 0.4s, autorize a habilidade de contra-ataque.
- **Rule:** Nunca usar este cache para persistir história (missões, quests). Use exclusivamente para frames de reatividade temporal.

#### 2.5.4 Individual Historical Tracking

All three tag types maintain individual historical buffers in `ASComponent`:

- **NAME History**: Tracks persistent state changes (`state.stunned`, `class.warrior`)
- **CONDITIONAL History**: Tracks permission/immunity changes (`can.parried`, `immune.fire`)
- **EVENT History**: Tracks event occurrences with full payload data

Cada buffer mantém 128 entradas com gerenciamento automático de overflow. As APIs históricas fornecem consultas especializadas:

```cpp
// Consultas de tags NAME
ASTagUtils::name_was_tag_added("state.stunned", target, 2.0f);
ASTagUtils::name_count_additions("state.stunned", target, 10.0f);

// Consultas de tags CONDITIONAL
ASTagUtils::cond_had_tag("immune.fire", enemy, 5.0f);

// Consultas de tags EVENT
ASTagUtils::event_did_occur("event.damage", target, 1.0f);
ASTagUtils::event_get_last_magnitude("event.damage", target);

// Consultas unificadas
ASTagUtils::history_was_tag_present("state.stunned", target, 2.0f);
```

### 2.6 Tag Groups (Visual Organization)

**Tag Groups are not code entities.** They are an editorial convention emerging automatically from dot (`.`) hierarchy in tag identifiers.

- `ASTagsPanel` renderiza tags como uma **árvore**, usando cada segmento separado por ponto como um nó pai.
- `state.stunned`, `state.dead` agrupam automaticamente sob o nó visual `state`.
- Não existe um objeto `TagGroup` C++ — o "grupo" é apenas o prefixo compartilhado.
- **Mandatory Convention:** O prefixo raiz DEVE refletir seu `Tag Type` (ex: tags `Event.*` são sempre `ASTagType::EVENT`).

### 2.7 Logical Evaluation (Predicates)

O sistema suporta 4 estados lógicos em Blueprints (Ability, Effect, Cue) na hora de avaliar os requisitos e bloqueios de um alvo:

- `Required All` (AND): Sucesso se possuir TODAS.
- `Required Any` (OR): Sucesso se possuir PELO MENOS UMA.
- `Blocked Any` (OR): Falha se possuir QUALQUER UMA.
- `Blocked All` (AND): Falha se possuir TODAS SIMULTANEAMENTE.

> [!NOTE]
> Predicates work exclusively on `CONDITIONAL` tags. Tags do tipo `NAME` e `EVENT` não entram em listas de requisito/bloqueio de blueprints. O Editor enforce isso automaticamente via `ASInspectorPlugin`.

### 2.8 Split Registry Pattern

Event identifiers (ex: `event.weapon.hit`) **are registered in the Singleton** like any other tag — to work in editor autocomplete, `ASTagsPanel`, and prevent designer typos. The difference lies in the type: they are registered as `Tag Type = EVENT`.

What **never** goes up to the Singleton is the **data instance** — the `ASEventTagData` struct. Essa separação configura o padrão de **Registro Dividido**:

- **Singleton (Registry):** Conhece o _nome_ `event.weapon.hit`. Garante que existe, que tem o tipo certo e aparece no autocomplete.
- **ASComponent (Occurrence):** Conhece o _acontecimento_. Sabe quem bateu, em quem, com qual força e em qual tick. O Singleton não precisa — nem deve — saber disso.

> [!IMPORTANT]
> **Golden Rule:** Never call `dispatch_event` with an `event_tag` not registered in the Singleton with `Tag Type = EVENT`. Isso seria equivalente a usar um `StringName` digitado à mão sem validação — o campo exato de erros que o sistema foi projetado para eliminar.

---

## 3. ASUtils Structures Registry (Centralized Data)

Todas as estruturas internas são centralizadas em `ASUtils` com API própria, documentação e suporte de serialização. Isso substitui structs internos espalhados por um sistema unificado e documentado.

### 3.1 State Management Structures

- **ASStateCache:**
  - **Purpose**: High-performance circular buffer for rollback
  - **Features**: Capture/restore O(1), configurable size (default: 128), debug utilities
  - **Usage**: Multiplayer prediction and fast state restoration

- **ASStateCacheEntry:**
  - **Purpose**: Lightweight cache entry for a single tick
  - **Data**:
    - tick
    - attributes
    - tags
    - active_effects
  - **Methods**:
    - `to_dict()`
    - `from_dict()`
    - validation

- **ASComponentState:**
  - **Purpose**: Full component state for save/load
  - **Features**:
    - Full historical buffers
    - cooldowns
    - computation of diff
  - **Usage**:
    - Save games
    - full serialization
    - network transfer

### 3.2 Effect System Structures

- **ASEffectState:**
  - **Purpose**: Active effect state representation
  - **Data**: tag, remaining_time, period_timer, stack_count, level
  - **Methods**: `is_expired()`, `is_period_ready()`, serialization

- **ASEffectModifier:**
  - **Purpose**: Single attribute modifier definition
  - **Data**: attribute, operation, magnitude
  - **Usage**: Effect resource definitions

- **ASEffectModifierData:**
  - **Purpose**: Runtime modifier with customized values
  - **Features**: Support for custom magnitude override
  - **Usage**: Runtime calculations of `ASEffectSpec`

- **ASEffectRequirement:**
  - **Purpose**: Attribute requirement for activation
  - **Data**: attribute, amount
  - **Usage**: Effect activation conditions

### 3.3 Attribute System Structures

- **ASAttributeValue:**
  - **Purpose**: Base and current values of attributes
  - **Features**: Difference calculation, value management
  - **Methods**: `set_base()`, `set_current()`, `get_difference()`

### 3.4 Cooldown System Structures

- **ASCooldownData:**
  - **Purpose**: Cooldown timing and associated tags
  - **Features**: Automatic update, support for group cooldowns
  - **Methods**: `is_expired()`, `update()`, serialization

### 3.5 Tag System Structures

- **ASEventTagData:**
  - **Purpose**: Full event dispatch data
  - **Features**: Node references, payload, timing
  - **Methods**: `get_instigator()`, `get_target()`, `set_*()`

- **ASEventTagHistoricalEntry:**
  - **Purpose**: Event occurrence history entry
  - **Data**: Full `ASEventTagData` + tick
  - **Usage**: Event history buffer

- **ASNameTagHistoricalEntry:**
  - **Purpose**: NAME tag change history entry
  - **Data**: tag_name, target_id, timestamp, tick_id, flag added
  - **Usage**: NAME tag history buffer

- **ASConditionalTagHistoricalEntry:**
  - **Purpose**: CONDITIONAL tag change history entry
  - **Data**: tag_name, target_id, timestamp, tick_id, flag added
  - **Usage**: CONDITIONAL tag history buffer

### 3.6 Universal Structure Features

Todas as estruturas ASUtils implementam:

- **Serialization**: Métodos `to_dict()` / `from_dict()`
- **Validation**: `is_valid()` e verificações de integridade
- **Helper Methods**: Funções de conveniência específicas do tipo
- **Documentation**: Documentação XML completa para integração Godot
- **Consistency**: Standardized API patterns across all structures

---

## 4. THE SINGLETON: ABILITY SYSTEM (PROJECT INTERFACE)

- **Role:** The **Global Configuration API** e a ponte com o `ProjectSettings`.
- **Business Rules:**
  - É o único responsável por salvar e carregar a lista global de tags nas configurações do projeto (`project.godot`).
  - Atua como um **Registro Central de Nomes** para garantir que recursos duplicados não entrem em conflito.
- **Limit:** Não deve armazenar estado de nenhum Actor. Se uma informação pertence a uma instância de personagem, ela **não** deve estar aqui.

---

## 5. TOOLS LAYER: EDITORS

Interface entre o Humano e os Resources.

### 5.1 ASEditorPlugin

- **Role:** **Bootloader**.
- **Rule:** Registro de tipos, ícones e inicialização de outros sub-editores. Proibido conter lógica de jogo.

### 5.2 ASTagsPanel

- **Role:** Interface visual para o **Registro Global**.
- **Rule:** Manipula exclusivamente o dicionário de tags do `AbilitySystem` Singleton.

### 5.3 ASInspectorPlugin (and Property Selectors)

- **Role:** Contextualization.
- **Rule:** Deve fornecer seletores inteligentes (dropdowns de tags, busca de atributos) para facilitar a configuração de Resources e Components no Inspetor.

### 5.4 The Compat Layer (Camada de Compatibilidade)

Localizada em `src/compat/`.
O projeto é arquitetado estritamente sob a **Estratégia de Compilação Dual**, suportando compilação tanto como Module nativo da Godot quanto como GDExtension.

- **Role:** Blindar o núcleo do framework das divergências entre as APIs internas do Module e da GDExtension.
- **Contour Rule:** Toda lógica central (`src/core`, `src/resources`, `src/scene`) DEVE ser desenhada de forma agnóstica. Qualquer diferença estrutural necessária para interagir com a Engine (especialmente funcionalidades de GUI do Editor) deve ser isolada nesta pasta, resolvendo a compatibilidade via `#ifdef ABILITY_SYSTEM_GDEXTENSION`.
- **Exclusivity:** Core files nunca devem usar diretamente classes exclusivas de GDExtension que quebrem compilação de Module em C++; eles devem invocar o Wrapper no `compat/`.

---

## 6. THE BLUEPRINTS: RESOURCES (THE "WHAT")

Localizados em `src/resources/`. São as **Definições de Dados**.

- **Resources (Blueprints):** São objetos estáticos (`.tres`) que definem o "DNA" de uma habilidade ou efeito. **Regra:** Não devem ser modificados em runtime (exceto `ASStateSnapshot`). Eles são compartilhados entre centenas de instâncias.
- **Specs (Runtime Instances):** São instâncias leves criadas a partir de Resources que mantêm o estado dinâmico (cooldowns, stacks, duração).
- **Snapshot Exception:** O `ASStateSnapshot` é grafado como `Resource` mas é populado em runtime. Ele rompe a regra de imutabilidade para permitir persistência nativa (Save/Load) e cache de rede via sistema de arquivos/recursos da Godot.

### Restrictions of use and performance

> [!WARNING]
> **O `ASStateSnapshot` é um recurso pesado.** Devido à natureza da captura completa de estado, ele consome CPU e memória significativos se usado em larga escala.

1. **Uso Exclusivo para Players:** O uso de Snapshots deve ser restrito a entidades controladas por jogadores (Playable Characters), onde o determinismo e o rollback são críticos para o multiplayer online.
2. **NPCs e Inimigos:** Entidades não-jogáveis **NÃO** devem usar `ASStateSnapshot`. Para persistência de NPCs, utilize mecanismos mais leves como o `SaveServer` da Zyris Engine ou sistemas personalizados de dicionários em Godot.
3. **Regra de Ouro:** Se é jogável, use `ASStateSnapshot`. Se não for, descarte-o.
4. **Independência de Referência:** O snapshot armazena valores primitivos e nomes de tags, não ponteiros para objetos, garantindo que possa ser serializado com segurança.

- **What lives here:** Valores base, ícones, nomes, tags de requisito e listas de modificadores brutos.

### 6.1 ASAbility & ASEffect (Actions and Modifiers)

- **ASAbility - Role:** Definir a lógica de uma ação (Custos, Cooldown, Triggers).
- **ASAbility - Rule:** Único Resource capaz de gerenciar requisitos de ativação e custos de atributos através de especificação. Suporta **Ability Phases** para execuções complexas.
- **ASEffect - Role:** Modificador de estado (Buffs, Debuffs, Dano).
- **ASEffect - Rule:** Define políticas de empilhamento (Stacking) e magnitudes de mudança nos atributos.

### 6.2 ASAttribute & ASAttributeSet (The Attribute System)

- **ASAttribute - Role:** Define os metadados (limites min/max) de uma única estatística.
- **ASAttributeSet - Role:** Agrupa as estatísticas e define o estado inicial de um personagem. Detém a lógica de modificação de atributos.
- **ASAttributeSet - Regra (Attribute Drivers):** Permite derivar o valor base de um atributo a partir de outro (ex: 2 \* Força = 1 Ataque). O recalculo é automático em mudanças de valor base.
- **ASAttributeSet - Regra (Prioridade):** Modificadores (Flat Add, Multiplier) são aplicados _após_ o cálculo dos Drivers.

### 6.3 ASContainer & ASPackage (Archetypes and Payloads)

- **ASContainer - Role:** Arquétipo completo (Dicionário de Identidade do Ator).
- **ASContainer - Regra:** Atua como o "Template de Fábrica" para inicialização total do `ASComponent`.
- **ASPackage - Role:** Agrupador de transporte (Envelope de Dados).
- **ASPackage - Regra:** Deve ser usado exclusivamente para transmitir coleções de efeitos e cues via `ASDelivery`.

### 6.4 ASCue (Visual Feedbacks)

- **Role:** Feedback audiovisual puro (Animação, Som, Partículas).
- **Rule:** Proibido alterar qualquer dado de gameplay. Deve ser disparado reativamente.

### 6.5 ASAbilityPhase (O Ciclo de Vida Complexo)

A funcionalidade mais poderosa para designers em termos de "Máquinas de Estado" embutidas (Hierarchical Abilities).

- **Role:** Fragmentar a execução engessada de uma habilidade em estágios granulares e altamente configuráveis (ex: `Windup`, `Execution`, `Recovery`).
- **Natureza:** Se uma habilidade padrão age como uma "pistola" (inicia, aplica e termina num click), uma habilidade com Fases atua como um "ritual" com várias etapas no tempo.
- **Regras Críticas:**
  - **Temporário & Específico:** Cada Fase pode aplicar e remover seus próprios `ASEffects` transitórios que duram apenas enquanto aquela fase estiver ativa.
  - **Duração ou Evento:** Uma Fase avança para a próxima de duas formas: (a) Expirou o tempo de duração da fase; (b) Ocorreu o _Transition Trigger Event_ (a habilidade estava aguardando o Node de Animação enviar um Evento `.Hit` para avançar).
  - **Avisos Autônomos:** A transição entre Fases sempre dispara um AS Event automático da própria framework para permitir fluidez e resposta de UI.

---

## 7. THE EXECUTORS: SPECS (THE "HOW")

Localizados em `src/core/`. Onde o estado e a lógica de execução residem.

- **Role:** Representar a **Instância Ativa**. É o dono do **"Agora"**.
- **Golden Rule: STATE SOVEREIGNTY.**
- **O que deve viver aqui (e não no Component):**
  - `duration_remaining`: O timer individual de cada instância.
  - `stack_count`: Quantas vezes este efeito específico está acumulado.
  - `calculate_...`: Lógica de cálculo que depende de atributos variáveis (ex: dano baseado em força atual).
- **Responsabilidade:** O Spec deve saber se "terminou" ou não. O Component apenas pergunta a ele.

### 7.1 ASAbilitySpec & ASEffectSpec (Execution)

- **ASAbilitySpec - Role:** Habilidade em execução ativa ou equipada.
- **ASAbilitySpec - Regra:** Gerencia o cooldown individual e o estado de ativação.
- **ASEffectSpec - Role:** Instância ativa de um modificador.
- **ASEffectSpec - Regra:** Detém a soberania sobre o tempo restante (`duration`) e pilhas (`stacks`).

### 7.2 ASCueSpec & ASTagSpec (Feedback and Identity)

- **ASCueSpec - Role:** Gerenciador do ciclo de vida de um feedback na cena.
- **ASCueSpec - Regra:** Garante a limpeza (Queue Free) do Node instanciado após o término.
- **ASTagSpec - Role:** Contador de referências (Refcount) para Tags.
- **ASTagSpec - Regra:** Garante que uma Tag só saia do ator quando todos os seus Specs de origem expirarem.

### 7.3 ASAbilitySpec (Gestão de Fases)

- **Role:** Gerencia o índice da fase atual e a progressão temporal entre os estágios definidos no `ASAbility`.
- **Regra:** Deve ser capaz de avançar para a próxima fase via tempo ou via recebimento de um `ASEventTag` específico.

---

## 8. THE ORCHESTRATOR: COMPONENT (THE HUB)

O `ASComponent` (ASC).

- **Role:** **Gestor de Coleções** e **Roteador de Sinais**.
- **Business Rules:**
  - Não deve gerenciar timers individuais de efeitos (isso é do Spec).
  - Responsável por manter a lista de `active_specs` e `unlocked_specs`.
  - Atua como o **Dono dos Atributos** (via `AttributeSet`).
  - É o único que pode adicionar/remover tags do Actor.
- **Garantir Determinismo:** O ASC deve ser capaz de retroceder e avançar seu estado (Rollback/Prediction) via `ASStateSnapshot`.
- **Cache de Estado:** Manter um buffer interno de snapshots para sincronização de rede.
- **Limite:** O ASC não deve saber os detalhes internos de como uma habilidade funciona. Ele apenas diz: `spec->_activate()`, `spec->tick(delta)`, `spec->_deactivate()`.
- **Node Registry:** O Componente deve manter um registro de aliases de nós (ex: "Muzzle") para que Cues saibam onde instanciar efeitos visuais sem dependências de caminhos de cena.
- **ASEventTagTagHistorical (Memória de Eventos):**
  - **Role:** O ASC mantém um buffer circular de eventos recentes para permitir lógica condicional baseada no passado imediato.
  - **Golden Rule:** Não deve ser usado para persistência de longo prazo. É um "cache de reatividade".
  - **Queries:** Permite perguntar: "Ocorreu o evento X nos últimos Y ticks?".
  - **Sincronia:** Assim como o `ASStateCache`, o histórico de eventos deve ser sensível ao `tick` temporal para garantir consistência em situações de Rollback.

---

## 9. DELIVERY SYSTEMS AND REACTIVITY

### 9.1 ASDelivery (Payload Injections)

- **Role:** Desacoplar o emissor do alvo em interações espaciais (projéteis, AoEs).
- **Rule:** Transporta um `ASPackage` e injeta o conteúdo ao colidir com um ASC.

### 9.2 Ability Triggers (Reactive Automation)

- **Role:** Permitir ativação automática de habilidades baseada em eventos de estado (Tags) ou eventos transitórios (AS Events).
- **Rule:** Ativação baseada exclusivamente em `ON_TAG_ADDED`, `ON_TAG_REMOVED` ou `ON_EVENT_RECEIVED`.

---

## 10. REPLICATION AND PERSISTENCE (DETERMINISMO)

O Ability System é projetado para multiplayer autoritativo com suporte a Predição e Rollback. O estado de um Ator em um determinado momento (Tick) é gerido por dois mecanismos sincronizados:

- **Fonte de Verdade (Physics Only):** O `tick` é o único identificador temporal válido. O `ASComponent` opera **exclusivamente** via `physics_process`. O uso de `_process` (Idle/Frame) é terminantemente proibido para lógica de gameplay para garantir determinismo entre instâncias e suporte a Rollback.

### 10.1 ASStateSnapshot (The heavy)

- **Role:** Persistência de longo prazo (Save/Load) e sincronização externa de "Diferencial de Estado".
- **Natureza:** É um **Godot Resource** (`.tres`). Alocado na Heap, suporta serialização nativa.
- **Regra de Uso:** Reservado exclusivamente para **Players** (Playable Characters) ou estados que precisam sobreviver a reinicializações de cena.
- **SSOT:** É o único recurso autorizado a ser mutável em runtime para fins de captura de estado completo.

### 10.2 ASStateCache (The lightweight)

- **Role:** Memória de curto prazo para Predição, Reconciliação e NPCs.
- **Natureza:** **Struct C++ pura**. Alocada em stack/inline dentro de um buffer circular (`Vector`).
- **Regra de Uso:** Deve ser usado para manter o histórico recente de ticks (ex: últimos 64-128 ticks) para cálculos de rede.
- **Vantagem:** Zero overhead de alocação de Resource. Ideal para sincronização rápida de entidades não-jogáveis (NPCs/Inimigos).

### 10.3 Net Activation and Determinism Flow

1. **Request:** O cliente solicita a ativação chamando `request_activate_ability(tag)`.
2. **Predict:** O cliente executa localmente a ação para latência zero e gera uma entrada no `cache_buffer` via `capture_snapshot()`. Se for um Player, o `ASStateSnapshot` também é atualizado.
3. **Confirm/Correct:** O servidor valida o request e responde. Se houver divergência, o servidor envia o estado autoritativo. O cliente então realiza o **Rollback** buscando o tick correspondente no `cache_buffer` para restaurar atributos e tags instantaneamente.
4. **Determinism:** Lógicas de gameplay (Magnitude de dano, custos) devem ser puras e basear-se exclusivamente nos dados contidos no ASC e seus Specs para garantir que o mesmo input gere o mesmo output em todas as instâncias.

---

## 11. REACTIVITY PROTOCOL (THE TRICHROMATIC MATRIX)

Para evitar que a arquitetura decline e se torne um emaranhado caótico onde todos os sistemas interferem uns nos outros, estabelecemos o _Protocolo de Reatividade_. Esta é a doutrina de como os 3 Pilares operam em uníssono orquestrado.

### 11.1 A Ordem Natural

1. **INPUT/AÇÃO:** Uma interação, término temporizador ou impacto físico emite um **AS Event** (`event.damage`).
2. **ESCUTA/PROCESSAMENTO:** Uma Entidade escuta via Triggers (`ON_EVENT`).
3. **MUTAÇÃO:** A habilidade reativa acerta os requisitos, invoca e aplica o mutador (`ASEffect`).
4. **ESTADO (Fim do Ciclo):** O Effect mutou os atributos ou adicionou permanentemente a **AS Tag** (`state.stunned`).

> [!CAUTION]
> **Fatal Errors punished with deep refactor:**
>
> - Esperar que uma habilidade inicie baseada em "perda de tag". (Isso é sintoma de acoplamento de estado; dispare um Evento avisando o fim).
> - Se uma Habilidade falhar num requisito de Tag ou Custo, NUNCA gerencie estado (aplicar tags temporárias). Emita o gatilho `event.ability.failed` relatando por qual motivo (Dicionário Payload), para loggers ou UI reagirem.

### 11.2 Hybrid Triggers

A ativação de Habilidades através do _Ability Triggers_ no Spec passou por revisão de hierarquia. Pode-se construir automação pura através deles:

- `TRIGGER_ON_EVENT`: É o padrão ouro para responsidade de combate (Reagir instantaneamente no impacto).
- `TRIGGER_ON_TAG_ADDED` / `REMOVED`: É o padrão para automação de ambiente (Ligar aura de lentidão quando entrar na água).

### 11.3 O Pacto do ASDelivery

O componente `ASDelivery` (ex: um míssil ou aura rastreado) carrega o envelope mortífero `ASPackage`.

- **Regra e Obrigação:** Todo ASDelivery, ao concluir rota e aplicar pacote a um ASC alvo, DEVE obrigatoriamente emitir o disparo de Evento invocando `target_asc->dispatch_event(package_tag)`.
- Isso assegura que "tomar uma fireball na cara" automaticamente preencha a memória temporal do ASC alvo (`ASEventTagHistorical`), habilitando bloqueios ou triggers reativos purificados.

---

---

## 12. ATOMIC AI INTEGRATION (ASBridge & HSM)

To interact with the LimboAI framework atomically, the Ability System provides **ASBridge** — not as a class or Singleton, but as an architectural native integration layer (located at `src/bridge/`).

- **Status v0.2.0:** The AI infrastructure is part of the same binary as the Ability System, guaranteeing zero latency. Support for the obsolete `ASBridge` Singleton has been completely decommissioned.
- **The Integrator Layer Role:** `ASBridge` acts simply as the collection of Behavior Tree Tasks and States (`BTAction`, `BTCondition`, `LimboState`) that connect LimboAI directly to the `ASComponent`.
- **Centralized Resolution:** The global `AbilitySystem` Singleton is the absolute authority responsible for `resolve_component()`. When an AI agent needs to find its `ASComponent`, the Bridge Tasks query `AbilitySystem::get_singleton()->resolve_component(agent, path)`. The Singleton acts as the **Reference Judge**, maintaining a secure node search and resolving string aliases efficiently so AI nodes do not need to hardcode absolute scene paths.
- **HSM Synergy:** `ASComponent` is natively compatible with **LimboHSM**. The character's hierarchical state machine is driven by Ability System's Tag identifiers and Events.
- **Rigorous Decoupling:** Even though they are in the same binary, BT/HSM tasks never call ability cores directly. They use the Bridge layer to preserve the **Reactivity Protocol**.

---

## 13. TECHNICAL RIGOR AND TEST QUALITY

### 13.1 Padrão 300% (Iron Law)

Cada funcionalidade deve ser provada por pelo menos **3 variações** no mesmo teste:

1. **Happy Path:** Cenário base ideal.
2. **Negative:** Entrada inválida ou falha esperada.
3. **Edge Case:** Combinações complexas (multi-tags, limites de borda).

### 13.2 Test Suites

- **Core (Unit):** Atômicos, sem efeitos colaterais.
- **Advanced (Integration):** DoT/HoT periódico, fluxos de RPG complexos.
- **Multiplayer (Simulation):** Executado via `utility/multiplayer/runner.py` com latência injetada.

---

## 14. API NAMING STANDARDS (AS Specific)

Para manter a consistência, toda a API pública deve seguir estes padrões próprios do Ability System:

### 14.1 Method Prefixes

Os métodos são categorizados pela sua intenção e camada de acesso:

- **🎮 Camada de Gameplay (Usage for Game Logic)**
  - `try_activate_...`: **Safe execution.** Tenta disparar uma lógica que depende de requisitos prévios. Integra a verificação e a ação. **Uso obrigatório para habilidades e efeitos.**
  - `can_...`: **Pre-authorization.** Avalia se uma ação pode ser executada sem iniciá-la.
  - `is_...`: **Status study.** Verifica condições booleanas de estado ou identidade (ex: `is_ability_active`).
  - `has_...`: **Possession query.** Verifica se o objeto detém uma chave específica (ex: `has_tag`).
  - `get_...`: **Extracting information.** Obtém valores, referências ou metadados de leitura.
  - `cancel_...`: **Interruption.** Encerra voluntariamente um fluxo em execução.
  - `request_...`: **Network intention.** Solicita a execução de uma ação via RPC (Multiplayer).
- **🏗️ Camada de Infraestrutura/Interna (Restricted use or Config)**
  - `apply_...`: **Forced application.** Injeta um payload ou container ignorando regras de ativação. Usado em inicialização ou por sistemas de entrega (`ASDelivery`).
  - `add_...` / `remove_...`: **Low-level mutation.** Altera coleções internas. Não deve ser usado como atalho para ativar lógica de jogo (ex: use `try_activate` em vez de tentar "adicionar" um efeito manualmente).
  - `unlock_...` / `lock_...`: **Inventory management.** Altera a disponibilidade de habilidades no catálogo do ator.
  - `register_...` / `unregister_...`: **System bound.** Conecta o componente a nós externos ou gerencia o Singleton global.
  - `rename_...`: **Refactor internals.** Altera identificadores de Tags e propaga a mudança.
  - `set_...`: **Direct mutation.** Define valores brutos (Base Values).
  - `clear_...`: **Reset total.** Limpa estados ou coleções de forma absoluta.
  - `capture_...`: **Persistence.** Congela o estado atual em um snapshot.

> [!IMPORTANT]
> **Não existe `activate` direto na API pública.** O uso de `try_...` é o único caminho seguro para gameplay. Métodos de infraestrutura (`apply_`, `add_`) existem para sistemas de baixo nível e não devem ser usados para contornar verificações de custos e cooldowns.

### 14.2 Suffixes of origin and context

Sufixos são obrigatórios para resolver ambiguidades de entrada, destino ou comportamento:

- **Origem (Input):**
  - `..._by_tag`: Operação baseada na identidade global da tag (ex: `has_tag`, `try_activate_ability_by_tag`).
  - `..._by_resource`: Operação baseada na instância do arquivo de recurso (ex: `try_activate_ability_by_resource`).
- **Destino (Target):**
  - `..._to_self`: A ação é aplicada exclusivamente ao componente que a chamou.
  - `..._to_target`: A ação exige um componente alvo explícito como argumento.
- **Comportamento (Behavior):**
  - `..._debug`: Retorna dados verbosos ou não otimizados para ferramentas de diagnóstico.
  - `..._preview`: Realiza cálculos teóricos (E se...?) sem aplicar efeitos colaterais ou consumir recursos.
  - `..._all`: Operação em massa que afeta toda a coleção relevante (ex: `cancel_all_abilities`).

### 14.3 Arguments and Typing

A nomenclatura de argumentos deve ser autoexplicativa e seguir a hierarquia de tipos do projeto:

**Descritive nomenclature:** Proibido o uso de variáveis de um único caractere (ex: `t`, `a`). Use o nome completo do conceito (`tag`, `ability`, `effect`, `attribute`, `level`, `value`, `target_node`, `data`).

- **Performance Typing:**
  - `StringName`: Para todos os identificadores de tags e nomes de atributos (chave de dicionário).
  - `float`: Para todas as magnitudes, níveis e durações (mesmo que sejam valores inteiros na lógica, o motor de atributos opera em ponto flutuante).
  - `Dictionary`: Para payloads de dados variáveis (ex: no sistema de Cues).
- **Canonical Order:** Quando múltiplos argumentos são necessários, siga a ordem de importância:
  1. **Identificador:** `tag` ou `resource`.
  2. **Magnitude:** `level` ou `value`.
  3. **Target/Context:** `target_node` ou `data`.
- **Valores Default:** Argumentos opcionais devem possuir valores neutros documentados (ex: `level = 1.0`, `target_node = null`).

### 14.4 Signals

Sinais devem comunicar eventos que **já ocorreram**, seguindo o padrão de voz passiva:

- **Formato:** `snake_case` no tempo passado.
- **Exemplos Corretos:** `ability_activated`, `effect_removed`, `attribute_changed`, `tag_event_received`.
- **Exemplos Incorretos:** `on_ability_activate` (prefixo desnecessário), `activate_ability` (confunde com método).

### 14.5 Internal C++ Members

Para garantir segurança e legibilidade no código-fonte GDExtension:

- **Variáveis Privadas/Protegidas:** Devem obrigatoriamente começar com `_` (underscore). Ex: `_attribute_set`, `_is_active`.
- **reactive Getters/Setters:** Sempre que uma mudança de variável exigir uma reavaliação (ex: mudar a tag exige recontar o ASTagSpec), deve-se usar um setter formal em vez de acesso direto.
- **Propriedades Públicas:** Devem espelhar os nomes brutos (sem `_`) para serem expostas corretamente ao Inspetor do Godot.
- **Dual Build:** Todo código de teste deve suportar a compilação via `#ifdef ABILITY_SYSTEM_GDEXTENSION`.

---

## 15. DESIGN PATTERNS (C++/GDEXTENSION)

O sistema utiliza padrões clássicos adaptados para a arquitetura de alta performance da Godot Engine.

### 15.1 Spec Pattern (Resource vs. Instance)

- **O Problema:** Resources Godot são compartilhados. Modificar um afetaria todos os inimigos do mesmo tipo.
- **A Solução:** Separação total entre **Resource** (Dados Imutáveis/DNA) e **Spec** (Estado de Runtime/Instância).
- **Regra:** Toda lógica que exige alteração de valor (timers, stacks) deve residir no Spec. O Resource é apenas um Provedor de Dados.

### 15.2 Flyweight Pattern (optimized memory)

- **Conceito:** Milhares de atores compartilham as mesmas referências de `ASAbility` e `ASEffect`.
- **Implementação:** O `ASComponent` armazena apenas ponteiros (RefCounters) para os Resources. Os dados pesados (ícones, curvas de dano) nunca são duplicados na memória.

### 15.3 Command Pattern (Abilities)

- **Conceito:** Cada habilidade é um comando auto-contido que sabe como iniciar, executar e cancelar.
- **Requisito:** Encapsulamento total. O componente não deve conhecer a lógica interna da habilidade; ele apenas despacha o comando.

### 15.4 Data-Driven Design

- **Regra:** O comportamento deve ser definido em arquivos `.tres` no Editor, não em código rígido (Hardcoded).
- **Vantagem:** Permite que designers alterem o balanceamento sem recompilar o plugin.

---

## 16. TEST PATTERNS (RIGOR AND DETERMINISM)

A confiabilidade do sistema é garantida por padrões de teste industriais.

### 16.1 Deterministic Physics Ticking

- **Regra:** Testes de tempo (Cooldowns/Duração) devem ser validados via `physics_process` em passos fixos (Ticks).
- **Mocking do Tempo:** Em testes unitários, simulamos a passagem do tempo chamando `tick(delta)` manualmente para garantir que 1.0s seja exatamente 1.0s, independente do lag do hardware.

### 16.2 Isolation & Mocking

- **Mock Assets:** Para testar habilidades complexas, criamos Resources temporários em memória via código no `TEST_CASE`.
- **Dummy Actors:** O uso de nodes simples com `ASComponent` é preferível a carregar cenas complexas (`.tscn`) para testes unitários, garantindo velocidade de execução.

### 16.3 State Injection

- **Padrão:** Em vez de esperar 10 segundos para testar o fim de um efeito, o teste deve injetar um Spec com `duration_remaining = 0.1` e validar o próximo tick.

### 16.4 Signal Auditing

- **Padrão:** Todo teste de ativação deve auditar se o sinal correspondente (ex: `ability_activated`) foi emitido com os argumentos corretos, garantindo que o feedback visual (Cues) também funcione.
