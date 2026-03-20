# BUSINESS RULES: LIMBOAI - GOVERNANCE CONTRACT

Este documento estabelece as fronteiras arquiteturais e regras de negócio obrigatórias para o uso e integração do plugin **LimboAI** (Behavior Trees e Hierarchical State Machines) com o projeto. Qualquer implementação que viole estes limites deve ser refatorada imediatamente.

---

## 1. FILOSOFIA E ENGENHARIA RIGOROSA

O projeto rejeita o **"Vibe-Coding"** (programação por intuição ou sorte). Cada linha de lógica de comportamento em AI deve ser intencional e testável.

### 1.1 Pair Programming e Governança

- **Desapego Radical ao Código:** A árvore de comportamento ou máquina de estados que falha indica uma arquitetura de IA mal definida. A correção acontece via reavaliação de contexto, nunca via "remendos" de task para pular etapas.
- **SSOT (Single Source of Truth):** Este arquivo é a Lei de Ferro do comportamento inteligente. Novas guidelines sobre IAs e BTs devem ser inseridas aqui.
- **Idioma:** Código e documentação técnica das Tasks em **Inglês**.

---

## 2. A MENTE DO AGENTE: BLACKBOARD

O Blackboard é o mecanismo de Memória do Agente. É fundamental para compartilhar contexto entre Tasks desconexas sem forçar forte acoplamento (Strong Coupling).

### 2.1 Blackboard & BlackboardPlan (Dados e Escopo)

- **Papel:** Armazenar e compartilhar dados globais e locais da IA durante o ciclo mental (`tick`).
- **Regra (Tipagem Estrita):** Sempre que possível, defina `BlackboardPlan` para delimitar e tipar as opções disponíveis para o agente, exportando as variáveis adequadas para o editor.
- **Regra (Semântica de Variáveis):** Variáveis definidas no Blackboard devem ser tratadas como "Fatos" percebidos pelo agente.
- **Regra (Escopo Múltiplo):** Aproveitar escopos/scopes de Blackboard para compartilhar "dados de facção" ou ambiente global entre diferentes agentes sem ter que usar Singletons pesados, criando a ilusão de _Hive Mind_ (Mentes Conectadas).

---

## 3. O EXECUTOR: BTPLAYER (O ORQUESTRADOR DE BT)

O `BTPlayer` é o Node raiz que amarra o `BehaviorTree` ao mundo físico (Node da cena).

- **Papel:** É o cérebro rodando a execução temporal (Tick).
- **Regra de Atualização:**
  - O `update_mode` deve alinhar-se com o Ability System para simular controle direto. `UpdateMode::PHYSICS` garante sincronia de rede e Rollbacks adequados.
  - Para casos multplayer determinísticos ou predições severas, é preferível `MANUAL` de modo a controlar granularmente quando o "Tick" mental da IA acontece dentro do physics handler nativo.
- **Soberania do BehaviorTree:** O `BTPlayer` apenas processa as decisões estáticas de um Resource `.tres` (`BehaviorTree`).

---

## 4. ÁRVORES DE COMPORTAMENTO (BEHAVIOR TREES)

O LimboAI modela as decisões através de hierarquia de ações com status rigorosos.

### 4.1 O Ciclo de `Status`

Tasks devem retornar estritamente os estados de promessa:

- `FRESH`: Em inicialização.
- `RUNNING`: Pendente, requer continuação no próximo frame. Promove `Blocking`.
- `SUCCESS`: Ação finalizada conforme design.
- `FAILURE`: Condição não alcançada ou ação interrompida/inválida.

### 4.2 Categorias de BT Tasks (Estruturação)

