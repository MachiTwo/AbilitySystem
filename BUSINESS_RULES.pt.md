# BUSINESS RULES: ABILITY SYSTEM - CONTRATO DE GOVERNANÇA

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

## 2. IDENTIFICADORES: AS TAGS (DNA DO SISTEMA)

Tags não são classes; são **Identificadores Hierárquicos Superpoderosos** baseados em `StringName`.

### 2.1 Tag (Identificador)

- **Papel:** Representar estados, ações ou categorias (ex: `State.Dead`, `Ability.Fireball`).

- **Regra:** Devem ser tratadas como imutáveis e globais. A comparação deve suportar hierarquia (checar `State` encontra `State.Dead`).

- **Lógica de Ativação:** O sistema suporta 4 estados lógicos em Blueprints (Ability/Effect/Cue):
  - `Required All` (AND): Sucesso se tiver todos.
  - `Required Any` (OR): Sucesso se tiver pelo menos um.
  - `Blocked Any` (OR): Falha se tiver qualquer um.
  - `Blocked All` (AND): Falha apenas se tiver todos simulatenamente.

### 2.2 Tag Type & Tag Group

- **Tag Type:** Define a natureza técnica da tag (ex: `NAME`, `CONDITIONAL`). Determina como o sistema lida com ela no backend.

- **Tag Group:** Organização lógica exclusiva para o Editor. Agrupa tags relacionadas para facilitar a busca e manipulação visual.

---

## 3. O SINGLETON: ABILITY SYSTEM (INTERFACE DE PROJETO)

- **Papel:** É a **API de Configuração Global** e a ponte com o `ProjectSettings`.

- **Regras de Negócio:**
  - É o único responsável por salvar e carregar a lista global de tags nas configurações do projeto (`project.godot`).
  - Atua como um **Registro Central de Nomes** para garantir que recursos duplicados não entrem em conflito.

- **Limite:** Não deve armazenar estado de nenhum Actor. Se uma informação pertence a uma instância de personagem, ela **não** deve estar aqui.

---

## 4. CAMADA DE FERRAMENTAS: EDITORES

Interface entre o Humano e os Resources.

### 4.1 ASEditorPlugin

- **Papel:** **Bootloader**.

- **Regra:** Registro de tipos, ícones e inicialização de outros sub-editores. Proibido conter lógica de jogo.

### 4.2 ASTagsPanel

- **Papel:** Interface visual para o **Registro Global**.

- **Regra:** Manipula exclusivamente o dicionário de tags do `AbilitySystem` Singleton.

### 4.3 ASInspectorPlugin (e Property Selectors)

- **Papel:** Contextualização.

- **Regra:** Deve fornecer seletores inteligentes (dropdowns de tags, busca de atributos) para facilitar a configuração de Resources e Components no Inspetor.

---

## 5. OS BLUEPRINTS: RESOURCES (O "O QUE")

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

### 5.1 ASAbility & ASEffect (Ações e Modificadores)

- **ASAbility - Papel:** Definir a lógica de uma ação (Custos, Cooldown, Triggers).

- **ASAbility - Regra:** Único Resource capaz de gerenciar requisitos de ativação e custos de atributos através de especificação.

- **ASEffect - Papel:** Modificador de estado (Buffs, Debuffs, Dano).

- **ASEffect - Regra:** Define políticas de empilhamento (Stacking) e magnitudes de mudança nos atributos.

### 5.2 ASAttribute & ASAttributeSet (O Sistema de Atributos)

- **ASAttribute - Papel:** Define os metadados (limites min/max) de uma única estatística.

- **ASAttributeSet - Papel:** Agrupa as estatísticas e define o estado inicial de um personagem. Detém a lógica de modificação de atributos.

- **ASAttributeSet - Regra (Attribute Drivers):** Permite derivar o valor base de um atributo a partir de outro (ex: 2 \* Força = 1 Ataque). O recalculo é automático em mudanças de valor base.

- **ASAttributeSet - Regra (Prioridade):** Modificadores (Flat Add, Multiplier) são aplicados _após_ o cálculo dos Drivers.

### 5.3 ASContainer & ASPackage (Arquétipos e Payloads)

- **ASContainer - Papel:** Arquétipo completo (Dicionário de Identidade do Ator).

