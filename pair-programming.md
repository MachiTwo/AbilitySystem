# PAIR PROGRAMMING: MACHI & GEMINI (PROTOCOLO ANTI VIBE-CODING)

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
