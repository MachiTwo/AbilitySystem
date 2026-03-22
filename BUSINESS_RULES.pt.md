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

### 2.1 Regras de Ouro das Tags (O que eu SOU)

- **Papel:** Representar estados contínuos, características e bloqueios (ex: `State.Dead`, `Status.Stunned`).
- **Natureza:** Persistentes. Requerem adição formal (`add_tag`) e remoção formal (`remove_tag`). Consomem tempo de CPU no cálculo de RefCounts no `ASTagSpec`.
- **Pergunta que responde:** _"Neste exato microssegundo, este ator está sob a condição X?"_
- **Proibição Absoluta (Anti-Padrão):** NUNCA usar tags para representar ocorrências instantâneas (ex: NÃO usar `State.JustGotHit`). Se a condição dura apenas 1 frame ou serve para alertar outros sistemas, DEVE ser um Evento, nunca uma Tag.

### 2.2 Os 3 Tipos Canônicos de Tag (Tag Types)

O `Tag Type` define o **papel semântico** que uma tag tem no sistema. É o campo que determina como o Singleton e o Editor tratam aquele identificador.

| Tipo          | Prefixo Convencional | Papel                                                                                                                 | Exemplo                                     |
| ------------- | -------------------- | --------------------------------------------------------------------------------------------------------------------- | ------------------------------------------- |
| `NAME`        | `Char.` / `Team.`    | Identidade e categorização estática do ator                                                                           | `Char.Warrior`, `Team.Blue`                 |
| `CONDITIONAL` | `State.` / `Status.` | Estado persistente de gameplay que pode ser adicionado/removido do ator                                               | `State.Stunned`, `Status.Poisoned`          |
| `EVENT`       | `Event.`             | Identificadores de ocorrências. Registrados para autocomplete e sem typos, mas **o payload jamais sobe ao Singleton** | `Event.Weapon.Hit`, `Event.Damage.Critical` |

### 2.2.1 Tag Groups (Organização Visual)

**Tag Groups não são entidades de código.** São uma convenção editorial que emerge automaticamente da hierarquia de pontos (`.`) no identificador da tag.

- O `ASTagsPanel` renderiza as tags como uma **árvore**, usando cada segmento separado por `.` como nó pai.
- `State.Stunned`, `State.Dead` e `State.Frozen` aparecem automaticamente agrupados sob o nó visual `State`.
- Não existe um objeto `TagGroup` em C++ — o "grupo" é apenas o prefixo compartilhado.
- **Convenção obrigatória:** O prefixo raiz de uma tag DEVE refletir seu `Tag Type` (ex: tags `Event.*` são sempre `TAG_TYPE_EVENT`; nunca misturar).

### 2.3 Avaliação Lógica (Predicados)

O sistema suporta 4 estados lógicos em Blueprints (Ability, Effect, Cue) na hora de avaliar os requisitos e bloqueios de um alvo:

- `Required All` (AND): Sucesso se possuir TODAS.
- `Required Any` (OR): Sucesso se possuir PELO MENOS UMA.
- `Blocked Any` (OR): Falha se possuir QUALQUER UMA.
- `Blocked All` (AND): Falha se possuir TODAS SIMULTANEAMENTE.

> [!NOTE]
> Predicados funcionam exclusivamente sobre tags `CONDITIONAL`. Tags do tipo `NAME` e `EVENT` não entram em listas de requisito/bloqueio de blueprints. O Editor enforce isso automaticamente via `ASInspectorPlugin`.

---

## 3. O SISTEMA NERVOSO: EVENTS E HISTÓRICO (A "AÇÃO")

Diferente das Tags, os **AS Events** são mensageiros instantâneos (sinais transitórios) que fluem pelo sistema informando acontecimentos cruciais. Eles impedem o espaguete de código e eliminam a prática destrutiva de adicionar/remover tags no mesmo tick.

### 3.1 Regras de Ouro dos Events (O que ACONTECEU)

- **Papel:** Representar ocorrências pontuais e informativas (ex: `Event.Weapon.Hit`, `Event.Ability.Failed`, `Event.Damage.Critical`).
- **Natureza:** Voláteis/Fugazes. São disparados (`dispatch_event`) e processados pelo barramento de eventos instantaneamente.
- **Pergunta que responde:** _"O que acabou de acontecer, onde, e quão forte foi?"_

### 3.2 O Payload do Evento (`ASEvent`)

Um evento jamais é apenas um nome. O poder reside no seu Payload (`ASEvent`), transmitindo contexto completo:

