# Documentação de Testes - Ability System Plugin

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

- **`AbilitySystemTagSpec`**: Correspondência hierárquica, exata e registro.
- **`AbilitySystemAttributeSet`**: Clamping de valores base e inicialização.
- **`AbilitySystemEffectSpec`**: Cálculos matemáticos para modificações de atributos.

### 2. Testes de Integração

Simulam cenários reais da Godot usando o `AbilitySystemComponent`:

- **Aplicação de Efeitos**: Testa duração, lógica de empilhamento (Override, Intensidade) e remoção.
- **Execução de Habilidades**: Valida requisitos de ativação, custos e cooldowns.
- **Integridade de Sinais**: Garante que os componentes emitam eventos corretamente quando tags ou atributos mudam.

### 3. Testes de Projeto (Cenário RPG Top-Down)

Localizado em `src/tests/test_ability_system_integration.h`, esta suíte simula um ecossistema completo de um RPG Top-Down para verificar interações de alto nível e "regras de negócio" de todo o sistema.

**Visão Geral do Cenário:**
- **Atores**: Jogador e Inimigo/NPC, ambos equipados com um `AbilitySystemComponent`.
- **Atributos**: `Health` (Vida), `Mana`, `Stamina` (Resistência).
- **Estados (Tags)**: `State.Talking` (Conversando), `State.Stunned` (Atordoado), `State.Dead` (Morto), `State.Burning` (Queimando).

**Fluxos de Interação Verificados:**

- **Diálogo vs. Combate**:
  - Ativar `Ability.Talk` concede a tag `State.Talking`.
  - Durante a conversa, habilidades de combate (ex: `Ability.Melee`) são bloqueadas via `activation_blocked_tags`, garantindo que o jogador não ataque durante um diálogo.
- **Combate & DOT (Dano ao Longo do Tempo)**:
  - `Ability.Fireball` verifica o custo de `Mana` e aplica o `Effect.Burning`.
  - `Effect.Burning` é um efeito periódico que reduz `Health` a cada segundo, verificando a capacidade do sistema de lidar com decaimento de atributos baseado em tempo.
- **Controle de Grupo (CC)**:
  - A aplicação de `State.Stunned` cancela habilidades atuais e bloqueia novas ativações.
- **Progressão e Nível**:
  - Habilidades verificam requisitos de `Attribute` (ex: precisar de 150 de Mana para lançar um feitiço de alto nível) sem necessariamente consumir o atributo.
- **Morte e Ciclo de Vida**:
  - Quando `Health` chega a zero, `State.Dead` é aplicado.
  - Verificado que todas as habilidades/efeitos ativos são limpos e ações futuras são restringidas.

---

## 🚀 Execução

### Local (GDExtension)

Para rodar a suíte completa usando seu binário local da Godot:

```powershell
python -m SCons tests=yes
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
