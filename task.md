# Lista de Tarefas - Demo Unificada & Suíte de Testes

- [x] **Fase 1: Infraestrutura de Testes Granulados**
  - [x] Criar [utility/tests/runner.py](file:///c:/Users/bruno/Desktop/AbilitySystem/utility/tests/runner.py) (Executor de simulação: singleplayer, multiplayer)
  - [x] Atualizar [SConstruct](file:///c:/Users/bruno/Desktop/AbilitySystem/SConstruct) com `EnumVariable` para compilação de testes `unit` e `integration`
  - [x] Garantir que o diretório `src/` seja 100% Core Puro (sem código de teste)
  - [x] Configurar Headless-por-padrão no runner

- [ ] **Fase 2: Entidades MMORPG & Gameplay**
  - [ ] Criar entidade `Player` com [ASComponent](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/scene/as_component.h#300-301) e `BTPlayer` (LimboAI)
  - [ ] Implementar Criação de Personagem (Nome, Raça, seleção de Classe)
  - [ ] Implementar Inventário Inicial & Moedas baseadas na Classe
  - [ ] Implementar Sistema de Missões (Missão de Tutorial)
  - [ ] Implementar flag `TestMode` para automatizar ações do Jogador via IA
  - [ ] Implementar movimento determinístico (Physics Ticking)
  - [ ] Configurar Conjuntos de Atributos iniciais (Health, Mana, Strength)
  - [ ] Criar uma [ASAbility](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/resources/as_ability.cpp#555-557) inicial (Strike/Fireball/Stealth)

- [x] **Fase 3: Persistência & Autenticação**
  - [x] Implementar lógica de captura/restauração do [ASStateSnapshot](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/resources/as_state_snapshot.cpp#174-175)
  - [x] Criar `SaveSystem` com 10 perfis de jogador pré-configurados
  - [x] Implementar handshake de Autenticação simples no `WorldManager`
  - [x] Verificar persistência entre reinicializações de sessão

- [ ] **Fase 4: Testes Multiplayer Avançados & Estresse**
  - [ ] Configurar Exportação Linux Headless para a Demo
  - [ ] Criar Simulador de Cliente em Python (`mp_client.py`)
  - [ ] Implementar validação de Segurança/Anti-Cheat (verificações de Habilidades no Servidor)
  - [ ] Implementar simulação de instabilidade de rede (Latência, Perda de Pacotes)
  - [ ] Configurar testes de validação de concorrência com 10 jogadores

- [x] **Fase 4.1: Testes de Integração C++ Nativa**
  - [x] Configurar `src/tests/unit` e `src/tests/integration` com `.gitkeep`
  - [x] Criar workflow [test_unit_integration.yml](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_unit_integration.yml)
  - [x] Implementar casos de teste C++ para compilação lado a lado de AS + LimboAI

- [x] **Fase 6: Infraestrutura CI/CD & Automação**
  - [x] **Runner:** Orquestração Global ([runner.yml](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/runner.yml))
  - [x] **Infraestrutura:** Cache de artefatos de build ([test_infrastructure.yml](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_infrastructure.yml))
  - [x] **Suíte de Testes:** fan-out para unit, integration, single e multiplayer ([all_tests.yml](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/all_tests.yml))
  - [x] **Paralelismo Multiplayer:** Simulação distribuída de 10 jogadores ([test_multiplayer.yml](file:///c:/Users/bruno/Desktop/AbilitySystem/.github/workflows/test_multiplayer.yml))
  - [x] **Diagnósticos:** Gerador de relatório log consolidado ([log_analyzer.py](file:///c:/Users/bruno/Desktop/AbilitySystem/utility/multiplayer/log_analyzer.py))

- [ ] **Fase 7: Refinamento & Fator Uau**
  - [ ] Adicionar feedback visual (Cues, Partículas)
  - [ ] Implementar IA básica (NPCs) usando a mesma lógica dos Jogadores
  - [ ] Polimento final e documentação