1. **BTComposite (Roteadores):** `Selector`, `Sequence`, etc. **Regra:** Nunca executam lógicas de jogo reais, apenas manipulam o direcionamento da árvore.
2. **BTDecorator (Modificadores):** Inversores, Limitadores de tempo. **Regra:** Operam apenas nos retornos ou pré-condições de um _único_ nó filho.
3. **BTCondition (Observadores):** Questionam Fatos. **Regra:** São nós puros, rápidos, em "O(1)". Nunca modifique o estado do jogo aqui. Devem retornar `SUCCESS` ou `FAILURE` instantaneamente. Muitas vezes lerão do `Blackboard`.
4. **BTAction (Ação Ativa - Folha):** Onde o trabalho sujo reside. Interage com `ASComponent`, dispara animações, move personagens. **Regra:** Suas ações podem ser instantâneas ou `RUNNING`. Elas alteram o ambiente.

---

## 5. MÁQUINAS DE ESTADOS: LIMBOHSM E LIMBOSTATE

Quando a lógica exige transições orientadas a evento de forma persistente em vez de avaliações em árvore (tick-by-tick base), usamos Hierarchical State Machines (HSM).

- **LimboHSM:** É o contêiner de estados ativos e gerente de transições (Dispatching).
- **LimboState (Estados):** Pode abrigar `enter`, `exit`, `update`. **Regra:** Use primariamente para mecânicas fechadas do agente (como estar "Stunned", ou gerenciar Animações primárias contíguas).
- **Hibridismo (`BTState`):** Quando a macro-logica precisa de rigidez (HSM) mas a micro-decisão pede fluidez (BT). **Regra:** Envolva Behavior Trees dentro de Estados para a melhor flexibilidade.

---

## 6. INTEGRAÇÃO LIMBOAI + ABILITY SYSTEM

Para evitar "God Objects" em Actions e garantir o uso correto da máquina de Abilities e Tags, estabelecem-se as seguintes pontes arquiteturais:

### 6.1 Condições via Tags (BTCondition)

IAs devem reagir a contexto aplicando e lendo `Tags`, nunca perguntando a variáveis booleanas arbitrárias do personagem.

- Utilize tarefas do tipo BTObservador que acessem o `ASComponent` e certifiquem-se do status de: `has_tag(...)`.
- _Exemplo:_ Uma IA deve recuar caso ela possua `Debuff.Fear`.

### 6.2 Lançamento de Habilidades (BTAction)

O BTAction **não deve** reproduzir dano direto ou projéteis, a menos que ele seja em si o efeito do tiro. Ele deve acessar o ASC e comandar:

- `try_activate_ability_by_tag(...)`.
  A task fica em `RUNNING` enquanto a habilidade não reportar por _signal_ sua conclusão (isso promove ações que esperam pela animação via Ability System).

### 6.3 O Agente Subordinado

A IA é apenas um "Controller" remoto que simula um jogador local operando as alavancas do `ASComponent`. O LimboAI dita _quando agir_, e o Ability System dicta o _se é possível_ e os _efeitos_.

---

## 7. CRITÉRIOS DE DESENVOLVIMENTO (TASKS CUSTOMIZADAS)

Criar novas Tasks em C++ ou GDScript exige obediência de código:

1. **Uso de Godot Virtuals:** O sobrescrito deve ocorrer nas virtuals `_enter`, `_tick`, `_exit`, e `_setup`.
2. **Propriedades Exportadas:** Quando usar atributos variáveis nas tasks preze pela possibilidade de apontar variaves de Blackboard (`BlackboardPlan`) usando as macros/sufijos que atam com as variáveis nativas de editor (`_var`).
3. **Idempotência no tick:** Se a lógica em `update(delta)` retornar `RUNNING`, asseverar que ela não inicializará eventos duplicados a cada frame a menos que a intenção seja um tick de continuidade.
4. **Isolamento de Erro:** Retornar `FAILURE` no log não é um erro fatal na Godot, é controle de fluxo da árvore. Abrace o `FAILURE` como via normal de controle (Ex: Target Not Found).

---

## 8. DEPURADOR VISUAL (VISUAL DEBUGGER)

- Obrigação manter a execução da árvore em conformidade tal que a ferramenta gráfica de Debugger nativa do LimboAI seja rastreável para a detecção de _bottlenecks_ nas escolhas da IA. Crie BTTasks com `get_custom_name()` claros se a lógica requer instâncias dinâmicas pesadas.
