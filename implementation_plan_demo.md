# Plano de Implementação - Demo e Testes

Este plano descreve a reconstrução do projeto de demonstração e testes do zero, integrando elementos de **MMORPG** (Raças, Classes, Quests) e uma arquitetura unificada de cena que suporta Singleplayer, Multiplayer (LAN/Dedicado) e salvamento persistente por mundo.

## Elementos de Design RPG

- **Classes:** Guerreiro (Foco em Força), Mago (Foco em Mana), Assassino (Foco em Agilidade).
- **Raças:** Humano (Equilibrado), Elfo (Afinidade mágica), Orc (Poder físico).
- **Inventário:** Cada classe nasce com armas, consumíveis e moedas iniciais.
- **Missões:** Sistema de missões (Tutorial) para guiar o início do jogador.

## Estratégia de Integração C++ Nativa

O job de teste `unit-integration` rodará em uma configuração de CI que compila o Ability System e o LimboAI lado a lado. Isso valida:

- **BTNodes Nativos:** Corretude da lógica de ativação de habilidades via Behavior Tree em C++.
- **Engenharia de Bindings:** Garantir que não haja conflitos de registro ou símbolos entre as duas GDExtensions.

## Revisão do Usuário Necessária

> [!IMPORTANT]
> **Modelo de Mundo Unificado:** O Singleplayer será tecnicamente um servidor local com um único cliente conectado. Isso garante 100% de paridade de lógica entre modos.

> [!CAUTION]
> **ASStateSnapshot para Persistência:** Utilizaremos snapshots nativos para salvar o progresso dos jogadores. Isso exige rigor no determinismo.

## Mudanças Propostas

### [Componente] Projeto Demo (`demo/`)

#### [NOVO] [main.tscn](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/main.tscn)

Cena central. Inclui UI de 'Seleção de Servidor' (IP, Porta, Username) e ambiente 3D/2D para o gameplay.

#### [NOVO] [player.tscn](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/entities/player.tscn)

Entidade controlada pelo jogador:

- **ASComponent:** Gerencia habilidades e atributos, com validação de servidor.
- **BTPlayer (LimboAI):** Ativado apenas via `TestMode` para automação de testes.
- **Modos de Jogo:**
  - **Automated (CI):** Player assume a IA e completa a quest tutorial sozinho.
  - **Manual (Humano):** Jogabilidade tradicional para diversão e validação manual.

#### [NOVO] [world_manager.gd](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/scripts/world_manager.gd)

Singleton de gerenciamento de sessão:

- Métodos: `start_singleplayer()`, `start_host(port)`, `connect_client(ip, port)`, `start_dedicated(port)`.
- **Autenticação:** Handler `_on_peer_connected` que aguarda um payload de login antes de instanciar o Player.

#### [NOVO] [save_system.gd](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/scripts/save_system.gd)

Lógica de persistência:

- Estrutura: `user://saves/[world_name]/players/[username].tres`.
- **Pre-seed:** Gerar 10 contas de teste (Player01-Player10) com estados variados para validação imediata.

### [Componente] Testes (`src/tests/` & `utility/tests/`)

#### [NOVO] [test_as_unit.h](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/tests/unit/test_as_unit.h)

Testes unitários atômicos focados puramente nas regras de negócio do Ability System.

#### [NOVO] [test_as_limbo_integration.h](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/tests/integration/test_as_limbo_integration.h)

Testes de integração nativa (C++) focado na interoperabilidade entre AS e LimboAI (ex: BTNodes consumindo a API nativa do AS).

#### [NOVO] [mp_client.py](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/utility/tests/mp_client.py)

Simulador de cliente em Python:

- Realiza handshake de login e validação de save.
- **Testes de Segurança:** Tenta disparar RPCs de habilidades não possuídas ou modificar atributos via rede para validar a segurança do servidor.

### [Componente] Infraestrutura CI (`.github/workflows/`)

#### [MODIFICAR] [🔗 CI/CD Runner (runner.yml)](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/runner.yml)

Orquestrador central que gerencia o ciclo de vida completo: Checks -> Builds -> Tests -> Packaging -> Release.

#### [NOVO] [🧪 All Tests (all_tests.yml)](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/all_tests.yml)

Distribuição de testes para cobertura 300%:

- **Unit Tests:** Validação atômica de lógica de negócio C++.
- **C++ Integration:** Interoperabilidade nativa (AS + LimboAI) compilada lado a lado.
- **Singleplayer:** Teste de gameplay funcional (Playtest) usando a cena `demo/`.
- **Multiplayer:** Stress test real distribuído em 10 instâncias paralelas.

#### [NOVO] [🧱 Test Infrastructure (test_infrastructure.yml)](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_infrastructure.yml)

Garante a consistência ambiental:

- Compila a GDExtension em modo **padrão**, servindo como base para todos os testes.
- Faz o download da versão exata do Godot Headless.
- Gerencia o cache SCons (`Save/Restore`) para reaproveitamento nos jobs seguintes.
- Armazenta os artefatos `dist/godot` e `dist/standard`.

#### [NOVO] [🧪 Unit Tests (test_unit.yml)](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_unit.yml)

Executa uma compilação **incremental** (via MD5 cache) com o parâmetro `tests=unit` e roda os testes unitários. Focado em lógica pura de C++ sem dependência de cena.

#### [NOVO] [🧪 Unit Integration Tests (test_unit_integration.yml)](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_unit_integration.yml)

Executa uma compilação **incremental** (via MD5 cache) com o parâmetro `tests=integration` e valida a integração nativa (C++) entre Ability System e LimboAI.

#### [NOVO] [🎮 Singleplayer Tests (test_singleplayer.yml)](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_singleplayer.yml)

Simula um cenário de playthrough real usando `demo/main.tscn`.

- Valida persistência de save.
- Valida o fluxo completo de quests e combate local.

#### [NOVO] [🌐 Multiplayer Tests (test_multiplayer.yml)](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_multiplayer.yml)

Arquitetura de simulação distribuída:

- **Parallel Strategy:** 10 jobs simultâneos no GHA, cada um simulando um `PlayerID` único.
- **Análise de Logs:** Após a execução, o `log_analyzer.py` consolida os logs em um relatório Markdown com gráficos Mermaid.
- **Simulação de Rede:** Injeta instabilidade via `net_simulator.py` para validar resiliência.

---

## Plano de Verificação

### Testes Automatizados

- Execução local via SCons: `python -m SCons tests=unit` (suporta também `tests=multiplayer`, `tests=integration`, etc.).
- GHA: Validar que `Singleplayer Tests` e `Multiplayer Tests` utilizam a mesma cena `demo/main.tscn`.

### Verificação Manual

- Iniciar a demo, equipar uma habilidade, fechar e reabrir para validar o save.
- Abrir duas instâncias (Server/Client) e validar a replicação de tags.
