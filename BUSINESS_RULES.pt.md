# BUSINESS RULES: ABILITY SYSTEM - GOVERNANCE CONTRACT

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](BUSINESS_RULES.md) | [**Português**](BUSINESS_RULES.pt.md)

Este documento estabelece as fronteiras arquiteturais e regras de negócio obrigatórias. Qualquer implementação que viole estes limites deve ser refatorada imediatamente.

---

## 1. FILOSOFIA E ENGENHARIA RIGOROSA

O projeto rejeita o **"Vibe-Coding"** (programação por intuição ou sorte). Cada linha de lógica de negócio é tratada como um compromisso de engenharia industrial.

### 1.1 Pair Programming e Governança

- **Desapego Radical ao Código:** Se o código falha, a falha é na comunicação ou na arquitetura. A correção é feita via diálogo e ajuste da documentação, nunca por remendos manuais.
- **SSOT (Single Source of Truth):** Este arquivo é a Lei de Ferro. Antes de qualquer mudança complexa, a regra deve estar documentada aqui.
- **Idioma:** Código e documentação técnica em **Inglês**. Diálogo e tom de pair programming em **Português**.

### 1.2 Protocolo TDD (Red-Green-Refactor)

Nenhuma lógica de negócio existe sem um teste que a justifique.

1. **RED:** Escrever o teste que falha, definindo o contrato.
2. **GREEN:** Implementar o código mínimo para passar.
3. **REFACTOR:** Otimizar mantendo o status de aprovação.

---

## 2. A MATRIZ DE IDENTIDADE: TAGS (O "ESTADO")

Tags não são classes; são **Identificadores Hierárquicos Superpoderosos** baseados em `StringName`. Representam a verdade absoluta sobre o estado presente de um Ator.

### 2.1 Regras de Ouro das Tags (Segurança v0.2.0)

- **Tipo CONDITIONAL (Requisito de Lógica):** Apenas tags marcadas como `CONDITIONAL` podem ser usadas em listas de requisitos (`Required/Blocked`) de Habilidades, Efeitos ou Cues. Tentar usar uma tag de Nome ou Evento para bloqueio lógico é uma violação de segurança.
- **Tipo NAME (Identidade de Recurso):** Apenas as tags do tipo `NAME` podem ser usadas para identificar e nomear recursos de `ASAbility` ou `ASEffect`. Isto garante que um estado (ex: `state.stunned`) nunca seja confundido com uma habilidade (ex: `ability.warrior.powerhit`).
- **Tipo EVENT (Ocorrência de Frame):** Tags do tipo `EVENT` são transitórias e usadas exclusivamente para despacho de ocorrências através de `ASTagUtils`. Elas não persistem no RefCount do Singleton.

### 2.2 Os 3 Tipos Canônicos de Tag (Tag Types / ASTagTypes)

| Tipo          | Papel Semântico                                | Restrição de Uso                                                        |
| ------------- | ---------------------------------------------- | ----------------------------------------------------------------------- |
| `NAME`        | Identidade estática (Quem eu Sou/O que isto é) | **EXCLUSIVO** para Identificação de Resources (`ASAbility`/`ASEffect`). |
| `CONDITIONAL` | Estado de gameplay persistente (Como eu Estou) | **EXCLUSIVO** para Requisitos Lógicos (`Required`/`Blocked`).           |
| `EVENT`       | Ocorrência instantânea (O que Aconteceu)       | **EXCLUSIVO** para despacho de eventos via `ASTagUtils` e `ASBridge`.   |

---

## 3. O SINGLETON: ABILITY SYSTEM & ASUTILS

- **Papel:** É a **API de Configuração Global** e o repositório de lógica estática atómica.
- **ASUtils:** Namespace C++ que centraliza funções de utilidade de alta performance para manipulação de tags e cálculos de atributos.
- **Regras de Negócio:**
  - O Singleton é o único que pode persistir a lista de tags no `project.godot`.
  - Atua como um **Validador de Tipagem**: Impede em tempo de edição (via Inspector) que tags do tipo errado sejam usadas em campos restritos.

### 2.3 Implementação dos Tipos de Tag (Visão Semântica)

O sistema unificado de tags fornece manipulação type-safe em três categorias primárias. A implementação detalhada reside em `as_tag_types.h`.

| Categoria            | Propósito                                           | Tempo de Vida Típico                  |
| :------------------- | :-------------------------------------------------- | :------------------------------------ |
| **ASNameTag**        | Identidade persistente (Classe, Time, Traços).      | Indefinido (Até remoção manual).      |
| **ASConditionalTag** | Portões lógicos (Imunidade, Requisitos, Estados).   | Transiente ou Ligado a Efeitos.       |
| **ASEventTag**       | Ocorrências instantâneas (Dano, Morte, Conjuração). | 1 Tick (Registro histórico persiste). |

### 2.4 Infraestrutura de Rastreamento Histórico

O `ASComponent` mantém **buffers circulares de alta performance de 128 entradas** para cada categoria de tag, suportando reatividade e reconciliação multiplayer.

- **NAME History**: Rastreia quando estados persistentes foram adicionados/removidos.
- **CONDITIONAL History**: Rastreia mudanças em permissões e imunidades.
- **EVENT History**: Armazena o payload completo `ASEventTagData` (Instigador, Magnitude, Tick) de ocorrências recentes.

