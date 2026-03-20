# Unified Demo & Test Suite Task List

- [x] **Phase 1: Infraestrutura de Testes Granulados**
  - [x] Create [utility/tests/runner.py](file:///c:/Users/bruno/Desktop/AbilitySystem/utility/tests/runner.py) (Simulation runner: singleplayer, multiplayer)
  - [x] Update [SConstruct](file:///c:/Users/bruno/Desktop/AbilitySystem/SConstruct) with `EnumVariable` for compilation of `unit` and `integration` tests
  - [x] Ensure `src/` directory is 100% Pure Core (no test code)
  - [x] Configure Headless-by-default in the runner

- [ ] **Phase 2: MMORPG Entidades & Gameplay**
  - [ ] Create `Player` entity with [ASComponent](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/scene/as_component.h#300-301) and `BTPlayer` (LimboAI)
  - [ ] Implement Character Creation (Name, Race, Class selection)
  - [ ] Implement Starting Inventory & Coins based on Class
  - [ ] Implement Quest System (Tutorial Quest)
  - [ ] Implement `TestMode` flag to automate Player actions via AI
  - [ ] Implement deterministic movement (Physics Ticking)
  - [ ] Setup initial Attribute Sets (Health, Mana, Strength)
  - [ ] Create a starter [ASAbility](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/resources/as_ability.cpp#555-557) (Strike/Fireball/Stealth)

- [x] **Phase 3: Persistence & Authentication**
  - [x] Implement [ASStateSnapshot](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/src/resources/as_state_snapshot.cpp#174-175) capture/restore logic
  - [x] Create `SaveSystem` with 10 pre-configured player profiles
  - [x] Implement simple Authentication handshake in `WorldManager`
  - [x] Verify persistence across session restarts

- [ ] **Phase 4: Advanced Multiplayer Testing & Stress**
  - [ ] Configure Headless Linux Export for the Demo
  - [ ] Create Python-based Client Simulator (`mp_client.py`)
  - [ ] Implement Security/Anti-Cheat validation (Server-side Ability checks)
  - [ ] Implement network instability simulation (Latency, Packet Loss)
  - [ ] Setup 10-player concurrency validation tests

- [ ] **Phase 4.1: Native C++ Integration Tests (Dedicated Branch)**
  - [ ] Setup `src/tests/unit` and `src/tests/integration` with `.gitkeep`
  - [ ] Create [test_unit_integration.yml](file:///c:/Users/bruno/Desktop/AbilitySystemPlugin/.github/workflows/test_unit_integration.yml) workflow
  - [ ] Implement C++ test cases for side-by-side compilation of AS + LimboAI

- [ ] **Phase 5: Refinement & Wow-Factor**
  - [ ] Add visual feedback (Cues, Particles)
  - [ ] Implement basic AI (NPCs) using the same logic as Players
  - [ ] Final polishing and documentation
