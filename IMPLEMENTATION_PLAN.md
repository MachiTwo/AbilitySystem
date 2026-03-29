# Plano de Implementação: Correções C++ Framework e Limpeza Demo

**Status**: Pronto para execução
**Data**: 2026-04-04

---

## FASE 1: Correções C++ Framework (✅ CONCLUÍDO)

### 1.1 Incluir Header Time para Build Module

**Problema**: `'Time' has not been declared` em 14 localizações de as_component.cpp

**Solução Implementada**:

- ✅ Adicionado `#include "core/os/time.h"` ao bloco else (module build) em as_component.cpp
- ✅ Envolvido todos os `Time::get_singleton()` com ifdef guards:
  - `#ifdef ABILITY_SYSTEM_GDEXTENSION` → usar `Time::get_singleton()`
  - `#else` → usar `OS::get_singleton()`
- ✅ Corrigido em 8 funções:
  1. `set_attribute_base_value_by_tag()` - linha 481
  2. `_record_ability_event()` - linha 2561
  3. `_record_effect_event()` - linha 2579
  4. `_record_cue_event()` - linha 2597
  5. `_record_name_tag_event()` - linha 2613
  6. `_record_conditional_tag_event()` - linha 2630
  7. `_record_event_tag_event()` - linha 2648
  8. `get_attribute_history()`, `get_ability_history()`, `get_effect_history()`, `get_cue_history()`

**Verificação**: Todos os 14 `Time::get_singleton()` foram envolvidos com ifdef ou já estavam protegidos.

**Status**: ✅ CONCLUÍDO

---

## FASE 2: Limpeza da Demo Layer

### 2.1 Análise e Remoção da Pasta Gameplay

**Descobertas**:

- 📁 Pasta: `demo/gameplay/` (26 arquivos, 13 scripts .gd)
- 📂 Estrutura:
  - `core/` - BehaviorMachine.gd, BehaviorStates.gd (estado do plugin legado)
  - `machines/` - Máquinas de estado específicas do jogo (Pause, Menu, etc.)
  - `resources/` - Configurações de máquinas de estado (resources .tres)

**Análise de Dependências**:

- `demo/autoload/GameMachine.gd` extends `Machine` (classe do gameplay)
- `demo/.godot/global_script_class_cache.cfg` referencia `BehaviorStates`
- Sistema é completamente separado do Ability System (não há conflito direto)

**Ação**:

1. Deletar pasta `demo/gameplay/` completamente
2. Atualizar `demo/autoload/GameMachine.gd` para não estender classe legada
3. Remover cache de classes globais do Godot

### 2.2 Refatoração de Markdowns da Demo

**Arquivos Identificados**:

```
demo/
├── BehaviorStateEngineering.md        ← Documentação do sistema legado
├── GEMINI.md                          ← Desconhecido, requer análise
├── HashMap.md                         ← Possível documentação interna
├── README.md                          ← README principal da demo
├── StateIndex.md                      ← Index de estados (legado?)
└── addons/ability_system/
    ├── README.md                      ← README English
    ├── README.pt.md                   ← README Português
    ├── release_note.md                ← Release notes English
    └── release_note.pt.md             ← Release notes Português
```

**Estratégia de Refatoração**:

#### 2.2.1 Remover/Reorganizar Markdowns Legados

- ❌ Deletar: `BehaviorStateEngineering.md` (sistema descontinuado)
- ❓ Revisar: `GEMINI.md`, `HashMap.md`, `StateIndex.md` (determinar relevância)

#### 2.2.2 Criar Novo README para Ability System Demo

**Arquivo**: `demo/ABILITY_SYSTEM_DEMO.md`

Conteúdo esperado:

```markdown
# Ability System - Demo Scene

## Overview

A cena de demo demonstra como usar o Ability System para:

- Movimentação do player (Idle, Walk, Run, Jump, Fall, Dash)
- Sistema de habilidades (Combo, Heavy Attack, Dash Attack)
- Rastreamento de estado via tags (NAME, CONDITIONAL, EVENT)
- UI responsiva com sinais tag_changed

## Arquitetura

### Player Component (demo/player/)

- `player.tscn` - Cena principal com CharacterBody2D
- `player.gd` - Lógica do player
- `player_animations.tres` - Biblioteca de animações
- `resources/hotbar.tres` - Sistema de inventário

### UI Component (demo/ui/)

- `hud.tscn` - HUD principal
- `hud.gd` - Rastreamento de estado e atributos
  - Conectado a `ASComponent.tag_changed` signal
  - Exibição de health/stamina bars
  - Estado atual colorizado
  - Histórico de ativações

### Resources (demo/resources/)

- `abilities/` - Definições de habilidades (ability\_\*.tres)
- `attributes/` - Definições de atributos (attr\_\*.tres)
- `effects/` - Definições de efeitos (effect\_\*.tres)
- `packages/` - Pacotes de entrega (package\_\*.tres)

## Key Concepts

### Tag System

- **NAME**: Tags persistentes (weapon.unarmed, weapon.sword)
- **CONDITIONAL**: Tags condicionais que podem ser removidas
- **EVENT**: Tags de eventos únicos

### Ability Flow

1. Player input → ASComponent.try_activate_ability_by_tag()
2. Habilidade executa fases
3. Efeitos são disparados (ASDelivery, ASPackage)
4. Sinais são emitidos (ability_activated, tag_changed)

### UI Binding

- HUD se conecta via `asc.tag_changed.connect(_on_asc_tag_changed)`
- Atualiza estado de forma reativa (não polling)
- Formata tags para exibição legível

## Running the Demo

1. Abra `demo/demo.tscn`
2. Pressione Play
3. WASD para mover, Space para pular, Mouse Click para atacar

## Debugging

Console prints adicionados em:

- `demo/ui/hud.gd`: "[HUD] tag_changed", "[HUD] State updated"
- `demo/player/player.gd`: "[State] Activating motion.\*", "[Tag Color]"
```

