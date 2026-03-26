# State Engineering

> **Arquitetura de Comportamento Orientada a Contexto e Recursos (ROP).**

O **State Engineering** é um framework para Godot 4.5+ que substitui Máquinas de Estado Finitas (FSM) tradicionais por um sistema dinâmico de **Seleção de Comportamento**. Em vez de hardcodar transições (`if state == WALK: animation = "RUN"`), o sistema avalia o **Contexto** atual e escolhe o melhor **Estado (Resource)** para aquele momento.

## 🌟 Filosofia: "Query, Don't Transition"

Em uma FSM tradicional, você define **Transições**:

> _"Se estou andando e aperto Shift, vou para Correr."_

No State Engineering, você define **Requisitos**:

> _"O estado Correr requer que o input 'Run' esteja ativo."_ > _"O estado Andar requer que o input 'Run' NÃO esteja ativo."_

A **Machine** (Cérebro) olha para o Contexto atual (Inputs, Física, Status, Arma, Item) e faz uma "Query" no banco de dados de estados disponíveis para encontrar o **Best Match**.

### Vantagens

- **Desacoplamento Total:** Estados não sabem da existência uns dos outros.
- **Escalabilidade:** Adicione 50 ataques novos apenas criando arquivos `.tres`.
- **Hot-Swapping:** Troque o "Deck" de habilidades (ex: trocar de arma) em tempo real.
- **Duck Typing:** Um estado pode ser `MoveData` e `AttackData` ao mesmo tempo.

---

## 🏗️ Arquitetura Core

O plugin é composto por 4 pilares principais localizados em `addons/state_engineering/core/`.

### 1. `BehaviorStates.gd` (Autoload)

O "Vocabulário" do jogo. Define todos os Enums globais usados para filtrar estados.

- **Motion:** `IDLE`, `WALK`, `RUN`, `DASH`...
- **Physics:** `GROUND`, `AIR`, `WATER`...
- **Weapon:** `KATANA`, `BOW`, `NONE`...
- **Reaction:** `CANCEL`, `ADAPT`, `FINISH`...

> **Nota:** Este arquivo atua como a "Verdade Única" para tipos. Adicionar um tipo aqui atualiza todos os Inspectors do jogo.

Além de Enums, este Autoload gerencia:

- **Física Global:** Funções utilitárias para cálculo de força (`F=ma`) e dano newtoniano.
- **Cache de Fallback:** Mantém estados padrão (ex: `Unarmed`) carregados na memória para quando uma arma não define comportamento específico.
- **Indexador (Hash Map):** Constrói e mantém o índice de busca rápida (`O(1)`) para o sistema de Query, eliminando loops em tempo de execução.

### 2. `State.gd` (Resource)

A unidade fundamental de dados. Um `.tres` que herda de `State` contém **Regras**, **Visual** e **Física**.

- **Filtros (Requirements):** `req_motion`, `req_weapon`, etc. Define _quando_ esse estado pode ser ativado.
- **Cooldowns de Contexto:** `context_cooldown_filter`, `context_cooldown_time`. Impede spam de certos contextos (ex: Dash).
- **Física:** `speed_multiplier`, `jump_force`, `friction`.
- **Combate:** `damage`, `cooldown`, `projectile_scene`.
- **Reações:** `on_damage`, `on_physics_change` (Default: `ADAPT`). Define o que fazer se o contexto mudar abruptamente.

### 3. `Compose.gd` (Resource)

Um container (Lista) de estados. Atua como o "Deck" de habilidades.

- Exemplo: `KatanaMoves.tres` contém `[Slash1.tres, Slash2.tres, JumpAttack.tres]`.
- Itens (`ItemData`) carregam um `Compose` para injetar seus comportamentos no Player.

### 4. `Machine.gd` (Node)

O cérebro que processa a lógica a cada frame.