#### APIs de Consulta Histórica

O namespace `ASTagUtils` fornece métodos otimizados para consultar estes buffers:

- `was_tag_added/removed(tag, lookback_sec)`: Verifica transições de estado.
- `did_occur(event_tag, lookback_sec)`: Verifica eventos de combate/gameplay recentes.
- `get_last_magnitude/instigator(event_tag)`: Recupera dados específicos da ocorrência mais recente.

      // Usada para mudanças de tags CONDITIONAL

  };

struct ASEventTagHistorical {
ASEventTagData data; // Payload completo do evento
uint64_t tick_id = 0;
};

````

#### 2.4.3 APIs de Consulta Histórica

**API de Consulta Histórica ASNameTag:**

```cpp
// Consultas Básicas
ASTagUtils::name_was_tag_added("state.stunned", target, 1.0f);
ASTagUtils::name_was_tag_removed("state.stunned", target, 1.0f);
ASTagUtils::name_had_tag("state.stunned", target, 1.0f);

// Recuperação de Dados
ASTagUtils::name_get_recent_additions(target, 1.0f);
ASTagUtils::name_get_recent_removals(target, 1.0f);
ASTagUtils::name_get_recent_changes(target, 1.0f);

// Operações de Contagem
ASTagUtils::name_count_additions("state.stunned", target, 1.0f);
ASTagUtils::name_count_removals("state.stunned", target, 1.0f);
````

**API de Consulta Histórica ASConditionalTag:**

```cpp
// Especializada para mudanças de tags CONDITIONAL
ASTagUtils::cond_was_tag_added("immune.fire", target, 1.0f);
ASTagUtils::cond_was_tag_removed("immune.fire", target, 1.0f);
ASTagUtils::cond_had_tag("immune.fire", target, 1.0f);
```

**API de Consulta Histórica ASEventTag:**

```cpp
// Consultas Específicas de Evento
ASTagUtils::event_did_occur("event.damage", target, 1.0f);
ASTagUtils::event_get_recent_events("event.damage", target, 1.0f);
ASTagUtils::event_get_all_recent_events(target, 1.0f);

// Acesso a Dados de Evento
ASTagUtils::event_get_last_data("event.damage", target);
ASTagUtils::event_get_last_magnitude("event.damage", target);
ASTagUtils::event_get_last_instigator("event.damage", target);

// Operações de Contagem
ASTagUtils::event_count_occurrences("event.damage", target, 1.0f);
```

**API de Consulta Histórica Unificada:**

```cpp
// Consultas Cross-Type
ASTagUtils::history_was_tag_present("state.stunned", target, 1.0f);
ASTagUtils::history_get_tag_history("state.stunned", target, 1.0f);
ASTagUtils::history_get_all_changes(target, 1.0f);