- `event_tag`: O identificador exato da ocorrência (ex: `Event.Interrupt`).
- `instigator`: O Node que causou (ofensor).
- `target`: O Node afetado (vítima).
- `magnitude`: Intensidade base do evento (Poder do impacto).
- `custom_payload`: Dicionário GDScript/Variant para transporte de arrays e metadados customizados.
- `timestamp`: Registrado nativamente em milissegundos precisos.

### 3.3 A Memória de Curto Prazo (Events Historical)

Eventos morrem em 1 tick, mas sua _"memória"_ persiste de forma levíssima:

- O `ASComponent` mantém um `_event_history` (buffer circular C++ super otimizado de até 64 entradas).
- **Como utilizar na prática:** Componentes reativos (como _Parry_ ou _Counter-Attack_) não precisam estar no estado eterno de "parrying". Podem checar o passado recente: `has_event_occurred(&"Event.Damage.Block", 0.4)`. Se o bloqueio aconteceu no último 0.4s, autorize a habilidade de contra-ataque.
- **Regra:** Nunca usar este cache para persistir história (missões, quests). Use exclusivamente para frames de reatividade temporal.

### 3.4 O Padrão de Registro Dividido (Split Registry)

Os identificadores de Evento (ex: `Event.Weapon.Hit`) **são registrados no Singleton** como qualquer outra tag — para funcionar no autocompletar do editor, no `ASTagsPanel` e evitar erros de digitação dos designers. A diferença está no tipo: eles são cadastrados como `Tag Type = EVENT`.

O que **nunca** sobe ao Singleton é a **instância de dados** — o struct `ASEvent` com `instigator`, `magnitude`, e `custom_payload`. Essa separação configura o padrão de **Registro Dividido**:

- **Singleton (Registro):** Conhece o _nome_ `Event.Weapon.Hit`. Garante que existe, que tem o tipo certo e aparece no autocomplete.
- **ASComponent (Ocorrência):** Conhece o _acontecimento_. Sabe quem bateu, em quem, com qual força e em qual tick. O Singleton não precisa — nem deve — saber disso.

> [!IMPORTANT]
> **Regra de Ouro:** Nunca chame `dispatch_event` com um `event_tag` que não esteja registrado no Singleton com `Tag Type = EVENT`. Isso seria equivalente a usar um `StringName` digitado à mão sem validação — o campo exato de erros que o sistema foi projetado para eliminar.

### 3.5 Como Emitir um Evento (API e Contrato de Uso)

**Events são sempre imperativos — nunca automáticos.** O sistema não observa passivamente efeitos ou habilidades e emite events por conta própria. Quem emite é o código (GDScript, C++ ou `ASDelivery`).

#### API de Emissão

```gdscript
# Assinatura: dispatch_event(event_tag, instigator, magnitude, custom_payload)
var asc: ASComponent = target.get_node("ASComponent")
asc.dispatch_event(&"Event.Weapon.Hit", self, 35.0, {})
```

#### Quem pode emitir:

| Origem                        | Quando usar                                                            |
| ----------------------------- | ---------------------------------------------------------------------- |
| **GDScript / C++ direto**     | Colisões físicas, inputs do jogador, lógica customizada                |
| **ASDelivery**                | Emite automaticamente o event do `ASPackage.tag` ao colidir com um ASC |
| **ASAbility (via GDVirtual)** | `_on_activate_ability` pode chamar `dispatch_event` no momento certo   |

#### Events e Resources — a Distinção Fundamental

- **Emissão imperativa (código):** Sempre que você precisar de controle total (colisão física, input, lógica condicional), chame `dispatch_event` diretamente.
- **Emissão declarativa (Resource):** Um Resource pode **declarar intenção** de quais events devem ser disparados em momentos canônicos do seu ciclo de vida. O executor (`ASComponent`, `ASDelivery`) lê essa lista e chama `dispatch_event` na hora certa.
- **Reação:** Um `ASAbility` Resource **PODE reagir** a eventos registrando um `TRIGGER_ON_EVENT` na lista de `triggers`.
- **Resumo:** O Resource é o **declarante/reator**. O executor é sempre o **emissor**.

### 3.6 O Padrão de Declaração de Eventos (Event Declaration Pattern)

Para tornar o sistema data-driven sem quebrar nenhuma regra arquitetural, os Resources **declaram** quais events são relevantes para o ator e em quais momentos devem ser emitidos. O executor (`ASComponent`, `ASDelivery`) ainda é o único a chamar `dispatch_event` — mas lê a intenção do Resource.

