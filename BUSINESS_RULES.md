# BUSINESS RULES: ABILITY SYSTEM PLUGIN - GOVERNANCE CONTRACT

Este documento estabelece as fronteiras arquiteturais e regras de negócio obrigatórias. Qualquer implementação que viole estes limites deve ser refatorada imediatamente.

---

## 1. IDENTIFICADORES: AS TAGS (DNA DO SISTEMA)

Tags não são classes; são **Identificadores Hierárquicos Superpoderosos** baseados em `StringName`.

### 1.1 Tag (Identificador)

- **Papel:** Representar estados, ações ou categorias (ex: `State.Dead`, `Ability.Fireball`).
- **Regra:** Devem ser tratadas como imutáveis e globais. A comparação deve suportar hierarquia (checar `State` encontra `State.Dead`).
- **Lógica Lógica de Ativação:** O sistema suporta 4 estados lógicos em Blueprints (Ability/Effect/Cue):
  - `Required All` (AND): Sucesso se tiver todos.
  - `Required Any` (OR): Sucesso se tiver pelo menos um.
  - `Blocked Any` (OR): Falha se tiver qualquer um.
  - `Blocked All` (AND): Falha apenas se tiver todos simulatenamente.

### 1.2 Tag Type & Tag Group

- **Tag Type:** Define a natureza técnica da tag (ex: `NAME`, `CONDITIONAL`). Determina como o sistema lida com ela no backend.
- **Tag Group:** Organização lógica exclusiva para o Editor. Agrupa tags relacionadas para facilitar a busca e manipulação visual.

---

## 2. O SINGLETON: ABILITY SYSTEM (INTERFACE DE PROJETO)

- **Papel:** É a **API de Configuração Global** e a ponte com o `ProjectSettings`.
- **Regras de Negócio:**
  - É o único responsável por salvar e carregar a lista global de tags nas configurações do projeto (`project.godot`).
  - Atua como um **Registro Central de Nomes** para garantir que recursos duplicados não entrem em conflito.
  - **Limite:** Não deve armazenar estado de nenhum Actor. Se uma informação pertence a uma instância de personagem, ela **não** deve estar aqui.

---

## 3. CAMADA DE FERRAMENTAS: EDITORES

Interface entre o Humano e os Resources.

### 3.1 ASEditorPlugin

- **Papel:** **Bootloader**.
- **Regra:** Registro de tipos, ícones e inicialização de outros sub-editores. Proibido conter lógica de jogo.

### 3.2 ASTagsPanel

- **Papel:** Interface visual para o **Registro Global**.
- **Regra:** Manipula exclusivamente o dicionário de tags do `AbilitySystem` Singleton.

### 3.3 ASInspectorPlugin (e Property Selectors)

- **Papel:** Contextualização.
- **Regra:** Deve fornecer seletores inteligentes (dropdowns de tags, busca de atributos) para facilitar a configuração de Resources e Components no Inspetor.

---

## 4. OS BLUEPRINTS: RESOURCES (O "O QUE")

Localizados em `src/resources/`. São as **Definições de Dados**.

- **Papel:** Definir as regras de negócio de uma habilidade ou efeito (ex: "Custa 10 MP", "Dura 5 segundos").
- **Regra de Ouro: IMUTABILIDADE.** Um Resource nunca deve mudar seus valores em tempo de execução. Eles são compartilhados entre centenas de instâncias.
- **O que vive aqui:** Valores base, ícones, nomes, tags de requisito e listas de modificadores brutos.

### 4.1 ASAbility & ASEffect (Ações e Modificadores)

- **ASAbility - Papel:** Definir a lógica de uma ação (Custos, Cooldown, Triggers).
- **ASAbility - Regra:** Único Resource capaz de gerenciar requisitos de ativação e custos de atributos através de especificação.
- **ASEffect - Papel:** Modificador de estado (Buffs, Debuffs, Dano).
- **ASEffect - Regra:** Define políticas de empilhamento (Stacking) e magnitudes de mudança nos atributos.

### 4.2 ASAttribute & ASAttributeSet (O Sistema de Atributos)

- **ASAttribute - Papel:** Define os metadados (limites min/max) de uma única estatística.
- **ASAttributeSet - Papel:** Agrupa as estatísticas e define o estado inicial de um personagem. Detém a lógica de modificação de atributos.
- **ASAttributeSet - Regra (Attribute Drivers):** Permite derivar o valor base de um atributo a partir de outro (ex: 2 * Força = 1 Ataque). O recalculo é automático em mudanças de valor base.
- **ASAttributeSet - Regra (Prioridade):** Modificadores (Flat Add, Multiplier) são aplicados *após* o cálculo dos Drivers.

### 4.3 ASContainer & ASPackage (Arquétipos e Payloads)