// Utilitários de Debug
ASTagUtils::history_dump(target, 5.0f);
ASTagUtils::history_get_total_size(target);
```

### 2.5 Integração do Sistema de Eventos

**Events agora são um tipo especializado de tag** gerenciado através do sistema unificado de tags. O antigo Resource ASEvent foi depreciado e substituído por manipulação de eventos baseada em ASTagUtils.

#### 2.5.1 Estrutura de Dados de Event Tag

Events usam o struct `ASEventTagData` em `ASUtils` para informações completas de payload:

- `event_tag`: Identificador exato da ocorrência (ex: `event.interrupt`)
- `instigator`: O Node que causou (ofensor)
- `target`: O Node afetado (vítima)
- `magnitude`: Intensidade base do evento (Poder do impacto)
- `custom_payload`: Dicionário GDScript/Variant para transporte de metadados
- `timestamp`: Registrado nativamente em milissegundos precisos
- `tick_id`: Identificador de tick para sincronização multiplayer

#### 2.5.2 API de Dispatch de Eventos

```gdscript
# Assinatura: dispatch_event(event_tag, instigator, magnitude, custom_payload)
var asc: ASComponent = target.get_node("ASComponent")
asc.dispatch_event(&"event.weapon.hit", self, 35.0, {})
```

#### 2.5.3 Memória de Curto Prazo (Events Historical)

Events morrem em 1 tick, mas sua _"memória"_ persiste de forma levíssima:

- O `ASComponent` mantém um `_event_history` (buffer circular C++ super otimizado de até 128 entradas).
- **Como utilizar na prática:** Componentes reativos (como _Parry_ ou _Counter-Attack_) não precisam estar no estado eterno de "parrying". Podem checar o passado recente: `has_event_occurred(&"event.damage.block", 0.4)`. Se o bloqueio aconteceu no último 0.4s, autorize a habilidade de contra-ataque.
- **Regra:** Nunca usar este cache para persistir história (missões, quests). Use exclusivamente para frames de reatividade temporal.

#### 2.5.4 Rastreamento Histórico Individual

Todos os três tipos de tags mantêm buffers históricos individuais no `ASComponent`:

- **NAME History**: Rastreia mudanças de estado persistente (`state.stunned`, `class.warrior`)
- **CONDITIONAL History**: Rastreia mudanças de permissão/imunidade (`can.parried`, `immune.fire`)
- **EVENT History**: Rastreia ocorrências de eventos com dados completos do payload

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

### 2.6 Grupos de Tags (Organização Visual)

**Grupos de Tags não são entidades de código.** São uma convenção editorial que emerge automaticamente da hierarquia de pontos (`.`) nos identificadores de tags.

- `ASTagsPanel` renderiza tags como uma **árvore**, usando cada segmento separado por ponto como um nó pai.
- `state.stunned`, `state.dead` agrupam automaticamente sob o nó visual `state`.
- Não existe um objeto `TagGroup` C++ — o "grupo" é apenas o prefixo compartilhado.
- **Convenção Obrigatória:** O prefixo raiz DEVE refletir seu `Tag Type` (ex: tags `Event.*` são sempre `ASTagType::EVENT`).

### 2.7 Avaliação Lógica (Predicados)

O sistema suporta 4 estados lógicos em Blueprints (Ability, Effect, Cue) na hora de avaliar os requisitos e bloqueios de um alvo:

- `Required All` (AND): Sucesso se possuir TODAS.
- `Required Any` (OR): Sucesso se possuir PELO MENOS UMA.
- `Blocked Any` (OR): Falha se possuir QUALQUER UMA.
- `Blocked All` (AND): Falha se possuir TODAS SIMULTANEAMENTE.

> [!NOTE]
> Predicados funcionam exclusivamente sobre tags `CONDITIONAL`. Tags do tipo `NAME` e `EVENT` não entram em listas de requisito/bloqueio de blueprints. O Editor enforce isso automaticamente via `ASInspectorPlugin`.

### 2.8 O Padrão de Registro Dividido (Split Registry)

Os identificadores de Evento (ex: `event.weapon.hit`) **são registrados no Singleton** como qualquer outra tag — para funcionar no autocompletar do editor, no `ASTagsPanel` e evitar erros de digitação dos designers. A diferença está no tipo: eles são cadastrados como `Tag Type = EVENT`.

O que **nunca** sobe ao Singleton é a **instância de dados** — o struct `ASEventTagData`. Essa separação configura o padrão de **Registro Dividido**:

- **Singleton (Registro):** Conhece o _nome_ `event.weapon.hit`. Garante que existe, que tem o tipo certo e aparece no autocomplete.
- **ASComponent (Ocorrência):** Conhece o _acontecimento_. Sabe quem bateu, em quem, com qual força e em qual tick. O Singleton não precisa — nem deve — saber disso.

> [!IMPORTANT]
> **Regra de Ouro:** Nunca chame `dispatch_event` com um `event_tag` que não esteja registrado no Singleton com `Tag Type = EVENT`. Isso seria equivalente a usar um `StringName` digitado à mão sem validação — o campo exato de erros que o sistema foi projetado para eliminar.

---

## 3. Registro de Estruturas ASUtils (Visão Semântica)

Todas as estruturas de dados internas são centralizadas em `as_utils.h` com suporte dedicado a serialização e validação.

| Estrutura            | Propósito                                           | Dados Primários                                        |
| :------------------- | :-------------------------------------------------- | :----------------------------------------------------- |
| **ASStateCache**     | Buffer circular de alta performance (128 ticks).    | Coleção de `ASStateCacheEntry`.                        |
| **ASComponentState** | Representação completa de estado para Persistência. | Atributos, Tags, Efeitos Ativos, Cooldowns, Histórico. |
| **ASAttributeValue** | Container atômico de estatísticas.                  | Valor Base, Valor Atual, Modificadores.                |
| **ASEffectState**    | Dados de instância de efeito em runtime.            | Tag, Tempo restante, Stacks, Nível.                    |
| **ASEventTagData**   | Payload de evento para despacho.                    | Tag, Instigador, Magnitude, Custom Payload, Tick.      |
| **AS\*Historical**   | Entradas individuais de log histórico.              | Tag, Contexto, Timestamp, Tick ID.                     |

### 3.1 Recursos Padronizados

Cada estrutura em `ASUtils` segue o **Padrão de Implementação Universal**:

- **Serialização**: `to_dict()` e `from_dict()` para armazenamento nativo Godot.
- **Validação**: `is_valid()` verifica a integridade arquitetural.
- **Consciência de Tick**: Suporte nativo para `tick_id` para sincronização multiplayer.
- **Consistência de API**: Getters e setters simétricos para todos os campos internos.

---

## 4. O SINGLETON: ABILITY SYSTEM (INTERFACE DE PROJETO)

- **Papel:** É a **API de Configuração Global** e a ponte com o `ProjectSettings`.
- **Regras de Negócio:**
  - É o único responsável por salvar e carregar a lista global de tags nas configurações do projeto (`project.godot`).
  - Atua como um **Registro Central de Nomes** para garantir que recursos duplicados não entrem em conflito.
- **Limite:** Não deve armazenar estado de nenhum Actor. Se uma informação pertence a uma instância de personagem, ela **não** deve estar aqui.

---

## 5. CAMADA DE FERRAMENTAS: EDITORES

Interface entre o Humano e os Resources.

### 5.1 ASEditorPlugin

- **Papel:** **Bootloader**.
- **Regra:** Registro de tipos, ícones e inicialização de outros sub-editores. Proibido conter lógica de jogo.

### 5.2 ASTagsPanel

- **Papel:** Interface visual para o **Registro Global**.
- **Regra:** Manipula exclusivamente o dicionário de tags do `AbilitySystem` Singleton.

### 5.3 ASInspectorPlugin (e Property Selectors)

- **Papel:** Contextualização.
- **Regra:** Deve fornecer seletores inteligentes (dropdowns de tags, busca de atributos) para facilitar a configuração de Resources e Components no Inspetor.

### 5.4 A Camada de Compatibilidade (Compat Layer)

Localizada em `src/compat/`.
O projeto é arquitetado estritamente sob a **Estratégia de Compilação Dual**, suportando compilação tanto como Module nativo da Godot quanto como GDExtension.

- **Papel:** Blindar o núcleo do framework das divergências entre as APIs internas do Module e da GDExtension.
- **Regra de Contorno:** Toda lógica central (`src/core`, `src/resources`, `src/scene`) DEVE ser desenhada de forma agnóstica. Qualquer diferença estrutural necessária para interagir com a Engine (especialmente funcionalidades de GUI do Editor) deve ser isolada nesta pasta, resolvendo a compatibilidade via `#ifdef ABILITY_SYSTEM_GDEXTENSION`.
- **Exclusividade:** Arquivos centrais nunca devem usar diretamente classes exclusivas de GDExtension que quebrem compilação de Module em C++; eles devem invocar o Wrapper no `compat/`.