> [!NOTE]
> O Resource **nunca** chama `dispatch_event` diretamente. Ele apenas declara listas de `StringName` com tags `EVENT`. O executor itera essas listas e despacha.

#### Tabela 1 — Inscrição: Quem escuta quais eventos?

Esta é a responsabilidade do `ASContainer`. Sem declarar os eventos aqui, o `ASComponent` não saberá quais event tags roteará para os seus `TRIGGER_ON_EVENT`. É o registro de "este ator é sensível a estes eventos".

| Resource      | Propriedade                 | Papel                                                                                                                                                                      |
| ------------- | --------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ASContainer` | `events: Array[StringName]` | Declara quais tags `EVENT` o ator está inscrito. Quando o ASComponent é inicializado por este Container, passa a rotear esses eventos para triggers e o historical buffer. |

> [!IMPORTANT]
> Se `Event.Weapon.Hit` não estiver no `ASContainer` do arqueiro, uma habilidade `Counter-Shot` com `TRIGGER_ON_EVENT = Event.Weapon.Hit` **jamais será ativada**, mesmo que alguém chame `dispatch_event(&"Event.Weapon.Hit", ...)` no componente. O Container é o manifesto de sensibilidade do ator.

#### Tabela 2 — Emissão: Quando cada Resource dispara um evento?

Estes são os momentos canônicos onde o executor lê a lista declarada no Resource e emite os eventos automaticamente.

| Resource    | Propriedade                             | Momento do Disparo                        | Executor      |
| ----------- | --------------------------------------- | ----------------------------------------- | ------------- |
| `ASPackage` | `events_on_deliver: Array[StringName]`  | Quando `ASDelivery` entrega ao ASC        | `ASDelivery`  |
| `ASAbility` | `events_on_activate: Array[StringName]` | Quando a habilidade é ativada com sucesso | `ASComponent` |
| `ASAbility` | `events_on_end: Array[StringName]`      | Quando a habilidade termina/expira        | `ASComponent` |
| `ASEffect`  | `events_on_apply: Array[StringName]`    | Quando o efeito é aplicado ao ator        | `ASComponent` |

#### Exemplo: Fluxo completo 100% data-driven

```
# 1. ASContainer do "Arqueiro" declara que ele escuta:
ASContainer.events = [&"Event.Weapon.Arrow.Hit", &"Event.Damage.Taken"]

# 2. ASPackage da "Flecha" declara que ela emite ao colidir:
ASPackage.events_on_deliver = [&"Event.Weapon.Arrow.Hit"]