- **ASContainer - Papel:** Arquétipo completo (Dicionário de Identidade do Ator).
- **ASContainer - Regra:** Atua como o "Template de Fábrica" para inicialização total do `ASComponent`.
- **ASPackage - Papel:** Agrupador de transporte (Envelope de Dados).
- **ASPackage - Regra:** Deve ser usado exclusivamente para transmitir coleções de efeitos e cues via `ASDelivery`.

### 4.4 ASCue (Feedbacks Visuais)

- **Papel:** Feedback audiovisual puro (Animação, Som, Partículas).
- **Regra:** Proibido alterar qualquer dado de gameplay. Deve ser disparado reativamente.

---

## 5. OS EXECUTORES: SPECS (O "COMO")

Localizados em `src/core/`. Onde o estado e a lógica de execução residem.

- **Papel:** Representar a **Instância Ativa**. É o dono do **"Agora"**.
- **Regra de Ouro: SOBERANIA DE ESTADO.**
- **O que deve viver aqui (e não no Component):**
  - `duration_remaining`: O timer individual de cada instância.
  - `stack_count`: Quantas vezes este efeito específico está acumulado.
  - `calculate_...`: Lógica de cálculo que depende de atributos variáveis (ex: dano baseado em força atual).
- **Responsabilidade:** O Spec deve saber se "terminou" ou não. O Component apenas pergunta a ele.

### 5.1 ASAbilitySpec & ASEffectSpec (Execução)

- **ASAbilitySpec - Papel:** Habilidade em execução ativa ou equipada.
- **ASAbilitySpec - Regra:** Gerencia o cooldown individual e o estado de ativação.
- **ASEffectSpec - Papel:** Instância ativa de um modificador.
- **ASEffectSpec - Regra:** Detém a soberania sobre o tempo restante (`duration`) e pilhas (`stacks`).

### 5.2 ASCueSpec & ASTagSpec (Feedback e Identidade)

- **ASCueSpec - Papel:** Gerenciador do ciclo de vida de um feedback na cena.
- **ASCueSpec - Regra:** Garante a limpeza (Queue Free) do Node instanciado após o término.
- **ASTagSpec - Papel:** Contador de referências (Refcount) para Tags.
- **ASTagSpec - Regra:** Garante que uma Tag só saia do ator quando todos os seus Specs de origem expirarem.

---

## 6. O ORQUESTRADOR: COMPONENT (O HUB)

O `ASComponent` (ASC).

- **Papel:** **Gestor de Coleções** e **Roteador de Sinais**.
- **Regras de Negócio:**
  - Não deve gerenciar timers individuais de efeitos (isso é do Spec).
  - Responsável por manter a lista de `active_specs` e `unlocked_specs`.
  - Atua como o **Dono dos Atributos** (via `AttributeSet`).
  - É o único que pode adicionar/remover tags do Actor.
- **Limite:** O ASC não deve saber os detalhes internos de como uma habilidade funciona. Ele apenas diz: `spec->activate()`, `spec->tick(delta)`, `spec->deactivate()`.
- **Node Registry:** O Componente deve manter um registro de aliases de nós (ex: "Muzzle") para que Cues saibam onde instanciar efeitos visuais sem dependências de caminhos de cena.

---

## 7. SISTEMAS DE ENTREGA E REATIVIDADE

### 7.1 ASDelivery (Payload Injections)

- **Papel:** Desacoplar o emissor do alvo em interações espaciais (projéteis, AoEs).
- **Regra:** Transporta um `ASPackage` e injeta o conteúdo ao colidir com um ASC.

### 7.2 Ability Triggers (Automação Reativa)

- **Papel:** Permitir ativação automática de habilidades baseada em eventos de estado (Tags).
- **Regra:** Ativação baseada exclusivamente em `ON_TAG_ADDED` ou `ON_TAG_REMOVED`.

---

## 8. MATRIZ DE RESPONSABILIDADE

| Funcionalidade                 | Resource | Spec | Component | Delivery |
| :----------------------------- | :------: | :--: | :-------: | :------: |
| **Configuração Base**          |   Sim    | Não  |    Não    |   Não    |
| **Timer / Estado**             |   Não    | Sim  |    Não    |   Não    |
| **Armazenamento de Atributos** |   Não    | Não  |    Sim    |   Não    |
| **Injeção em Alvos Externos**  |   Não    | Não  |    Não    |   Sim    |
| **Broadcast de Sinais**        |   Não    | Não  |    Sim    |   Não    |

---

## 9. REGRAS DE OURO (CÓDIGO MORTO E SEGURANÇA)

1. **Anti Vibe-Coding:** Mudanças estruturais exigem atualização deste documento e do `GEMINI.md`.
2. **MD5 Cache:** O build do `godot-cpp` não deve ser invalidado por timestamps.
3. **Depreciação de target_node:** Funções que pedem `Node*` direto no core são proibidas; use `register_node` ou `ASDelivery`.
4. **TDD:** Se não há teste, o recurso não existe.

---
