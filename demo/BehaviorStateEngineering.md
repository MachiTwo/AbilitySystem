# Behavior State Engineering: O Plugin de Comportamento

> **Contexto:** Documentação técnica do plugin de Máquina de Estados Orientada a Recursos (ROP) do projeto Godot MBA.
> **Status:** Arquitetura Definida.

Este documento detalha a arquitetura do sistema de comportamento, focado em dados (Resources), performance (O(K)) e desacoplamento.

---

## 1. Visão Geral da Arquitetura

O sistema inverte a lógica tradicional de State Machines. Em vez de nós com scripts pesados (`Idle.gd`, `Run.gd`), usamos **Dados** para definir comportamento.

- **Resource-Oriented:** Estados são arquivos `.tres` que podem ser trocados, compartilhados e editados visualmente.
- **Context-Driven:** O personagem não "escolhe" um estado. Ele define um **Contexto** (ex: "Estou no chão", "Apertei Pular") e a Machine decide qual o melhor estado para aquela situação.
- **Score System:** A decisão é baseada em um sistema de pontuação, permitindo que estados mais específicos (ex: "Ataque com Espada de Fogo") ganhem prioridade sobre genéricos (ex: "Ataque Básico").

---

## 2. Estrutura de Classes

### 2.1. O Cérebro e o Corpo

#### `class_name Machine extends Node`
O orquestrador do sistema.
- **Responsabilidade:** Gerenciar o estado atual, processar inputs de contexto e executar a lógica de transição.
- **Funcionamento:** Mantém um dicionário de Contexto (`{"Motion": RUN, "Ground": TRUE}`). A cada frame (ou evento), consulta o **Compose** para encontrar o melhor estado compatível.
- **Não Linear:** Não usa `switch/case`. Usa um algoritmo de busca otimizado.

#### `class_name CharacterSheet extends Resource`
*(Anteriormente Stats)*
A ficha de personagem.
- **Responsabilidade:** Armazenar atributos vitais (HP, Stamina, Mana) e estatísticas de movimento (Velocidade, Pulo).
- **Uso:** Injetado na `Machine` para validar requisitos (ex: "Tenho Stamina suficiente para esse ataque?").

### 2.2. Os Estados e Composição

#### `class_name State extends Resource`
A unidade atômica de comportamento.
- **Dados Visuais:** SpriteSheet, Animation, hframes, vframes.
- **Dados Espaciais:** Hitbox (Vector2), Pivot (Vector2), Scene Spawners (para projéteis).
- **Lógica de Filtro:**
    - **Requisitos de Entrada:** O que é necessário para entrar? (ex: `req_weapon: SWORD`, `req_motion: RUN`).
    - **Requisitos de Manutenção:** O que é necessário para continuar?
- **Lógica de Execução:** O que acontece durante o estado? (Velocidade, Dano, Tags aplicadas).

#### `class_name Compose extends Resource`
O "Deck" de estados.
- **Responsabilidade:** Agrupar múltiplos `States` em um único pacote.
- **Indexação:** Ao carregar, cria um **Hash Map Invertido** para busca O(K).
    - *Exemplo:* Em vez de iterar 100 estados, ele olha na chave `ATTACK` e encontra apenas os 3 ataques relevantes.

### 2.3. O Sistema de Itens

#### `class_name Item extends Resource`
A definição base de qualquer objeto no inventário.
- **Propriedades:** Nome, Ícone, Peso.
- **Comportamento:** Pode carregar um `Compose`. Segurar um item pode adicionar novos estados ao personagem (ex: Segurar uma Tocha adiciona "Attack_Torch").

#### `class_name Weapon extends Item`
Especialização para combate.
- **Propriedades:** Dano Base, Velocidade de Ataque, Tipo de Dano.
- **States:** Geralmente traz um `Compose` rico em ataques (Combo, Heavy, Air Attack).