# 3. ASAbility "Counter-Shot" reage ao evento:
ASAbility.triggers = [{tag: &"Event.Weapon.Arrow.Hit", type: TRIGGER_ON_EVENT}]
```

Resultado: Arqueiro é acertado por uma flecha → `ASDelivery` entrega o package → emite `Event.Weapon.Arrow.Hit` → `Counter-Shot` ativa automaticamente. **Nenhuma linha de código no projeto.**

#### Regra de Validação

O editor (via `ASInspectorPlugin`) deve garantir que todas as propriedades `events` e `events_on_*` só aceitem tags do tipo `TAG_TYPE_EVENT` no dropdown.

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

## 6. OS EXECUTORES: SPECS (O "COMO")

Localizados em `src/core/`. Onde o estado e a lógica de execução residem.

- **Papel:** Representar a **Instância Ativa**. É o dono do **"Agora"**.
- **Regra de Ouro: SOBERANIA DE ESTADO.**
- **O que deve viver aqui (e não no Component):**
  - `duration_remaining`: O timer individual de cada instância.
  - `stack_count`: Quantas vezes este efeito específico está acumulado.
  - `calculate_...`: Lógica de cálculo que depende de atributos variáveis (ex: dano baseado em força atual).
- **Responsabilidade:** O Spec deve saber se "terminou" ou não. O Component apenas pergunta a ele.

### 6.1 ASAbilitySpec & ASEffectSpec (Execução)

- **ASAbilitySpec - Papel:** Habilidade em execução ativa ou equipada.
- **ASAbilitySpec - Regra:** Gerencia o cooldown individual e o estado de ativação.
- **ASEffectSpec - Papel:** Instância ativa de um modificador.
- **ASEffectSpec - Regra:** Detém a soberania sobre o tempo restante (`duration`) e pilhas (`stacks`).

### 6.2 ASCueSpec & ASTagSpec (Feedback e Identidade)

- **ASCueSpec - Papel:** Gerenciador do ciclo de vida de um feedback na cena.
- **ASCueSpec - Regra:** Garante a limpeza (Queue Free) do Node instanciado após o término.
- **ASTagSpec - Papel:** Contador de referências (Refcount) para Tags.
- **ASTagSpec - Regra:** Garante que uma Tag só saia do ator quando todos os seus Specs de origem expirarem.

### 6.3 ASAbilitySpec (Gestão de Fases)

- **Papel:** Gerencia o índice da fase atual e a progressão temporal entre os estágios definidos no `ASAbility`.
- **Regra:** Deve ser capaz de avançar para a próxima fase via tempo ou via recebimento de um `ASEvent` específico.

---

## 7. O ORQUESTRADOR: COMPONENT (O HUB)

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
- **ASEventsHistorical (Memória de Eventos):**
  - **Papel:** O ASC mantém um buffer circular de eventos recentes para permitir lógica condicional baseada no passado imediato.
  - **Regra de Ouro:** Não deve ser usado para persistência de longo prazo. É um "cache de reatividade".
  - **Consultas (Queries):** Permite perguntar: "Ocorreu o evento X nos últimos Y ticks?".
  - **Sincronia:** Assim como o `ASStateCache`, o histórico de eventos deve ser sensível ao `tick` temporal para garantir consistência em situações de Rollback.

---

## 8. SISTEMAS DE ENTREGA E REATIVIDADE

### 8.1 ASDelivery (Payload Injections)

- **Papel:** Desacoplar o emissor do alvo em interações espaciais (projéteis, AoEs).
- **Regra:** Transporta um `ASPackage` e injeta o conteúdo ao colidir com um ASC.

### 8.2 Ability Triggers (Automação Reativa)

- **Papel:** Permitir ativação automática de habilidades baseada em eventos de estado (Tags) ou eventos transitórios (AS Events).
- **Regra:** Ativação baseada exclusivamente em `ON_TAG_ADDED`, `ON_TAG_REMOVED` ou `ON_EVENT_RECEIVED`.

---

## 9. REPLICAÇÃO E PERSISTÊNCIA (DETERMINISMO)

O Ability System é projetado para multiplayer autoritativo com suporte a Predição e Rollback. O estado de um Ator em um determinado momento (Tick) é gerido por dois mecanismos sincronizados:

- **Fonte de Verdade (Physics Only):** O `tick` é o único identificador temporal válido. O `ASComponent` opera **exclusivamente** via `physics_process`. O uso de `_process` (Idle/Frame) é terminantemente proibido para lógica de gameplay para garantir determinismo entre instâncias e suporte a Rollback.

### 9.1 ASStateSnapshot (O Recurso Pesado)

- **Papel:** Persistência de longo prazo (Save/Load) e sincronização externa de "Diferencial de Estado".
- **Natureza:** É um **Godot Resource** (`.tres`). Alocado na Heap, suporta serialização nativa.
- **Regra de Uso:** Reservado exclusivamente para **Players** (Playable Characters) ou estados que precisam sobreviver a reinicializações de cena.
- **SSOT:** É o único recurso autorizado a ser mutável em runtime para fins de captura de estado completo.

### 9.2 ASStateCache (A Estrutura Leve)

- **Papel:** Memória de curto prazo para Predição, Reconciliação e NPCs.
- **Natureza:** **Struct C++ pura**. Alocada em stack/inline dentro de um buffer circular (`Vector`).
- **Regra de Uso:** Deve ser usado para manter o histórico recente de ticks (ex: últimos 64-128 ticks) para cálculos de rede.
- **Vantagem:** Zero overhead de alocação de Resource. Ideal para sincronização rápida de entidades não-jogáveis (NPCs/Inimigos).

### 9.3 Fluxo de Ativação em Rede e Determinismo

1. **Request:** O cliente solicita a ativação chamando `request_activate_ability(tag)`.
2. **Predict:** O cliente executa localmente a ação para latência zero e gera uma entrada no `cache_buffer` via `capture_snapshot()`. Se for um Player, o `ASStateSnapshot` também é atualizado.
3. **Confirm/Correct:** O servidor valida o request e responde. Se houver divergência, o servidor envia o estado autoritativo. O cliente então realiza o **Rollback** buscando o tick correspondente no `cache_buffer` para restaurar atributos e tags instantaneamente.
4. **Determinismo:** Lógicas de gameplay (Magnitude de dano, custos) devem ser puras e basear-se exclusivamente nos dados contidos no ASC e seus Specs para garantir que o mesmo input gere o mesmo output em todas as instâncias.

---

## 10. O PROTOCOLO DE REATIVIDADE E FLUXOS (A MATRIZ TRICROMÁTICA)

Para evitar que a arquitetura decline e se torne um emaranhado caótico onde todos os sistemas interferem uns nos outros, estabelecemos o _Protocolo de Reatividade_. Esta é a doutrina de como os 3 Pilares operam em uníssono orquestrado.

### 10.1 A Ordem Natural

1. **INPUT/AÇÃO:** Uma interação, término temporizador ou impacto físico emite um **AS Event** (`Event.Damage`).
2. **ESCUTA/PROCESSAMENTO:** Uma Entidade escuta via Triggers (`ON_EVENT`).
3. **MUTAÇÃO:** A habilidade reativa acerta os requisitos, invoca e aplica o mutador (`ASEffect`).
4. **ESTADO (Fim do Ciclo):** O Effect mutou os atributos ou adicionou permanentemente a **AS Tag** (`State.Stunned`).

> [!CAUTION]
> **Erros Fatais punidos com refatoração profunda:**
>
> - Esperar que uma habilidade inicie baseada em "perda de tag". (Isso é sintoma de acoplamento de estado; dispare um Evento avisando o fim).
> - Se uma Habilidade falhar num requisito de Tag ou Custo, NUNCA gerencie estado (aplicar tags temporárias). Emita o gatilho `Event.Ability.Failed` relatando por qual motivo (Dicionário Payload), para loggers ou UI reagirem.

### 10.2 Triggers na Era Híbrida

A ativação de Habilidades através do _Ability Triggers_ no Spec passou por revisão de hierarquia. Pode-se construir automação pura através deles:

- `TRIGGER_ON_EVENT`: É o padrão ouro para responsidade de combate (Reagir instantaneamente no impacto).
- `TRIGGER_ON_TAG_ADDED` / `REMOVED`: É o padrão para automação de ambiente (Ligar aura de lentidão quando entrar na água).

### 10.3 O Pacto do ASDelivery

O componente `ASDelivery` (ex: um míssil ou aura rastreado) carrega o envelope mortífero `ASPackage`.

- **Regra e Obrigação:** Todo ASDelivery, ao concluir rota e aplicar pacote a um ASC alvo, DEVE obrigatoriamente emitir o disparo de Evento invocando `target_asc->dispatch_event(package_tag)`.
- Isso assegura que "tomar uma fireball na cara" automaticamente preencha a memória temporal do ASC alvo (`ASEventHistorical`), habilitando bloqueios ou triggers reativos purificados.

---

## 11. RIGOR TÉCNICO E QUALIDADE DE TESTES

### 11.1 Padrão 300% (Lei de Ferro)

Cada funcionalidade deve ser provada por pelo menos **3 variações** no mesmo teste:

1. **Happy Path:** Cenário base ideal.
2. **Negative:** Entrada inválida ou falha esperada.
3. **Edge Case:** Combinações complexas (multi-tags, limites de borda).

### 11.2 Suítes de Teste

- **Core (Unit):** Atômicos, sem efeitos colaterais.
- **Advanced (Integration):** DoT/HoT periódico, fluxos de RPG complexos.
- **Multiplayer (Simulation):** Executado via `utility/multiplayer/runner.py` com latência injetada.

---

## 12. PADRÕES DE NOMENCLATURA DA API (ESPECÍFICOS DO AS)

Para manter a consistência, toda a API pública deve seguir estes padrões próprios do Ability System:

### 11.1 Prefixos de Métodos

Os métodos são categorizados pela sua intenção e camada de acesso:

- **🎮 Camada de Gameplay (Uso para Lógica de Jogo)**
  - `try_activate_...`: **Execução Segura.** Tenta disparar uma lógica que depende de requisitos prévios. Integra a verificação e a ação. **Uso obrigatório para habilidades e efeitos.**
  - `can_...`: **Pré-autorização.** Avalia se uma ação pode ser executada sem iniciá-la.
  - `is_...`: **Consulta de Status.** Verifica condições booleanas de estado ou identidade (ex: `is_ability_active`).
  - `has_...`: **Consulta de Posse.** Verifica se o objeto detém uma chave específica (ex: `has_tag`).
  - `get_...`: **Extração de Informação.** Obtém valores, referências ou metadados de leitura.
  - `cancel_...`: **Interrupção.** Encerra voluntariamente um fluxo em execução.
  - `request_...`: **Intenção em Rede.** Solicita a execução de uma ação via RPC (Multiplayer).
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

### 11.2 Sufixos de Origem e Contexto

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

### 11.3 Argumentos e Tipagem

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

### 11.4 Sinais (Signals)

Sinais devem comunicar eventos que **já ocorreram**, seguindo o padrão de voz passiva:

- **Formato:** `snake_case` no tempo passado.
- **Exemplos Corretos:** `ability_activated`, `effect_removed`, `attribute_changed`, `tag_event_received`.
- **Exemplos Incorretos:** `on_ability_activate` (prefixo desnecessário), `activate_ability` (confunde com método).

### 11.5 Membros Internos e Propriedades (C++)

Para garantir segurança e legibilidade no código-fonte GDExtension:

- **Variáveis Privadas/Protegidas:** Devem obrigatoriamente começar com `_` (underscore). Ex: `_attribute_set`, `_is_active`.
- **Getters/Setters Reativos:** Sempre que uma mudança de variável exigir uma reavaliação (ex: mudar a tag exige recontar o ASTagSpec), deve-se usar um setter formal em vez de acesso direto.
- **Propriedades Públicas:** Devem espelhar os nomes brutos (sem `_`) para serem expostas corretamente ao Inspetor do Godot.
- **Dual Build:** Todo código de teste deve suportar a compilação via `#ifdef ABILITY_SYSTEM_GDEXTENSION`.

