# Implementation Plan HA & AS Events

O objetivo é projetar o suporte a **Hierarchical Abilities** (Ability Phases) e **AS Events** dentro do Ability System. Isso permitirá que uma única `ASAbility` seja composta por sub-estados (Fases como Windup, Execution, e Recovery), gerenciando transições de forma limpa e reativa.

## User Review Required

> [!WARNING]
> A introdução de Fases dentro das Habilidades adicionará certa complexidade na predição de Rede (Multiplayer). O Snapshot precisará serializar qual "Sub-estado" está ativo, junto do tempo decorrido nessa fase específica. Gostaria de validar com você se o design simplificado linear (Fases Sequenciais estritas) atende à demanda, ou se será necessário um grafo de estados pleno dentro da própria habilidade.

## Proposed Changes

### 1. Resources: Blueprints de Fases

#### [NEW] [as_ability_phase.h](file:///C:/Users/bruno/Desktop/AbilitySystemPlugin/src/resources/as_ability_phase.h)

Novo Resource de dados para representar as partes de uma habilidade.

- `StringName phase_name`: (Ex: "Windup", "Recovery").
- `float duration`: Duração base para expirar esta fase e ir para a próxima.
- `TypedArray<ASEffect> applied_effects`: Efeitos ou Custos exclusivos aplicáveis ao **iniciar** a fase.
- `StringName transition_event_tag`: Se preenchida, a fase não acaba por tempo, mas sim esperando passivamente que o ASC receba essa Tag via AS Event.

#### [MODIFY] [as_ability.h](file:///C:/Users/bruno/Desktop/AbilitySystemPlugin/src/resources/as_ability.h)

- Adição da propriedade `TypedArray<ASAbilityPhase> phases`.
- Permite design visual no Inspector para empilhar Fases de execução. Se vazio, a Habilidade atua da forma plana clássica de antes.

---

### 2. Runtime: Specs e Orquestração

#### [MODIFY] [as_ability_spec.h](file:///C:/Users/bruno/Desktop/AbilitySystemPlugin/src/core/as_ability_spec.h)

- Inserção de estado interno para navegação: `int _current_phase_index`.
- `void advance_phase()`: Processa os sub-efeitos da próxima fase da fila e reseta os timers.
- O método [tick(delta)](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/reverse-engineering/limboai-plugin/bt/tasks/bt_task.h#97-98) passa a checar não mais a duração total, mas o tempo da **fase atual**, ativando as transições internas até chegar ao índice final.

#### [NEW] [as_event.h](file:///C:/Users/bruno/Desktop/AbilitySystemPlugin/src/core/as_event.h)

Estrutura transitória C++ (Struct Leve) para transportar o payload:

```cpp
struct ASEvent {
    StringName event_tag;
    Node* instigator = nullptr;
    Node* target = nullptr;
    float magnitude = 0.0f;
    Dictionary custom_payload;
    double timestamp = 0.0; // Seconds since game start or Tick ID
};

struct ASEventHistoricalEntry {
    ASEvent data;
    uint64_t tick;
};
```

#### [MODIFY] [as_component.h](file:///C:/Users/bruno/Desktop/AbilitySystemPlugin/src/scene/as_component.h)

- `void dispatch_event(const ASEvent& p_data)`: Roteia um evento arbitário e o insere no historical buffer.
- `Vector<ASEventHistoricalEntry> _event_history`: Buffer circular para persistência de curto prazo.
- `bool has_event_occurred(StringName p_event_tag, float p_lookback_time)`: Consulta o histórico.

---

## Verification Plan

### Automated Tests

1. `validate_ability_phases(...)`: Uma habilidade de 3 fases que aplica efeitos no meio de cada estágio. O ASC será testado temporalmente avançando `delta` na mão e inspecionando o "phase index".
2. `validate_as_events(...)`: Uma habilidade empacada esperando o evento "Combo.Finisher". Envio do evento deve forçar a progressão do fluxo da habilidade.