- **ASContainer - Regra:** Atua como o "Template de Fábrica" para inicialização total do `ASComponent`.

- **ASPackage - Papel:** Agrupador de transporte (Envelope de Dados).

- **ASPackage - Regra:** Deve ser usado exclusivamente para transmitir coleções de efeitos e cues via `ASDelivery`.

### 5.4 ASCue (Feedbacks Visuais)

- **Papel:** Feedback audiovisual puro (Animação, Som, Partículas).

- **Regra:** Proibido alterar qualquer dado de gameplay. Deve ser disparado reativamente.

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

---

## 8. SISTEMAS DE ENTREGA E REATIVIDADE

### 8.1 ASDelivery (Payload Injections)

- **Papel:** Desacoplar o emissor do alvo em interações espaciais (projéteis, AoEs).

- **Regra:** Transporta um `ASPackage` e injeta o conteúdo ao colidir com um ASC.

### 8.2 Ability Triggers (Automação Reativa)

- **Papel:** Permitir ativação automática de habilidades baseada em eventos de estado (Tags).

- **Regra:** Ativação baseada exclusivamente em `ON_TAG_ADDED` ou `ON_TAG_REMOVED`.

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

## 10. RIGOR TÉCNICO E QUALIDADE DE TESTES

### 10.1 Padrão 300% (Lei de Ferro)

Cada funcionalidade deve ser provada por pelo menos **3 variações** no mesmo teste:

1. **Happy Path:** Cenário base ideal.
2. **Negative:** Entrada inválida ou falha esperada.
3. **Edge Case:** Combinações complexas (multi-tags, limites de borda).

### 10.2 Suítes de Teste

- **Core (Unit):** Atômicos, sem efeitos colaterais.

- **Advanced (Integration):** DoT/HoT periódico, fluxos de RPG complexos.

- **Multiplayer (Simulation):** Executado via `utility/multiplayer/runner.py` com latência injetada.

---

## 11. PADRÕES DE NOMENCLATURA DA API (ESPECÍFICOS DO AS)

Para manter a consistência, toda a API pública deve seguir estes padrões próprios do Ability System:

### 11.1 Prefixos de Métodos

Os métodos são categorizados pela sua intenção e camada de acesso:

- **🎮 Camada de Gameplay (Uso para Lógica de Jogo)**
  - `try_activate_...`: **Execução Segura.** Tenta disparar uma lógica que depende de requisitos prévios. Integra a verificação e a ação. **Uso obrigatório para habilidades e efeitos.**
  - `can_...`: **Pré-autorização.** Avalia se uma ação pode ser executada sem iniciá-la.
  - `is_...`: **Consulta de Status.** Verifica condições booleanas de estado ou identidade (ex: `is_ability_active`).
  - `has_...`: **Consulta de Posse.** Verifica se o objeto detém uma chave específica (ex: `has_tag`).
  - `get_...`: **Extração de Informação.** Obtém valores, referências ou metadados de leitura.
  - `cancel_...`: **Interruption.** Encerra voluntariamente um fluxo em execução.
  - `request_...`: **Intenção em Rede.** Solicita a execução de uma ação via RPC (Multiplayer).

- **🏗️ Infraestrutura/Interna (Uso Restrito ou de Configuração)**
  - `apply_...`: **Aplicação Forçada.** Injeta um payload ou container ignorando regras de ativação. Usado em inicialização ou por sistemas de entrega (`ASDelivery`).
  - `add_...` / `remove_...`: **Mutação de Baixo Nível.** Altera coleções internas. Não deve ser usado como atalho para ativar lógica de jogo (ex: use `try_activate` em vez de tentar "adicionar" um efeito manualmente).
  - `unlock_...` / `lock_...`: **Gestão de Inventário.** Altera a disponibilidade de habilidades no catálogo do ator.
  - `register_...` / `unregister_...`: **Vínculo de Sistema.** Conecta o componente a nós externos ou gerencia o Singleton global.
  - `rename_...`: **Internal Refactoring.** Altera identificadores de Tags e propaga a mudança.
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

## 12. DESIGN PATTERNS (C++/GDEXTENSION)

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

## 13. TEST PATTERNS (RIGOR E DETERMINISMO)

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