---

## 13. DESIGN PATTERNS (C++/GDEXTENSION)

O sistema utiliza padrões clássicos adaptados para a arquitetura de alta performance da Godot Engine.

### 12.1 Spec Pattern (Resource vs. Instance)

- **O Problema:** Resources Godot são compartilhados. Modificar um afetaria todos os inimigos do mesmo tipo.
- **A Solução:** Separação total entre **Resource** (Dados Imutáveis/DNA) e **Spec** (Estado de Runtime/Instância).
- **Regra:** Toda lógica que exige alteração de valor (timers, stacks) deve residir no Spec. O Resource é apenas um Provedor de Dados.

### 12.2 Flyweight Pattern (Memória Otimizada)

- **Conceito:** Milhares de atores compartilham as mesmas referências de `ASAbility` e `ASEffect`.
- **Implementação:** O `ASComponent` armazena apenas ponteiros (RefCounters) para os Resources. Os dados pesados (ícones, curvas de dano) nunca são duplicados na memória.

### 12.3 Command Pattern (Abilities)

- **Conceito:** Cada habilidade é um comando auto-contido que sabe como iniciar, executar e cancelar.
- **Requisito:** Encapsulamento total. O componente não deve conhecer a lógica interna da habilidade; ele apenas despacha o comando.