#### 2.2.3 Atualizar README Principal

**Arquivo**: `demo/README.md`

Conteúdo:

```markdown
# Demo - Ability System v4.6

Esta pasta contém uma cena de demonstração funcional do Ability System.

## Quick Start

- Cena principal: `demo.tscn`
- Cena do player: `player/player.tscn`
- HUD: `ui/hud.tscn`

## Documentação Completa

- [Ability System Demo Guide](ABILITY_SYSTEM_DEMO.md)
- [Framework Documentation](../src/doc_classes/)
- [Business Rules](../BUSINESS_RULES.pt.md)

## Estrutura

- `player/` - Player controller e assets
- `ui/` - Interface de usuário
- `resources/` - Dados de configuração (habilidades, atributos, efeitos)
- `autoload/` - Autoloads da demo
```

### 2.3 Remover Dependências de Camera e Animation Legadas

**Verificar**:

```bash
# Procurar por scripts de câmera legados
find demo -name "*camera*" -o -name "*animation*" | grep -v ".godot"

# Procurar por nodes que referenciam sistemas antigos
grep -r "extends Machine" demo --include="*.gd"
```

**Ação**: Remover qualquer script que estenda `Machine` ou implemente lógica de estado legada.

---

## FASE 3: Resolução de Erros de Teste GDExtension

### 3.1 Protected Method Access Errors

**Problema**: Unit tests acessam métodos/construtores protegidos de ASComponent

**Contexto**: Friend classes foram adicionadas a `as_component.h`, mas testes ainda falham

**Ação Necessária**:

1. Revisar erros específicos do build GDExtension
2. Adicionar friend declarations adicionais se necessário:
   ```cpp
   // em as_component.h
   friend class TestASComponent;
   friend class TestASComponentRollback;
   ```
3. Ou refatorar testes para não acessar métodos protegidos diretamente

---

## FASE 4: Validação de Estado do Player

### 4.1 Verificação de Colorização

**Status Atual**: Player não coloriza baseado em estado (bug não resolvido)

**Próximas Ações**:

1. Verificar se `tag_changed` signal está disparando
2. Verificar se `_on_tag_changed()` está sendo chamado
3. Adicionar logs adicionais em `demo/player/player.gd`

---

## Ordem de Execução

### Imediato (Build Blocker):

1. ✅ **CONCLUÍDO**: Compilar C++ com Time includes
2. **EM PROGRESSO**: Executar GHA build para confirmar sucesso
3. ⏳ **PENDENTE**: Resolver testes GDExtension (se houver erros)

### Curto Prazo (Demo Cleanup):

4. Deletar `demo/gameplay/`
5. Refatorar `demo/autoload/GameMachine.gd`
6. Refatorar markdowns da demo
7. Executar demo e validar funcionamento

### Médio Prazo (Polish):

8. Debugar player colorization
9. Documentar padrões de uso do Ability System
10. Criar exemplos adicionais

---

## Checklist de Implementação

### C++ Framework (✅ CONCLUÍDO)

- [x] Adicionar include `core/os/time.h`
- [x] Envolver Time::get_singleton() com ifdef
- [x] Compilação local validada

### Demo Layer - Fase 1

- [ ] Deletar `demo/gameplay/`
- [ ] Remover cache de classes globais Godot
- [ ] Atualizar `GameMachine.gd` ou remover se necessário

### Demo Layer - Fase 2

- [ ] Criar `demo/ABILITY_SYSTEM_DEMO.md`
- [ ] Atualizar `demo/README.md`
- [ ] Remover/reorganizar markdowns legados

### Validação

- [ ] GHA build passou (module + gdextension)
- [ ] Demo executa sem erros
- [ ] Player coloriza baseado em estado
- [ ] HUD exibe estado corretamente

---

## Arquivos Afetados

### C++

- `src/scene/as_component.cpp` ✅

### GDScript (Demo)

- `demo/autoload/GameMachine.gd` ⏳
- `demo/ui/hud.gd` ✅ (já refatorado na sessão anterior)
- `demo/player/player.gd` ✅ (já refatorado)

### Markdown

- `demo/README.md` ⏳
- `demo/ABILITY_SYSTEM_DEMO.md` ⏳ (criar novo)
- `demo/BehaviorStateEngineering.md` ❌ (deletar)

### Folders

- `demo/gameplay/` ❌ (deletar completamente)

---

## Notas Importantes

1. **GameMachine**: Pode ser mantido se refatorado para NÃO estender Machine (usar simple Node ou Control)
2. **Tag System**: Funciona corretamente - HUD foi corrigido em sessão anterior
3. **C++ Compilation**: Agora deve passar em module e gdextension targets
4. **Tests**: Podem precisar de refatoração para acessar public API em vez de protegida

---

**Próximo Passo**: Iniciar Fase 2 - Deletar demo/gameplay/ e refatorar markdowns