---

## 6. OS BLUEPRINTS: RESOURCES (O "O QUE")

Localizados em `src/resources/`. São as **Definições de Dados**.

- **Resources (Blueprints):** São objetos estáticos (`.tres`) que definem o "DNA" de uma habilidade ou efeito. **Regra:** Não devem ser modificados em runtime (exceto `ASStateSnapshot`). Eles são compartilhados entre centenas de instâncias.
- **Specs (Runtime Instances):** São instâncias leves criadas a partir de Resources que mantêm o estado dinâmico (cooldowns, stacks, duração).
- **Exceção Snapshot:** O `ASStateSnapshot` é grafado como `Resource` mas é populado em runtime. Ele rompe a regra de imutabilidade para permitir persistência nativa (Save/Load) e cache de rede via sistema de arquivos/recursos da Godot.

### Restrições de Uso e Performance

> [!WARNING]
> **O `ASStateSnapshot` é um recurso pesado.** Devido à natureza da captura completa de estado, ele consome CPU e memória significativos se usado em larga escala.

1. **Uso Exclusivo para Players:** O uso de Snapshots deve ser restrito a entidades controladas por jogadores (Playable Characters), onde o determinismo e o rollback são críticos para o multiplayer online.
2. **NPCs e Inimigos:** Entidades não-jogáveis **NÃO** devem usar `ASStateSnapshot`. Para persistência de NPCs, utilize mecanismos mais leves como o `SaveServer` da Zyris Engine ou sistemas personalizados de dicionários em Godot.
3. **Regra de Ouro:** Se é jogável, use `ASStateSnapshot`. Se não for, descarte-o.
4. **Independência de Referência:** O snapshot armazena valores primitivos e nomes de tags, não ponteiros para objetos, garantindo que possa ser serializado com segurança.

- **O que vive aqui:** Valores base, ícones, nomes, tags de requisito e listas de modificadores brutos.

### 6.1 ASAbility & ASEffect (Ações e Modificadores)

- **ASAbility - Papel:** Definir a lógica de uma ação (Custos, Cooldown, Triggers).
- **ASAbility - Regra:** Único Resource capaz de gerenciar requisitos de ativação e custos de atributos através de especificação. Suporta **Ability Phases** para execuções complexas.
- **ASEffect - Papel:** Modificador de estado (Buffs, Debuffs, Dano).
- **ASEffect - Regra:** Define políticas de empilhamento (Stacking) e magnitudes de mudança nos atributos.

### 6.2 ASAttribute & ASAttributeSet (O Sistema de Atributos)

- **ASAttribute - Papel:** Define os metadados (limites min/max) de uma única estatística.
- **ASAttributeSet - Papel:** Agrupa as estatísticas e define o estado inicial de um personagem. Detém a lógica de modificação de atributos.
- **ASAttributeSet - Regra (Attribute Drivers):** Permite derivar o valor base de um atributo a partir de outro (ex: 2 \* Força = 1 Ataque). O recalculo é automático em mudanças de valor base.
- **ASAttributeSet - Regra (Prioridade):** Modificadores (Flat Add, Multiplier) são aplicados _após_ o cálculo dos Drivers.

### 6.3 ASContainer & ASPackage (Arquétipos e Payloads)

- **ASContainer - Papel:** Arquétipo completo (Dicionário de Identidade do Ator).
- **ASContainer - Regra:** Atua como o "Template de Fábrica" para inicialização total do `ASComponent`.
- **ASPackage - Papel:** Agrupador de transporte (Envelope de Dados).
- **ASPackage - Regra:** Deve ser usado exclusivamente para transmitir coleções de efeitos e cues via `ASDelivery`.

### 6.4 ASCue (Feedbacks Visuais)

- **Papel:** Feedback audiovisual puro (Animação, Som, Partículas).
- **Regra:** Proibido alterar qualquer dado de gameplay. Deve ser disparado reativamente.

### 6.5 ASAbilityPhase (O Ciclo de Vida Complexo)

A funcionalidade mais poderosa para designers em termos de "Máquinas de Estado" embutidas (Hierarchical Abilities).

