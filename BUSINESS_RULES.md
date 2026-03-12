# BUSINESS RULES: ABILITY SYSTEM PLUGIN - GOVERNANCE CONTRACT

Este documento estabelece as fronteiras arquiteturais e regras de negócio obrigatórias. Qualquer implementação que viole estes limites deve ser refatorada imediatamente.

---

## 1. IDENTIFICADORES: AS TAGS (DNA DO SISTEMA)

Tags não são classes; são **Identificadores Hierárquicos Superpoderosos** baseados em `StringName`.

### 1.1 Tag (Identificador)

- **Papel:** Representar estados, ações ou categorias (ex: `State.Dead`, `Ability.Fireball`).
- **Regra:** Devem ser tratadas como imutáveis e globais. A comparação deve suportar hierarquia (checar `State` encontra `State.Dead`).

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

---

## 7. RESUMO DE FRONTEIRAS (MATRIZ DE RESPONSABILIDADE)

| Funcionalidade | Vive no Resource? | Vive no Spec? | Vive no Component? |
| :--- | :---: | :---: | :---: |
| **Custo Base** | Sim (Definição) | Não | Não |
| **Custo Calculado** | Não | Sim (Execução) | Não |
| **Timer/Duração** | Não | Sim (Estado) | Não (Apenas Tick) |
| **Valor do Atributo** | Não | Não | Sim (AttributeSet) |
| **Lista de Tags** | Sim (Requisitos) | Não | Sim (Estado Atual) |
| **Lógica de Stacking** | Sim (Política) | Sim (Aplicação) | Não |

---