- **Contexto:** Mantém um Dicionário `context` (ex: `{"Motion": RUN, "Physics": GROUND}`).
- **Algoritmo `find_best_match()`:**
  1. Recebe uma lista de candidatos (do `Compose`).
  2. Filtra quem não atende aos requisitos do Contexto (ex: Estado requer `AIR` mas estamos no `GROUND`).
  3. Pontua os candidatos restantes (Match exato = +1 ponto).
  4. Seleciona o vencedor.

---

## 🎮 Implementação (Machines)

### `PlayerMachine.gd`

Especialização para o Jogador.

- **Input Buffering:** Suporta fila de ações (`_queued_state`) para combos fluidos.
- **Combo System:** Gerencia `combo_step` e janelas de tempo.
- **Reatividade:**
  - `_try_engage_attack()`: Tenta encontrar um ataque válido para o input atual.
  - `_try_engage_move()`: Tenta encontrar um movimento válido (se não estiver atacando).
  - `_handle_reaction()`: Processa interrupções (ex: levar dano, cair no buraco).

### `EnemyMachine.gd` (Exemplo)

Especialização para IA.

- Recebe comandos de alto nível (`update_ai_decision`) e traduz para mudanças de Contexto (`Motion: RUN`, `Attack: NORMAL`).

---

## 🛠️ Guia de Uso

### 1. Criando um Novo Estado

1. No FileSystem, clique com botão direito -> **Create New... -> Resource**.
2. Procure por `MoveData` ou `AttackData`.
3. Configure os **Requisitos**:
   - `Req Motion`: `RUN`
   - `Req Physics`: `GROUND`
4. Configure a **Física**:
   - `Speed Multiplier`: `2.0`

### 2. Configurando o Player

1. Adicione o nó `PlayerMachine` como filho do seu `CharacterBody2D`.
2. No Inspector do `PlayerMachine`, arraste seus `Compose` (ex: `PlayerMoves.tres`).
3. No script do Player (`Player`), conecte os inputs ao Contexto:

   ```gdscript
   if Input.is_action_pressed("run"):
       machine.set_context("Motion", BehaviorStates.Motion.RUN)
   ```

### 3. Criando Itens (Armas)

1. Crie um `ItemData`.
2. Defina os **Modificadores de Contexto**:
   - `Weapon`: `KATANA`
3. (Futuro) Associe um `Compose` específico para a arma.

---

## ⚡ Performance: Query Hash Map (O(1))

Para garantir escalabilidade em jogos complexos, o sistema utiliza uma arquitetura de **Indexação Invertida**.

> **[📄 Documentação Técnica Completa: HashMap.md](HashMap.md)**

### O Problema da Busca Linear

Iterar sobre uma lista de 100 estados a cada frame (`O(N)`) consome CPU desnecessária, especialmente com múltiplos inimigos.

### A Solução Indexada

Ao iniciar (ou trocar de arma), o sistema organiza os estados em "Baldes" (Buckets) baseados em seus gatilhos principais.

- **Query:** "Preciso de um Ataque Rápido".
- **Ação:** O sistema acessa `Index[ATTACK][FAST]` instantaneamente.
- **Resultado:** Retorna apenas os 2 ou 3 ataques relevantes para validação final, ignorando os outros 97.

Isso mantém o custo de processamento baixo e constante (`O(1)`), independente do número de habilidades que o personagem possui.

> **Nota:** Os índices são construídos e salvos no disco pelo `Compose.gd` (agora um script `@tool`), garantindo zero overhead de inicialização.

---

## 📂 Estrutura de Pastas

```
addons/state_engineering/
├── autoload/           # Singletons (BehaviorStates.gd)
├── core/               # Scripts Base (State, Machine, Compose)
├── machines/           # Implementações (PlayerMachine, EnemyMachine)
├── resources/          # Definições de Dados (ItemData, MoveData)
└── example/            # Cena de Demo e Recursos de Exemplo
```
