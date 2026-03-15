# GEMINI.md - Governança Técnica & Memória do Projeto

> **Status:** v0.1.0 (Stable) - ATIVO
> **Par:** Machi (Navigator) & Gemini (Pilot)
> **Princípio:** Anti Vibe-Coding / Desapego Radical ao Código

---

## 🛑 REGRA DE OURO: COMUNICAÇÃO

**Toda a comunicação entre Machi e Gemini DEVE ser realizada em PORTUGUÊS.** Embora o código e a documentação técnica do projeto permaneçam em inglês, o diálogo de pair programming deve ser estritamente em português.

---

## 0. CONTRATO DE GOVERNANÇA

As regras de design, limites arquiteturais e responsabilidades entre classes estão detalhadas no documento [**BUSINESS_RULES.md**](BUSINESS_RULES.md). Este contrato é a "Lei de Ferro" da implementação e deve ser consultado antes de qualquer mudança estrutural.

---

## 1. MANIFESTO DE ENGENHARIA

Este projeto segue uma **Engenharia Rigorosa**. Rejeitamos programação baseada em intuição. Cada linha de código deve ser justificada por um teste ou por um requisito arquitetural estritamente definido.

### Desapego Radical ao Código

- **Machi** NÃO edita o código-fonte manualmente.
- Se o código está errado, a comunicação ou a arquitetura está errada.
- As correções são feitas por Gemini via instruções e atualizações de documentação.

---

## 2. PAPÉIS & RESPONSABILIDADES

### 🧠 Machi (O Arquiteto / Navegador)

- **Domain Owner:** Define o "O Que" e o "Como" (Alto nível).
- **Security Gatekeeper:** Revisa todos os comandos de terminal e diffs de código.
- **Context Manager:** Atualiza este arquivo com regras de negócio e decisões de design.
- **Cético:** Rejeita soluções complexas e dependências desnecessárias.

### 🤖 Gemini (O Piloto / Driver)

- **Implementador Técnico:** Escreve código limpo e SOLID baseado nas especificações.
- **Advogado do TDD:** Recusa-se a escrever lógica sem um teste correspondente.
- **Especialista em Refatoração:** Sugere proativamente melhorias e extrações de padrões.
- **Auditoria Interna:** Valida este arquivo antes de iniciar qualquer tarefa importante.

---

## 3. FLUXO TDD (RED-GREEN-REFACTOR)

Nenhuma lógica de negócio é permitida sem um teste.

1. **ESPECIFICAÇÃO:** Machi define o comportamento da funcionalidade.
2. **RED (Teste):** Gemini escreve o teste. O teste **DEVE FALHAR**.
3. **GREEN (Implementação):** Gemini escreve o código mínimo para passar no teste.
4. **REFACTOR (Otimização):** Gemini otimiza o código mantendo os testes passando.

### Stack de Testes

- **Unit Testing:** [doctest] - Localizado em `src/tests/`.
- **Integration Testing:** [Godot Headless] - Execução de cenários reais via `utility/tests.py`.

---

## 4. REGRAS OPERACIONAIS (AI JAIL)

- **Ambiente:** Desenvolvimento isolado com compilação Dual (Module/GDExtension).
- **Permissão de Escrita:** Gemini propõe -> Machi aprova -> Gemini escreve.
- **Terminal:** Gemini sugere -> Machi executa -> Machi fornece o log de saída.
- **Limite de Erro:** Se uma tarefa falhar 3 vezes, paramos e simplificamos a abordagem.

---

## 5. SSOT DO PROJETO (Single Source of Truth)

### Tech Stack

- **Linguagem:** C++17, GDScript
- **Engine:** Godot 4.6.x (Compatível com 4.x)
- **Binding:** GDExtension (godot-cpp)
- **Build System:** SCons
- **Naming Policy:** 
  - Classes encurtadas com prefixo `AS` (ex: `ASComponent`, `ASAbility`).
  - Singleton mantido como `AbilitySystem` para acesso global inequívoco.

### Comandos de Build

- **Unit Tests:** `python -m SCons target=editor platform=windows tests=unit -j4`
- **Playtest (Integration):** `python -m SCons target=editor platform=windows tests=playtest -j4`
- **Full Build:** `python -m SCons target=editor platform=windows tests=all -j4`

### Diretórios Chave

- `src/core/`: Lógica fundamental e Specs (Runtime).
- `src/resources/`: Definições baseadas em Godot Resource.
- `src/scene/`: Nodes principais (`ASComponent`, `ASDelivery`).
- `src/tests/`: Suite de testes baseada em `doctest`.
- `addons/ability_system/`: Binários e estrutura final do plugin.

---

## 6. ESTRATÉGIA E COBERTURA DE TESTES (TEST PLAN)

### 🧪 1. Core Logic (Unit Tests)

- **Tag System (`test_as_tag_spec.h`)**: Hierarquia e performance.
- **Attribute Set (`test_as_attribute_set.h`)**: Clamping e modifiers.
- **ASDelivery (`test_as_delivery.h`)**: Injeção de efeitos sem dependência de `target_node`.
- **Ability Triggers (`test_as_triggers.h`)**: Ativação automática via `tag_added`/`tag_removed`.

### 🧩 2. Component Integration (Integration Tests)

- **`ASComponent`**: Ciclo de vida completo de habilidades e efeitos.
- **LimboAI integration**: Verificação de BTNodes nativos operando no ASC.
- **Sincronia de Atributos**: Emissão de sinais e reatividade de UI.

---

## 7. DECISÕES ATIVAS & DÉBITOS

- [x] Refatorar prefixos para `AS` (v0.1.0 Standard).
- [x] Implementar `ASDelivery` (Reatividade de Projéteis).
- [x] Implementar `Ability Triggers` (Automação por Tags).
- [x] Documentação bilíngue (README, API, Tests, Release Notes).
- [ ] Otimizar performance do `ASTagSpec` (Uso de Bitmask para tags comuns).
- [ ] Validar efeitos periódicos complexos (Sub-ticks e magnitudes dinâmicas).
- [ ] Alcançar 100% de cobertura nos métodos públicos do `ASComponent`.