- **Papel:** Fragmentar a execução engessada de uma habilidade em estágios granulares e altamente configuráveis (ex: `Windup`, `Execution`, `Recovery`).
- **Natureza:** Se uma habilidade padrão age como uma "pistola" (inicia, aplica e termina num click), uma habilidade com Fases atua como um "ritual" com várias etapas no tempo.
- **Regras Críticas:**
  - **Temporário & Específico:** Cada Fase pode aplicar e remover seus próprios `ASEffects` transitórios que duram apenas enquanto aquela fase estiver ativa.
  - **Duração ou Evento:** Uma Fase avança para a próxima de duas formas: (a) Expirou o tempo de duração da fase; (b) Ocorreu o _Transition Trigger Event_ (a habilidade estava aguardando o Node de Animação enviar um Evento `.Hit` para avançar).
  - **Avisos Autônomos:** A transição entre Fases sempre dispara um AS Event automático da própria framework para permitir fluidez e resposta de UI.

---

## 7. OS EXECUTORES: SPECS (O "COMO")

Localizados em `src/core/`. Onde o estado e a lógica de execução residem.

- **Papel:** Representar a **Instância Ativa**. É o dono do **"Agora"**.
- **Regra de Ouro: SOBERANIA DE ESTADO.**
- **O que deve viver aqui (e não no Component):**
  - `duration_remaining`: O timer individual de cada instância.
  - `stack_count`: Quantas vezes este efeito específico está acumulado.
  - `calculate_...`: Lógica de cálculo que depende de atributos variáveis (ex: dano baseado em força atual).
- **Responsabilidade:** O Spec deve saber se "terminou" ou não. O Component apenas pergunta a ele.

### 7.1 ASAbilitySpec & ASEffectSpec (Execução)

- **ASAbilitySpec - Papel:** Habilidade em execução ativa ou equipada.
- **ASAbilitySpec - Regra:** Gerencia o cooldown individual e o estado de ativação.
- **ASEffectSpec - Papel:** Instância ativa de um modificador.
- **ASEffectSpec - Regra:** Detém a soberania sobre o tempo restante (`duration`) e pilhas (`stacks`).

### 7.2 ASCueSpec & ASTagSpec (Feedback e Identidade)

- **ASCueSpec - Papel:** Gerenciador do ciclo de vida de um feedback na cena.
- **ASCueSpec - Regra:** Garante a limpeza (Queue Free) do Node instanciado após o término.
- **ASTagSpec - Papel:** Contador de referências (Refcount) para Tags.
- **ASTagSpec - Regra:** Garante que uma Tag só saia do ator quando todos os seus Specs de origem expirarem.

### 7.3 ASAbilitySpec (Gestão de Fases)

- **Papel:** Gerencia o índice da fase atual e a progressão temporal entre os estágios definidos no `ASAbility`.
- **Regra:** Deve ser capaz de avançar para a próxima fase via tempo ou via recebimento de um `ASEventTag` específico.

---

## 8. O ORQUESTRADOR: COMPONENT (O HUB)

O `ASComponent` (ASC).

- **Papel:** **Gestor de Coleções** e **Roteador de Sinais**.
- **Regras de Negócio:**
  - Não deve gerenciar timers individuais de efeitos (isso é do Spec).
  - Responsável por manter a lista de `active_specs` e `unlocked_specs`.
  - Atua como o **Dono dos Atributos** (via `AttributeSet`).
  - É o único que pode adicionar/remover tags do Actor.
- **Garantir Determinismo:** O ASC deve ser capaz de retroceder e avançar seu estado (Rollback/Prediction) via `ASStateSnapshot`.
- **Cache de Estado:** Manter um buffer interno de snapshots para sincronização de rede.
- **Limite:** O ASC não deve saber os detalhes internos de como uma habilidade funciona. Ele apenas diz: `spec->_activate()`, `spec->tick(delta)`, `spec->_deactivate()`.
- **Node Registry:** O Componente deve manter um registro de aliases de nós (ex: "Muzzle") para que Cues saibam onde instanciar efeitos visuais sem dependências de caminhos de cena.
- **ASEventTagTagHistorical (Memória de Eventos):**
  - **Papel:** O ASC mantém um buffer circular de eventos recentes para permitir lógica condicional baseada no passado imediato.
  - **Regra de Ouro:** Não deve ser usado para persistência de longo prazo. É um "cache de reatividade".
  - **Consultas (Queries):** Permite perguntar: "Ocorreu o evento X nos últimos Y ticks?".
  - **Sincronia:** Assim como o `ASStateCache`, o histórico de eventos deve ser sensível ao `tick` temporal para garantir consistência em situações de Rollback.

---

## 9. SISTEMAS DE ENTREGA E REATIVIDADE

### 9.1 ASDelivery (Payload Injections)

- **Papel:** Desacoplar o emissor do alvo em interações espaciais (projéteis, AoEs).
- **Regra:** Transporta um `ASPackage` e injeta o conteúdo ao colidir com um ASC.

### 9.2 Ability Triggers (Automação Reativa)

- **Papel:** Permitir ativação automática de habilidades baseada em eventos de estado (Tags) ou eventos transitórios (AS Events).
- **Regra:** Ativação baseada exclusivamente em `ON_TAG_ADDED`, `ON_TAG_REMOVED` ou `ON_EVENT_RECEIVED`.