#### `class_name Hotbar extends Resource`
Gerenciador de acesso rápido.
- **Responsabilidade:** Armazenar uma lista de `Items` e controlar qual está ativo.
- **Integração:** Quando o item ativo muda, a `Machine` recebe o `Compose` desse item e atualiza suas possibilidades de ação.
- **Fallback (Unharmed):** Se o item selecionado não tiver estados para a ação (ex: tentar atacar com uma Poção), o sistema busca no `Compose` padrão (Desarmado/Unharmed).

### 2.4. Progressão e Efeitos

#### `class_name Skill extends Resource`
Habilidades desbloqueáveis.
- **Funcionamento:** Semelhante a um Item, mas permanente. Adiciona `States` passivos ou ativos ao `Compose` do jogador.

#### `class_name SkillTree extends Resource`
Estrutura de dependência de Skills.
- **Responsabilidade:** Definir pré-requisitos e caminhos de evolução.

#### `class_name Effect extends Resource`
Modificadores temporários ou instantâneos.
- **Exemplos:** Buff de Dano, DoT (Damage over Time), Stun.
- **Aplicação:** Estados podem aplicar Efeitos ao entrar (Buff) ou ao acertar um alvo (Debuff).

### 2.5. Entidades e Cena

#### `class_name Player extends CharacterBody2D`
O avatar do jogador.
- **Responsabilidade:** Mover-se, colidir e renderizar.
- **Integração:** Possui um nó filho `PlayerMachine` que controla seu comportamento. O Player em si é "burro", apenas obedece aos comandos da Machine (ex: `move_and_slide()`).

#### `Level extends Node2D`
A cena do jogo.
- **Responsabilidade:** Conter o mapa, inimigos e o Player.

#### `Hud extends Control`
A interface do usuário, atualmente focada em **Debug**.
- **Visualização de Estado:** Mostra o nome do estado atual e o tempo decorrido.
- **Monitor de Contexto:** Exibe em tempo real os valores de todas as tags de contexto (Motion, Jump, Attack, Weapon, Physics, Status).
- **Lista de Compose:** Lista todos os estados disponíveis no Item atual, destacando o estado ativo e mostrando um contador de uso (quantas vezes cada estado foi ativado).
- **Barras:** Exibe Vida e Stamina sincronizadas com o `CharacterSheet`.

#### `Camera extends Camera2D`
A câmera do jogo.
- **Responsabilidade:** Seguir o Player com suavização (Smoothing).

---

## 3. O Algoritmo de Decisão (The Brain)

O coração do sistema é o método de busca não-linear.

### 3.1. Busca O(K) (Hash Map Invertido)
Para evitar iterar sobre 500 estados a cada frame (O(N)), o `Compose` organiza os estados em "Buckets" baseados em suas chaves primárias (ex: `Motion`, `Attack`).

1. **Input:** Jogador aperta "Ataque".
2. **Lookup:** A Machine pede ao Compose: "Me dê os estados na gaveta `ATTACK`".
3. **Retorno:** O Compose retorna apenas 3 ou 4 estados, não 500. (Complexidade O(K), onde K é o número de estados naquele bucket).

### 3.2. Sistema de Score (Pontuação)
Como decidir entre dois estados possíveis? (ex: "Ataque Básico" vs "Ataque Correndo").

Cada estado recebe pontos baseados no **Contexto Atual**:
- **Match Exato:** +10 Pontos (ex: O estado pede `Motion: RUN` e o personagem está `RUN`).
- **Match Genérico (ANY):** +0 Pontos (ex: O estado aceita qualquer movimento).
- **Prioridade Manual:** +N Pontos (definido pelo Designer).
- **Chain (Combo):** +Pontos se for a continuação correta de um estado anterior.

**Vencedor:** O estado com maior pontuação é ativado.

### 3.3. Fallback System
Se nenhum estado do Item atual servir (ex: Item "Poção" não tem ataque), o sistema automaticamente verifica o `Compose` base do personagem (Unharmed). Isso garante que o jogador nunca fique "travado" sem ação.
