# Documentação de Testes - Ability System

## 🧪 Metodologia: Test-Driven Development (TDD)

Este projeto segue uma abordagem de **Rigor de Engenharia**. Nenhuma lógica de negócio é implementada sem um caso de teste correspondente que justifique sua existência.

### Ciclo Red-Green-Refactor

1. **RED (Vermelho)**: Defina o requisito através de um teste. O build deve falhar ou o teste não deve passar.
2. **GREEN (Verde)**: Implemente o código mínimo necessário para satisfazer o teste. Evite sobre-engenharia.
3. **REFACTOR (Refatorar)**: Otimize o código para performance e legibilidade, garantindo que todos os testes continuem passando.

---

## 🏗️ Arquitetura de Testes

O sistema de testes é projetado para funcionar tanto em **GDExtension** (via `doctest`) quanto em **Módulo Nativo** (via o test runner interno da Godot).

### 1. Lógica Core (Testes Unitários)

Localizados em `src/tests/`, verificam comportamentos atômicos do sistema:

- **`ASTagSpec`**: Correspondência hierárquica, exata e registro.
- **`ASAttributeSet`**: Clamping de valores base e inicialização.
- **`ASEffectSpec`**: Cálculos matemáticos para modificações de atributos.
- **`ASDelivery`**: Sistema de entrega reativa de efeitos para alvos (ideal para projéteis).
- **`Ability Triggers`**: Ativação automática de habilidades baseada em eventos de Tags (Adição/Remoção).

### 2. Testes de Integração

Simulam cenários reais da Godot usando o `ASComponent`:

- **Aplicação de Efeitos**: Testa duração, lógica de empilhamento (Override, Intensidade) e remoção.
- **Execução de Habilidades**: Valida requisitos de ativação, custos e cooldowns.
- **Integridade de Sinais**: Garante que os componentes emitam eventos corretamente quando tags ou atributos mudam.

### 3. Testes de Projeto (Cenário RPG Top-Down & LimboAI)

Localizado em `src/tests/test_as_integration.h`, esta suíte simula um ecossistema completo de um RPG Top-Down para verificar interações de alto nível e "regras de negócio" de todo o sistema.

**Visão Geral do Cenário:**

- **Atores**: Jogador e Inimigo/NPC (Charger), ambos equipados com um `ASComponent`.
- **IA**: O Inimigo utiliza LimboAI integrado via `BTAction` para disparar habilidades no componente.
- **Fluxos Verificados**:
  - **Diálogo vs. Combate**: Bloqueio de habilidades sociais por estados de combate.
  - **Controle de Grupo (CC)**: Stun cancelando habilidades ativas.
  - **Morte e Ciclo de Vida**: Transição para `State.Dead` limpando todos os recursos.

---

## 🚀 Execução

### Local (GDExtension)

Para rodar a suíte completa usando seu binário local da Godot:

```powershell
python -m SCons target=editor platform=windows tests=playtest -j4
```

Isso compilará o plugin com símbolos de teste e invocará o `utility/tests.py`, que lança a Godot de forma headless para rodar a suíte `doctest`.

### CI/CD (GitHub Actions)

Testes são executados automaticamente em cada Push ou Pull Request para:

- **Linux (x64 & ARM64)**
- **Windows (x64)**
- **macOS (Universal)**

---

## 📊 Metas de Cobertura

Nosso objetivo é **100% de Cobertura da Lógica Core**. Código de UI específico do Editor é excluído dos testes unitários automatizados e depende da validação empírica via o projeto `demo/`.