---

## 10. ARQUITETURA MULTIPLAYER: PREDIÇÃO E ROLLBACK

O Ability System é construído nativamente para multiplayer autoritativo usando um modelo de **Predição no Cliente (CSP)** e **Reconciliação pelo Servidor**.

### 10.1 Consciência Temporal de Estado (O Buffer de 128 Ticks)

Cada `ASComponent` mantém um **`ASStateCache`**—um buffer circular leve e de alta performance que armazena o estado dos últimos 128 ticks.

- **Captura**: A cada `physics_tick` (60Hz), o cliente e o servidor registram um snapshot de atributos, tags e efeitos ativos no cache.
- **Afinidade de Tick**: Cada evento, aplicação de efeito e entrada histórica é marcada com um `tick_id`, permitindo o cruzamento temporal preciso.

### 10.2 Predição no Cliente (Latência Zero)

Quando um jogador inicia uma ação (ex: `try_activate_ability`):

1. **Execução Preditiva**: O cliente aplica imediatamente a lógica localmente (reduz atributos, dispara cues, adiciona tags previstas).
2. **Bufferização**: O resultado é armazenado no `ASStateCache` local.
3. **Transmissão**: A intenção é enviada ao servidor via `request_activate_ability`.

### 10.3 Reconciliação e Rollback do Servidor

O servidor processa o request e envia de volta o **Estado Autoritativo** para aquele tick específico.

1. **Comparação**: O cliente compara o estado recebido do servidor com seu estado em cache para o mesmo `tick_id`.
2. **Detecção**: Se houver divergência (ex: o dano previsto foi mitigado de forma diferente no servidor), um **Rollback** é disparado.
3. **Reversão**: O cliente usa `ASComponentState::from_dict()` para sobrescrever instantaneamente seu estado atual com os dados autoritativos do servidor.
4. **Re-predição**: O cliente re-simula todos os inputs locais desde o tick autoritativo até o frame atual para manter a continuidade visual.

### 10.4 Regras de Determinismo de Gameplay

Para minimizar rollbacks, toda a lógica de simulação DEVE ser determinística:

- **Physics Only**: Toda a lógica de gameplay DEVE residir em `_physics_process`. Usar `_process` para mutação de estado é uma violação do protocolo de rede.
- **Prioridade de Atributos**: A ordem de cálculo determinística (Drivers -> Base -> Modificadores) garante resultados idênticos em diferentes arquiteturas.
- **Recálculo Histórico**: Durante o rollback, os logs históricos (`ASAttributeHistorical`, etc.) são fundamentais para re-sincronizar feedbacks visuais (Cues) que podem ter sido pulados no reset.

---

## 11. O PROTOCOLO DE REATIVIDADE E FLUXOS (A MATRIZ TRICROMÁTICA)

Para evitar que a arquitetura decline e se torne um emaranhado caótico onde todos os sistemas interferem uns nos outros, estabelecemos o _Protocolo de Reatividade_. Esta é a doutrina de como os 3 Pilares operam em uníssono orquestrado.

### 11.1 A Ordem Natural

1. **INPUT/AÇÃO:** Uma interação, término temporizador ou impacto físico emite um **AS Event** (`event.damage`).
2. **ESCUTA/PROCESSAMENTO:** Uma Entidade escuta via Triggers (`ON_EVENT`).
3. **MUTAÇÃO:** A habilidade reativa acerta os requisitos, invoca e aplica o mutador (`ASEffect`).
4. **ESTADO (Fim do Ciclo):** O Effect mutou os atributos ou adicionou permanentemente a **AS Tag** (`state.stunned`).

> [!CAUTION]
> **Erros Fatais punidos com refatoração profunda:**
>
> - Esperar que uma habilidade inicie baseada em "perda de tag". (Isso é sintoma de acoplamento de estado; dispare um Evento avisando o fim).
> - Se uma Habilidade falhar num requisito de Tag ou Custo, NUNCA gerencie estado (aplicar tags temporárias). Emita o gatilho `event.ability.failed` relatando por qual motivo (Dicionário Payload), para loggers ou UI reagirem.

### 11.2 Triggers na Era Híbrida

A ativação de Habilidades através do _Ability Triggers_ no Spec passou por revisão de hierarquia. Pode-se construir automação pura através deles:

- `TRIGGER_ON_EVENT`: É o padrão ouro para responsidade de combate (Reagir instantaneamente no impacto).
- `TRIGGER_ON_TAG_ADDED` / `REMOVED`: É o padrão para automação de ambiente (Ligar aura de lentidão quando entrar na água).

### 11.3 O Pacto do ASDelivery

O componente `ASDelivery` (ex: um míssil ou aura rastreado) carrega o envelope mortífero `ASPackage`.

- **Regra e Obrigação:** Todo ASDelivery, ao concluir rota e aplicar pacote a um ASC alvo, DEVE obrigatoriamente emitir o disparo de Evento invocando `target_asc->dispatch_event(package_tag)`.
- Isso assegura que "tomar uma fireball na cara" automaticamente preencha a memória temporal do ASC alvo (`ASEventTagHistorical`), habilitando bloqueios ou triggers reativos purificados.

---

---

## 12. INTEGRAÇÃO ATÔMICA DE IA (A Camada ASBridge & HSM)

