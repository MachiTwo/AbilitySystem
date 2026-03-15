# 🧪 Referência de Testes (v0.1.0 Estável)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](Tests.md) | [**Português**](Tests.pt.md)

---

## 🏗️ Metodologia: Rigor de Engenharia (TDD)

Este projeto rejeita o "Vibe-Coding". Cada regra de negócio é respaldada por uma suíte de **doctest**. Seguimos o ciclo **Red-Green-Refactor**:

1. **RED**: Escreva um teste que falha definindo o requisito.
2. **GREEN**: Implemente o código mínimo para passar.
3. **REFACTOR**: Otimize mantendo o status de aprovação.

---

## 🧩 Visão Geral das Suítes de Teste

Nossos testes são divididos em headers especializados em `src/tests/` para garantir modularidade e alta cobertura.

### 1. Testes Atômicos do Core (Unitários)

Verificam a lógica individual das classes sem efeitos colaterais.

- **`ASTagSpec`**: Contagem de referências, correspondência hierárquica (`State.Dead.Bleeding` corresponde a `State.Dead`) e correspondência exata.
- **`ASAttributeSet`**: Inicialização, trava de valores (clamping - ex: Vida não exceder VidaMáxima) e atualizações manuais de valores base.
- **`ASEffect` / `ASEffectSpec`**: Operações matemáticas (`ADD`, `MULTIPLY`, `OVERRIDE`) e políticas de empilhamento complexas (`INTENSITY`, `DURATION`).
- **`ASAbility` / `ASAbilitySpec`**: Verificações individuais de ativação, satisfação de custos/requisitos e transições de estado.

### 2. Testes de Sistemas Avançados

- **`ASDelivery`**: Injeção de payload em alvos. Verifica cálculos de atributos relativos à fonte (ex: dano baseado na Força do Atacante).
- **`ASPackage`**: Valida o empacotamento de múltiplos Efeitos e Cues em um único recurso portátil.
- **`Ability Triggers`**: Lógica de ativação automática quando tags são adicionadas ou removidas de um componente.
- **`AS Cues`**: Execução de feedback visual (Animação) e sonoro (AudioStream) em pontos específicos do ciclo de vida.

### 3. Integração & Cenários

Simulam a complexidade real de gameplay dentro do `ASComponent`.

- **`Lógica Avançada`**: Ticks periódicos para **Dano por Tempo (DoT)** e **Cura por Tempo (HoT)**, incluindo políticas de execução no primeiro tick.
- **`Integração RPG Flow`**: Cenários de alta densidade incluindo:
  - **Diálogo vs. Combate**: Bloqueio de habilidades baseado em estados sociais.
  - **Exaustão de Recursos**: Falha em ações quando Mana/Stamina é insuficiente.
  - **Ações Paralelas**: Execução de múltiplas habilidades (ex: Andar + Pular) simultaneamente.
  - **Resolução de Morte**: Bloqueio total de ações e limpeza de recursos após a morte do personagem.
- **`Mega Integração`**: Um teste unitário "End-to-End" completo cobrindo Habilidade -> Efeito -> Cue em uma única sequência.

---

## 🚀 Guia de Execução

### Testes Locais

A suíte roda via modo headless da Godot. Execute via SCons:

```powershell
# Rodar todos os testes unitários
python -m SCons target=editor tests=unit -j4

# Rodar cenários de integração
python -m SCons target=editor tests=playtest -j4

# Rodar tudo
python -m SCons target=editor tests=all -j4
```

### Ambiente CI/CD

O GitHub Actions executa a suíte completa em cada PR para Windows, Linux e macOS. Um código de saída diferente de zero em qualquer teste bloqueará o merge.

---

## 🛠️ Ferramentas de Teste

- **`test_helpers.h`**: Macros como `CHECK_ATTR_EQ` e `make_standard_asc` para reduzir boilerplate.
- **`test_signal_watcher.h`**: Utilitário para verificar se os sinais da GDExtension estão sendo emitidos corretamente.

> [!IMPORTANT]
> Para manter o princípio de **Desapego Radical ao Código**, mudanças de lógica DEVEM ser precedidas por um PR que atualize estes alvos de teste.
