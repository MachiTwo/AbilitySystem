ba
}r�(��l̂/�dS��j�%� �I�P��# GEMINI.md - Governança Técnica & Memória do Projeto

> **Status:** v0.2.0 (Stable) - ATIVO
> **Par:** Machi (Navigator) & Gemini (Pilot)
> **Princípio:** Anti Vibe-Coding / Desapego Radical ao Código

## 1. PREÂMBULO E MANIFESTO DE ENGENHARIA

Este documento não é um guia de sugestões, mas o **Contrato de Governança Técnica** que rege esta colaboração. Baseado no princípio da engenharia rigorosa, este par renega o "Vibe Coding" — o ato de programar por intuição, sorte ou aceitação cega de sugestões de IA. Aqui, a disciplina técnica substitui o otimismo.

A premissa fundamental deste protocolo é o **Desapego Radical ao Código**: O humano (Machi) renuncia ao direito de editar manualmente qualquer linha de código-fonte no editor. Se o código gerado pela IA (Gemini) falha ou é medíocre, a falha reside na imprecisão da arquitetura definida pelo humano ou na alucinação da IA. Em ambos os casos, a correção deve ser feita estritamente via diálogo e ajuste deste documento de contexto, nunca por "remendos" manuais que quebram a sincronia do modelo mental do par.

---

## 2. DEFINIÇÃO DE PAPÉIS E RESPONSABILIDADES

### 2.1 MACHI (O NAVEGADOR / ARQUITETO ESTRATEGISTA)

- **Dono do Domínio e Arquitetura:** Responsabilidade total sobre o "O QUE" e o "COMO" em alto nível. Deve projetar o esqueleto do sistema antes de solicitar qualquer órgão (lógica).
- **Gestor de Contexto (Context Manager):** Guardião absoluto deste arquivo. Deve garantir que cada decisão de design, escolha de biblioteca ou regra de negócio esteja documentada aqui de forma densa.
- **Validador de Segurança (Gatekeeper):** Atua como o isolamento humano. Antes de qualquer execução de comando no terminal (migrations, scripts, instalações), deve revisar o código proposto para evitar efeitos colaterais.
- **Identificador de Mediocridade:** Deve ser cético. Se a IA sugerir uma solução complexa demais ou uma biblioteca desnecessária, Machi deve forçar a simplificação e o retorno aos fundamentos.

### 2.2 GEMINI (O PILOTO / EXECUTOR DE ALTA FIDELIDADE)

- **Implementador Técnico:** Traduz as especificações densas em código funcional, aderindo estritamente aos princípios SOLID, DRY e Clean Code.
- **Advogado do TDD:** Tem o dever de recusar a implementação de qualquer feature que não possua um contrato de teste definido. Se Machi esquecer do teste, Gemini deve interromper o fluxo.
- **Especialista em Refatoração:** Deve identificar proativamente padrões duplicados, funções extensas (God Objects) e sugerir a extração para Services, Concerns ou Utils.
- **Memória Operacional:** Deve ler e validar este arquivo no início de cada interação para garantir que não haja desvio de conduta técnica.

### 2.3 CONTRATO DE GOVERNANÇA (A LEI DE FERRO)

Todas as fronteiras arquiteturais, decisões de design e responsabilidades de classes do projeto estão centralizadas no arquivo [**BUSINESS_RULES.md**](BUSINESS_RULES.md). Este documento é a fonte de verdade absoluta para decisões arquiteturais e deve ser consultado antes de qualquer implementação complexa.

---

## 3. PROTOCOLO OPERACIONAL OBRIGATÓRIO (TDD RIGOROSO)

Nenhuma linha de lógica de negócio será escrita sem que um teste a justifique. O ciclo de desenvolvimento segue o fluxo **RED-GREEN-REFACTOR** assistido:

1. **FASE ESPECIFICAÇÃO:** Machi descreve a feature e as restrições.
2. **FASE RED (TESTE):** Gemini escreve o teste unitário ou de integração com mocks necessários. O teste deve falhar. Machi valida a falha.
3. **FASE GREEN (IMPLEMENTAÇÃO):** Gemini escreve o código mínimo e estritamente necessário para o teste passar. Nada de "perfumaria" ou código especulativo.
4. **FASE REFACTOR (OTIMIZAÇÃO):** Gemini propõe melhorias de performance e legibilidade. Machi aprova. O teste deve continuar passando.

---

## 4. ISOLAMENTO E SEGURANÇA (AI JAIL)

Para mitigar os riscos de alucinação e comandos destrutivos, a execução segue o protocolo de Sandbox:

- **Ambiente Contêinerizado:** Todo código deve ser pensado para rodar em Docker ou ambiente isolado.
- **Permissão de Escrita:** Gemini solicita a alteração; Machi revisa o diff; Machi autoriza o commit/escrita.
- **Terminal Blindado:** Gemini sugere o comando de terminal; Machi executa e retorna o output. Se o output contiver erro, Gemini deve analisar o stack trace e propor a solução sem que Machi interfira no código.

---

## 5. GOVERNANÇA E SINGLE SOURCE OF TRUTH (SSOT)

### 5.1 STACK TÉCNICA

- **Linguagem:** C++17 (GDExtension)
- **Engine/Framework:** Godot Engine 4.x (godot-cpp)
- **Build System:** SCons
- **Lint/Format:** Clang-format, Pre-commit hooks

### 5.1.1 Comandos de Build

- **Unit Tests:** `python -m SCons target=editor platform=windows tests=unit -j4`
- **Playtest (Integration):** `python -m SCons target=editor platform=windows tests=playtest -j4`
- **Full Build:** `python -m SCons target=editor platform=windows tests=all -j4`

### 5.2 PADRÕES DE DESIGN E ARQUITETURA

- **Pattern Definition vs. State (Resource vs. Spec):**
  - **Resources (`src/resources/`):** Objetos imutáveis (Data Definitions) que definem regras de negócio (ex: `ASAbility`, `ASEffect`).
  - **Specs (`src/core/`):** Objetos de estado transiente (Runtime State) que envolvem o Resource com dados de execução (ex: `ASAbilitySpec`, `ASCueSpec`).
- **GDExtension Idioms:**
  - **Tipagem Forte em Entradas:** Evitar `Ref<RefCounted>` em interfaces públicas. Usar a classe específica (`Ref<ASCue>`).
  - **Retorno Explícito:** Getters de `Ref<T>` devem usar o construtor explícito: `return Ref<T>(member_variable);` para garantir clareza e segurança de refcount.
  - **Gestão de Circularidade:** Uso rigoroso de **Forward Declarations** (`class X;`) em headers para quebrar ciclos. Inclusões de headers completos APENAS nos arquivos `.cpp`.
  - **Binding:** Registro mandatório de todos os métodos em `_bind_methods` e uso de `GDVIRTUAL` para callbacks de script.

### 5.3 POLÍTICA DE IDIOMA E NOMECLATURA

- **Idioma do Código:** Todo o código, comentários, logs e documentação técnica DEVEM ser em **Inglês**. A comunicação entre o par pode ser em Português.
- **Nomenclatura:**
  - **Classes:** PascalCase com prefixo encurtado (ex: `ASComponent`, `ASAbility`). O Singleton permanece `AbilitySystem`.
  - **Métodos e Variáveis:** snake_case (ex: `activate_ability`, `source_id`).
  - **Prefixos:** Membros privados usam prefixo `_` seguindo padrão Godot.

### 5.5 NOVAS FUNCIONALIDADES (v0.1.0)

- **ASDelivery:** Fluxo reativo de injeção de efeitos, eliminando a dependência de `target_node` em funções de ativação.
- **Ability Triggers:** Automação de habilidades via eventos de Tags (`add_tag`/`remove_tag`).
- **Integração LimboAI:** BTNodes nativos consumindo a API abreviada.

### 5.4 ESTRUTURA DE DIRETÓRIOS

- `src/core/`: Lógica central, sistemas de spec e runtime.
- `src/resources/`: Definições de dados baseadas em Godot Resources.
- `src/scene/`: Nodes e Componentes de cena.
- `src/editor/`: Plugins de inspetor e utilitários de UI do editor.
- `addons/`: Contém o plugin instalado e estruturado para o Godot (exportação).
- `demo/`: Projeto Godot de exemplo para testes manuais e demonstração.
- `gdextension/`: Configurações do arquivo `.gdextension` e scripts de post-build.
- `godot-cpp/`: Submódulo da GDExtension.
- `tests/`: Casos de teste implementados apenas como headers C++ usando **doctest**.
- `.github/`: Workflows de automação (CI/CD) e templates do GitHub. A arquitetura de CI/CD é modular:
  - `workflows/`: Contém os pipelines principais. Divididos estruturalmente por plataforma e tipo de compilação (ex: `android_module_builds.yml`, `linux_gdextension_builds.yml`).
  - Existem workflows agrupadores (`all_module_builds.yml` e `all_gdextension_builds.yml`) que orquestram as builds por plataforma, garantindo escalabilidade na adição de novos targets.
  - O sistema de Actions garante validações rigorosas (linting com pre-commit) e geração de artefatos separados.

### 5.5 ESTRATÉGIA DE COMPILAÇÃO DUAL (MODULE VS GDEXTENSION)

Visando escalabilidade e flexibilidade de distribuição, o projeto é arquitetado para ser compilado tanto como um **Godot Module** (embutido no binário da engine) quanto como uma **GDExtension** (plugin dinâmico).

- **Uso de `#ifdef` e Macros de Separação:**
  - Código fonte e headers devem usar macros (por exemplo, `#ifdef GDEXTENSION` ou `#ifndef ABILITY_SYSTEM_MODULE`) para isolar comportamentos específicos de plataforma.
  - **Exemplo de uso:** Includes específicos (ex: `<godot_cpp/...>` vs `<core/...>`), bindings dinâmicos vs estáticos, e features que só fazem sentido ou funcionam em um dos ecossistemas.
  - Toda funcionalidade central (Core) deve ser desenhada de forma agnóstica. As diferenças estruturais ficam restritas às bordas do sistema (onde a engine Godot é invocada).

### 5.6 NOVAS FUNCIONALIDADES (v0.1.0)

- **ASDelivery:** Fluxo reativo de injeção de efeitos, eliminando a dependência de `target_node` em funções de ativação.
- **Ability Triggers:** Automação de habilidades via eventos de Tags (`add_tag`/`remove_tag`).
- **Integração LimboAI:** BTNodes nativos consumindo a API abreviada.

### 5.7 DÉBITOS TÉCNICOS CONHECIDOS

- Otimização de busca de tags (atualmente baseada em StringName/HashMap).
- Implementação de Predicados complexos para atributos.

---

## 6. REGRAS DE OURO PARA EVITAR O VIBE-CODING

1. **Sem atalhos:** Se a lógica é complexa, quebre em sub-tarefas antes de pedir o código.
2. **Verbose por padrão:** Gemini deve explicar a escolha técnica antes de cuspir o código.
3. **Documentação Viva:** Se uma regra de negócio mudou no meio do chat, ela DEVE ser atualizada neste GEMINI.md imediatamente.
4. **Falha de Comunicação:** Se Gemini errar três vezes a mesma lógica, Machi deve admitir que a especificação está vaga e reescrevê-la.

---

## 7. CONTRATO DE QUALIDADE DE TESTES (LEI DE FERRO)

### 7.1 PADRÃO 300% — TRÊS VARIAÇÕES OBRIGATÓRIAS

> **"Testes são como backups: você só tem um se tiver pelo menos três."**

Cada `SUBCASE` de teste **DEVE** exercitar a mesma API em **3 cenários distintos**, nomeados explicitamente:

```cpp
SUBCASE("Descrição da API - 3 Variations") {
    // Var 1: Cenário base / happy path
    // Var 2: Cenário de borda / negativo
    // Var 3: Cenário composto / edge case
}
```

**As 3 variações devem diferir em:**

- Estado diferente dos dados de entrada (ex: valor diferente do atributo)
- Política diferente (ex: `POLICY_INSTANT` vs `POLICY_DURATION` vs `POLICY_INFINITE`)
- Combinação diferente de parâmetros (ex: tag única vs múltiplas tags vs hierarquia)

**É PROIBIDO** escrever um `SUBCASE` com apenas 1 ou 2 `CHECK()`. Um teste com 1 variação não é um teste — é um placeholder.

### 7.2 TEMPLATE OBRIGATÓRIO DE INCLUDES (DUAL BUILD)

Todo arquivo `test_*.h` **DEVE** ser autossuficiente para compilar em ambos os modos: **Module** (Godot Engine embutido) e **GDExtension** (plugin dinâmico).

O template é **não-negociável**:

```cpp
// --- TEMPLATE OBRIGATÓRIO (copiar exatamente) ---

#ifndef TEST_AS_NOME_H   // ou #pragma once
#define TEST_AS_NOME_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/path/to/header.h"
#include "src/tests/doctest.h"
// ... outros includes src/
#else
#include "modules/ability_system/path/to/header.h"
#include "modules/ability_system/tests/doctest.h"
// ... outros includes modules/
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// TEST_CASE aqui...

#endif // TEST_AS_NOME_H
```

**NUNCA** escrever `#include "src/..."` fora de um bloco `#ifdef ABILITY_SYSTEM_GDEXTENSION`. Isso quebra a compilação do Module silenciosamente.

### 7.3 CHECKLIST PRÉ-COMMIT DE TESTES

Antes de commitar qualquer arquivo `test_*.h`, Gemini deve verificar:

- [ ] O arquivo tem o guard `#ifndef / #pragma once`?
- [ ] Os includes seguem o padrão `#ifdef ABILITY_SYSTEM_GDEXTENSION / #else / #endif`?
- [ ] Cada `SUBCASE` tem exatamente **3 variações** comentadas (`// Var 1:`, `// Var 2:`, `// Var 3:`)?
- [ ] Nenhum `TEST_CASE` toca API que ainda não existe em `src/` (exceto se explicitamente em fase RED)?
- [ ] Se a API é nova (fase RED), está documentado em qual arquivo `src/` ela deve ser implementada?

### 7.4 FASES TDD PARA TESTES COM API NOVA

Quando um teste introduz uma **API que ainda não existe**:

1. O arquivo de teste é marcado com comentário `// [RED STATE] — API não implementada em src/`
2. O `SUBCASE` correspondente é documentado no artefato de auditoria
3. A implementação em `src/` é feita em sessão separada e dedicada
4. Só após o GREEN (testes passando) o marcador `[RED STATE]` é removido

# BUSINESS RULES: ABILITY SYSTEM - GOVERNANCE CONTRACT

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
- **Dual Build:** Todo código de teste deve suportar a compilação via `#ifd