Para interagir com o framework LimboAI de forma atômica, o Ability System fornece o conceito de **ASBridge** — não mais como uma classe abstrata ou Singleton, mas como uma **camada arquitetural de integração nativa** (localizada na pasta `src/bridge/`).

- **Status v0.2.0:** A infraestrutura de IA faz parte do mesmo binário do Ability System. O antigo Singleton `ASBridge` foi totalmente descomissionado e explodido.
- **Papel da Camada ASBridge:** O termo refere-se estritamente à coleção de Tarefas e Estados nativos (`BTAction`, `BTCondition`, `LimboState`) que traduzem os comandos do LimboAI para a linguagem do `ASComponent`.
- **A Autoridade de Resolução (AbilitySystem Singleton):** A responsabilidade de encontrar e validar Componentes na árvore (o `resolve_component()`) foi transferida de forma definitiva para o **Singleton `AbilitySystem`**. Todas as tarefas de IA e eventos externos consultam a Engine central via `AbilitySystem::get_singleton()->resolve_component(agente, caminho_ou_alias)`. O Singleton atua como o **Juiz de Referências**, resolvendo caminhos de cena com segurança (evitando travamentos em nós mortos) e lidando com dependências virtuais, de forma que as Behaviour Trees não demandem do Designer saber onde o componente está fisicamente montado.
- **Sinergia HSM:** O `ASComponent` interage intimamente com **LimboHSM**. O estado da máquina de estados hierárquica do personagem pode ser conduzido de forma reativa através dos Eventos e Tags do Ability System.
- **Desacoplamento Rigoroso:** As Tarefas de Behavior Tree e Sub-Estados nunca chamam o motor da habilidade ou os Specs diretamente; a camada Bridge existe para encorajar a separação e o respeito ao **Protocolo de Reatividade**.

---

## 13. RIGOR TÉCNICO E QUALIDADE DE TESTES

### 13.1 Padrão 300% (Lei de Ferro)

Cada funcionalidade deve ser provada por pelo menos **3 variações** no mesmo teste:

1. **Happy Path:** Cenário base ideal.
2. **Negative:** Entrada inválida ou falha esperada.
3. **Edge Case:** Combinações complexas (multi-tags, limites de borda).

### 13.2 Suítes de Teste

- **Core (Unit):** Atômicos, sem efeitos colaterais.
- **Advanced (Integration):** DoT/HoT periódico, fluxos de RPG complexos.
- **Multiplayer (Simulation):** Executado via `utility/multiplayer/runner.py` com latência injetada.

---

## 14. PADRÕES DE NOMENCLATURA DA API (ESPECÍFICOS DO AS)

Para manter a consistência, toda a API pública deve seguir estes padrões próprios do Ability System:

### 14.1 Prefixos de Métodos

Os métodos são categorizados pela sua intenção e camada de acesso:

- **🎮 Camada de Gameplay (Uso para Lógica de Jogo)**
  - `try_activate_...`: **Execução Segura.** Tenta disparar uma lógica que depende de requisitos prévios. Integra a verificação e a ação. **Uso obrigatório para habilidades e efeitos.**
  - `can_...`: **Pré-autorização.** Avalia se uma ação pode ser executada sem iniciá-la.
  - `is_...`: **Consulta de Status.** Verifica condições booleanas de estado ou identidade (ex: `is_ability_active`).
  - `has_...`: **Consulta de Posse.** Verifica se o objeto detém uma chave específica (ex: `has_tag`).
  - `get_...`: **Extração de Informação.** Obtém valores, referências ou metadados de leitura.
  - `cancel_...`: **Interrupção.** Encerra voluntariamente um fluxo em execução.
  - `request_...`: **Intenção em Rede.** Solicita a execução de uma ação via RPC (Multiplayer).
  - `activation_cancel_tags`: **Política de Interrupção.** Tags de habilidades ou efeitos ativos que serão cancelados ao disparar esta ação (HSM Nativa).
- **🏗️ Camada de Infraestrutura/Interna (Uso Restrito ou de Configuração)**
  - `apply_...`: **Aplicação Forçada.** Injeta um payload ou container ignorando regras de ativação. Usado em inicialização ou por sistemas de entrega (`ASDelivery`).
  - `add_...` / `remove_...`: **Mutação de Baixo Nível.** Altera coleções internas. Não deve ser usado como atalho para ativar lógica de jogo (ex: use `try_activate` em vez de tentar "adicionar" um efeito manualmente).
  - `unlock_...` / `lock_...`: **Gestão de Inventário.** Altera a disponibilidade de habilidades no catálogo do ator.
  - `register_...` / `unregister_...`: **Vínculo de Sistema.** Conecta o componente a nós externos ou gerencia o Singleton global.
  - `rename_...`: **Refatoração Interna.** Altera identificadores de Tags e propaga a mudança.
  - `set_...`: **Mutação Direta.** Define valores brutos (Base Values).
  - `clear_...`: **Reset Total.** Limpa estados ou coleções de forma absoluta.
  - `capture_...`: **Persistência.** Congela o estado atual em um snapshot.

> [!IMPORTANT]
> **Não existe `activate` direto na API pública.** O uso de `try_...` é o único caminho seguro para gameplay. Métodos de infraestrutura (`apply_`, `add_`) existem para sistemas de baixo nível e não devem ser usados para contornar verificações de custos e cooldowns.

