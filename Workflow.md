# 📜 WORKFLOW.md: CONTRATO DE GOVERNANÇA E PROTOCOLO DE ENGENHARIA

> **Status:** v0.2.0 (Stable) - ATIVO  
> **Atores:** Machi (Navigator) & Gemini (Pilot)  
> **Domínios:** Zyris Engine (C++), Ability System (GDExtension), Morpheus (Next.js/TS/FastAPI).  
> **Princípio:** Anti Vibe-Coding / Desapego Radical ao Código.

---

## 1. PREÂMBULO E MANIFESTO

Este documento é o **Contrato de Governança Técnica**. Rejeitamos o "Vibe-Coding" (programação por intuição ou sorte). A disciplina técnica e o rigor arquitetural substituem o otimismo.

### 1.1 Desapego Radical ao Código

O Navegador (Machi) renuncia ao direito de editar manualmente o código-fonte. Se o código falha, a falha é na comunicação ou na arquitetura. A correção é feita estritamente via diálogo e ajuste das **Business Rules**, nunca por remendos manuais que quebram a sincronia do modelo mental do par.

---

## 2. OPERAÇÃO DE TERMINAL E ARQUIVOS (AI JAIL)

Para maximizar a eficiência de contexto e tokens, o Pilot opera sob restrições de Sandbox:

- **2.1. Pesquisa Nativa:** Proibido o uso de `ls`, `find` ou `grep` puro no terminal. O Pilot deve utilizar as ferramentas internas de análise (`grep_search`, `glob`, `list_directory`) para identificar pontos de interesse.
- **2.2. Manipulação via Git:** Renomear, mover ou apagar arquivos deve ser feito estritamente via comandos Git (`git mv`, `git rm`) para preservação de histórico.
- **2.3. Respeito ao .gitignore:** Arquivos em pastas como `godot-cpp` e `LimboAI` podem ser lidos para referência, mas **jamais** modificados.
- **2.4. Automação via Scripts:** Lógica complexa de terminal ou automação de build deve ser escrita em arquivos `.py` e executada. Comandos diretos no shell são permitidos apenas para gestão de dependências (`pip`, `npm`, `bun`).
- **2.5. Higiene de Licenças:** Remover cabeçalhos de licença de terceiros (ex: Godot/LimboAI) em qualquer arquivo novo ou refatorado para manter a soberania da Zyris Engine.

---

## 3. GESTÃO DE ESTADO E VERSIONAMENTO

- **3.1. Sincronização Pré-Tarefa:** Antes de qualquer alteração, executar `git add .` e `git status`. Se houver arquivos modificados, revisar e fazer `git commit` antes de iniciar a nova tarefa.
- **3.2. Restauração Fiel:** Para desfazer alterações, usar sempre `git checkout [arquivo]`. Jamais reescrever o conteúdo de memória.
- **3.3. Protocolo de Commit:**
  - Mensagens claras focadas no "porquê" e não no "o quê".
  - Validar hooks antes do commit: `pre-commit run --all-files` (ou `--files` para diff).
- **3.4. Entrega Integral:** O Pilot envia sempre o código **completo** ou utiliza ferramentas de substituição cirúrgica (`replace`). Sem snippets parciais que exijam colagem manual.

---

## 4. PROTOCOLO DE DESENVOLVIMENTO (TDD 300%)

Nenhuma lógica de negócio existe sem um teste que a justifique. Seguimos o ciclo **RED-GREEN-REFACTOR**.

### 4.1 O Padrão 300% (Lei de Ferro)

Cada funcionalidade deve ser provada por pelo menos **3 variações** no mesmo teste:

1.  **Happy Path:** Cenário base ideal.
2.  **Negative:** Entrada inválida ou falha esperada.
3.  **Edge Case:** Combinações complexas (multi-tags, limites de borda, latência).

### 4.2 Template de Teste (Dual Build)

Todo teste C++ deve ser autossuficiente para compilar como Module ou GDExtension:

```cpp
#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/path/to/header.h"
#else
#include "modules/ability_system/path/to/header.h"
#endif
```

---

## 5. ARQUITETURA E STACK TECNOLÓGICA

- **5.1. Core & Engine (C++):** GDExtension e Módulos nativos. Foco em alta performance e determinismo (Physics Ticking).
- **5.2. Web & App (TS/Bun):** Stack baseada em Next.js, Tailwind e FastAPI.
- **5.3. Desacoplamento:** Separação obrigatória entre **Data** (Resources), **Logic** (C++/TS) e **State** (Specs/Runtime).
- **5.4. Compilação Dual:** Toda funcionalidade central deve ser agnóstica à forma de compilação (Module vs GDExtension), isolando divergências na pasta `src/compat/`.

---

## 6. NOMENCLATURA E GOVERNANÇA DE API

A API deve ser autoexplicativa e seguir a hierarquia de intenção:

- **6.1. Gameplay (Tentativa/Estado):**
  - `try_activate_...`: Execução segura com verificação de requisitos.
  - `can_activate_...`: Pré-autorização.
  - `is_...` / `has_...`: Consultas de estado/posse.
- **6.2. Infraestrutura (Mutação Direta):**
  - `apply_...`: Injeção forçada de payloads (ex: `ASDelivery`).
  - `add_...` / `remove_...`: Modificação de baixo nível de coleções.
- **6.3. Sinais (Signals):** Sempre no passado/voz passiva (`ability_activated`, `effect_expired`).
- **6.4. Idioma:** Código e Documentação Técnica em **Inglês**. Diálogo e Estratégia em **Português**.

---

## 7. DOCUMENTAÇÃO E SSOT

Este arquivo e o [**BUSINESS_RULES.md**](BUSINESS_RULES.md) são a **Lei de Ferro**.

- Decisões estruturais geram um **ADR** (Architecture Decision Record) individual.
- Mudanças de regras no meio da sessão devem ser refletidas nestes documentos imediatamente.
- Se o Pilot falhar três vezes na mesma lógica, o Navigator deve admitir que a especificação está vaga e reescrevê-la.