### 12.4 Data-Driven Design

- **Regra:** O comportamento deve ser definido em arquivos `.tres` no Editor, não em código rígido (Hardcoded).
- **Vantagem:** Permite que designers alterem o balanceamento sem recompilar o plugin.

---

## 14. TEST PATTERNS (RIGOR E DETERMINISMO)

A confiabilidade do sistema é garantida por padrões de teste industriais.

### 13.1 Deterministic Physics Ticking

- **Regra:** Testes de tempo (Cooldowns/Duração) devem ser validados via `physics_process` em passos fixos (Ticks).
- **Mocking do Tempo:** Em testes unitários, simulamos a passagem do tempo chamando `tick(delta)` manualmente para garantir que 1.0s seja exatamente 1.0s, independente do lag do hardware.

### 13.2 Isolation & Mocking

- **Mock Assets:** Para testar habilidades complexas, criamos Resources temporários em memória via código no `TEST_CASE`.
- **Dummy Actors:** O uso de nodes simples com `ASComponent` é preferível a carregar cenas complexas (`.tscn`) para testes unitários, garantindo velocidade de execução.

### 13.3 State Injection

- **Padrão:** Em vez de esperar 10 segundos para testar o fim de um efeito, o teste deve injetar um Spec com `duration_remaining = 0.1` e validar o próximo tick.

### 13.4 Signal Auditing

- **Padrão:** Todo teste de ativação deve auditar se o sinal correspondente (ex: `ability_activated`) foi emitido com os argumentos corretos, garantindo que o feedback visual (Cues) também funcione.