### 14.2 Sufixos de Origem e Contexto

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

### 14.3 Argumentos e Tipagem

A nomenclatura de argumentos deve ser autoexplicativa e seguir a hierarquia de tipos do projeto:

**Nomenclatura Descritiva:** Proibido o uso de variáveis de um único caractere (ex: `t`, `a`). Use o nome completo do conceito (`tag`, `ability`, `effect`, `attribute`, `level`, `value`, `target_node`, `data`).

- **Tipagem de Performance:**
  - `StringName`: Para todos os identificadores de tags e nomes de atributos (chave de dicionário).
  - `float`: Para todas as magnitudes, níveis e durações (mesmo que sejam valores inteiros na lógica, o motor de atributos opera em ponto flutuante).
  - `Dictionary`: Para payloads de dados variáveis (ex: no sistema de Cues).
- **Ordem Canônica:** Quando múltiplos argumentos são necessários, siga a ordem de importância:
  1. **Identificador:** `tag` ou `resource`.
  2. **Magnitude:** `level` ou `value`.
  3. **Alvo/Contexto:** `target_node` ou `data`.
- **Valores Default:** Argumentos opcionais devem possuir valores neutros documentados (ex: `level = 1.0`, `target_node = null`).

### 14.4 Sinais (Signals)

Sinais devem comunicar eventos que **já ocorreram**, seguindo o padrão de voz passiva:

- **Formato:** `snake_case` no tempo passado.
- **Exemplos Corretos:** `ability_activated`, `effect_removed`, `attribute_changed`, `tag_event_received`.
- **Exemplos Incorretos:** `on_ability_activate` (prefixo desnecessário), `activate_ability` (confunde com método).

### 14.5 Membros Internos e Propriedades (C++)

Para garantir segurança e legibilidade no código-fonte GDExtension:

- **Variáveis Privadas/Protegidas:** Devem obrigatoriamente começar com `_` (underscore). Ex: `_attribute_set`, `_is_active`.
- **Getters/Setters Reativos:** Sempre que uma mudança de variável exigir uma reavaliação (ex: mudar a tag exige recontar o ASTagSpec), deve-se usar um setter formal em vez de acesso direto.
- **Propriedades Públicas:** Devem espelhar os nomes brutos (sem `_`) para serem expostas corretamente ao Inspetor do Godot.
- **Dual Build:** Todo código de teste deve suportar a compilação via `#ifdef ABILITY_SYSTEM_GDEXTENSION`.

---

## 15. DESIGN PATTERNS (C++/GDEXTENSION)

O sistema utiliza padrões clássicos adaptados para a arquitetura de alta performance da Godot Engine.

### 15.1 Spec Pattern (Resource vs. Instance)

- **O Problema:** Resources Godot são compartilhados. Modificar um afetaria todos os inimigos do mesmo tipo.
- **A Solução:** Separação total entre **Resource** (Dados Imutáveis/DNA) e **Spec** (Estado de Runtime/Instância).
- **Regra:** Toda lógica que exige alteração de valor (timers, stacks) deve residir no Spec. O Resource é apenas um Provedor de Dados.

### 15.2 Flyweight Pattern (Memória Otimizada)

- **Conceito:** Milhares de atores compartilham as mesmas referências de `ASAbility` e `ASEffect`.
- **Implementação:** O `ASComponent` armazena apenas ponteiros (RefCounters) para os Resources. Os dados pesados (ícones, curvas de dano) nunca são duplicados na memória.

### 15.3 Command Pattern (Abilities)

- **Conceito:** Cada habilidade é um comando auto-contido que sabe como iniciar, executar e cancelar.
- **Requisito:** Encapsulamento total. O componente não deve conhecer a lógica interna da habilidade; ele apenas despacha o comando.

### 15.4 Data-Driven Design

- **Regra:** O comportamento deve ser definido em arquivos `.tres` no Editor, não em código rígido (Hardcoded).
- **Vantagem:** Permite que designers alterem o balanceamento sem recompilar o plugin.

---

## 16. TEST PATTERNS (RIGOR E DETERMINISMO)

A confiabilidade do sistema é garantida por padrões de teste industriais.

### 16.1 Deterministic Physics Ticking

- **Regra:** Testes de tempo (Cooldowns/Duração) devem ser validados via `physics_process` em passos fixos (Ticks).
- **Mocking do Tempo:** Em testes unitários, simulamos a passagem do tempo chamando `tick(delta)` manualmente para garantir que 1.0s seja exatamente 1.0s, independente do lag do hardware.

### 16.2 Isolation & Mocking

- **Mock Assets:** Para testar habilidades complexas, criamos Resources temporários em memória via código no `TEST_CASE`.
- **Dummy Actors:** O uso de nodes simples com `ASComponent` é preferível a carregar cenas complexas (`.tscn`) para testes unitários, garantindo velocidade de execução.

### 13.3 State Injection

- **Padrão:** Em vez de esperar 10 segundos para testar o fim de um efeito, o teste deve injetar um Spec com `duration_remaining = 0.1` e validar o próximo tick.

### 13.4 Signal Auditing

- **Padrão:** Todo teste de ativação deve auditar se o sinal correspondente (ex: `ability_activated`) foi emitido com os argumentos corretos, garantindo que o feedback visual (Cues) também funcione.
