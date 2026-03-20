# Implementation Plan Demo e Testes

Este plano descreve a reconstrução do projeto de demonstração e testes do zero, integrando elementos de **MMORPG** (Raças, Classes, Quests) e uma arquitetura unificada de cena que suporta Singleplayer, Multiplayer (LAN/Dedicado) e salvamento persistente por mundo.

## RPG Design Elements

- **Classes:** Warrior (Strength-based), Mage (Mana-based), Rogue (Agility-based).
- **Races:** Human (Balanced), Elf (Magic affinity), Orc (Physical power).
- **Inventory:** Cada classe nasce com armas, consumíveis e moedas iniciais.
- **Quests:** Sistema de missões (Tutorial) para guiar o início do jogador.

## Native C++ Integration Strategy

O teste job `unit-integration` rodará em uma configuração de CI que compila o Ability System e o LimboAI lado a lado. Isso valida:

- **BTNodes Nativos:** Corretude da lógica de ativação de habilidades via Behavior Tree em C++.
- **Engenharia de Bindings:** Garantir que não haja conflitos de registro ou símbolos entre as duas GDExtensions.

## User Review Required

> [!IMPORTANT]
> **Modelo de Mundo Unificado:** O Singleplayer será tecnicamente um servidor local com um único cliente conectado. Isso garante 100% de paridade de lógica entre modos.

> [!CAUTION]
> **ASStateSnapshot para Persistência:** Utilizaremos snapshots nativos para salvar o progresso dos jogadores. Isso exige rigor no determinismo.

## Proposed Changes

### [Component] Demo Project (`demo/`)

#### [NEW] [main.tscn](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/main.tscn)

Cena central. Inclui UI de 'Server Selection' (IP, Porta, Username) e ambiente 3D/2D para o gameplay.

#### [NEW] [player.tscn](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/entities/player.tscn)

Entidade controlada pelo jogador:

- **ASComponent:** Gerencia habilidades e atributos, com validação de servidor.
- **BTPlayer (LimboAI):** Ativado apenas via `TestMode` para automação de testes.
- **Gameplay Modes:**
  - **Automated (CI):** Player assume a IA e completa a quest tutorial sozinho.
  - **Manual (Human):** Jogabilidade tradicional para diversão e validação manual.

#### [NEW] [world_manager.gd](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/scripts/world_manager.gd)

Singleton de gerenciamento de sessão:

- Métodos: `start_singleplayer()`, `start_host(port)`, `connect_client(ip, port)`, `start_dedicated(port)`.
- **Authentication:** Handler `_on_peer_connected` que aguarda um payload de login antes de instanciar o Player.

#### [NEW] [save_system.gd](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/demo/scripts/save_system.gd)

Lógica de persistência:

- Estrutura: `user://saves/[world_name]/players/[username].tres`.
- **Pre-seed:** Gerar 10 contas de teste (Player01-Player10) com estados variados para validação imediata.

### [Component] Tests (`src/tests/` & `utility/tests/`)

#### [NEW] [test_as_unit.h](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/tests/unit/test_as_unit.h)

Testes unitários atômicos focados puramente nas regras de negócio do Ability System.

#### [NEW] [test_as_limbo_integration.h](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/tests/integration/test_as_limbo_integration.h)

Testes de integração nativa (C++) focado na interoperabilidade entre AS e LimboAI (ex: BTNodes consumindo a API nativa do AS).

#### [NEW] [mp_client.py](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/utility/tests/mp_client.py)

Simulador de cliente em Python:

- Realiza handshake de login e validação de save.
- **Security Testing:** Tenta disparar RPCs de habilidades não possuídas ou modificar atributos via rede para validar a segurança do servidor.

### [Component] CI Infrastructure

#### [MODIFY] [test_infrastructure.yml](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/.github/workflows/test_infrastructure.yml)

- Adicionar step de exportação da demo para `Linux Headless`.

#### [MODIFY] [test_multiplayer.yml](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/.github/workflows/test_multiplayer.yml)

- Injetar instabilidade de rede via ferramenta `tc` no ambiente de runner do GitHub para simular perca de pacotes e latência variável.

---

## Verification Plan

### Automated Tests

- Execução local via SCons: `python -m SCons tests=unit`.
- GHA: Validar que `Singleplayer Tests` e `Multiplayer Tests` utilizam a mesma cena `demo/main.tscn`.

### Manual Verification

- Iniciar a demo, equipar uma habilidade, fechar e reabrir para validar o save.
- Abrir duas instâncias (Server/Client) e validar a replicação de tags.
