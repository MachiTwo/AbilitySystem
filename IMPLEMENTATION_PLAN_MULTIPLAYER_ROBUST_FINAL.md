# 🚀 Plano Definitivo - Infraestrutura Multiplayer Robusta & Ambiciosa

**Data**: 2026-04-04
**Status**: Pronto para Execução
**Duração**: 25-30 dias
**Complexidade**: ⭐⭐⭐⭐⭐ Muito Alta
**Prioridade**: CRÍTICA

---

## 📌 Resposta Direta às Suas Perguntas

### ❓ "Como está funcionando o teste multiplayer?"

**RESPOSTA**: O teste multiplayer **atual é apenas unitário** - executa testes C++ em contexto ENet, mas:

- ❌ Nenhum gameplay real
- ❌ Sem sincronização de movimento/combate
- ❌ Sem IA para automação
- ❌ Sem demo build
- ❌ Sem latência simulada
- ❌ Sem desconexão/reconexão real

### ❓ "Você desenvolveu IA dos players para teste automatizado?"

**RESPOSTA**: Não, mas vou desenvolver:

- ✅ **LimboAI Behavior Tree** determinístico (`player_ai.bt`)
- ✅ **Seed-based** para reproduzibilidade
- ✅ **Automático** (sem input manual)
- ✅ **Inteligente** (patrolha, ataca, usa habilidades)
- ✅ **Sincronizado** entre todos players

---

## 🎯 O Que Será Implementado (Visão Geral)

```
ANTES (Current):
Server ← [Unitário] → Client 1-10
         (apenas testes C++)

DEPOIS (Proposed):
┌─────────────────────────────────────────┐
│  🎮 DEMO MULTIPLAYER REAL               │
├─────────────────────────────────────────┤
│                                          │
│  Server (player_id=0)                   │
│  ├─ Headless executable (compilado)     │
│  ├─ Autoridade de todas decisões        │
│  ├─ Snapshot capture (10 ticks)         │
│  └─ Distribui estado para clientes      │
│                                          │
│  10 Clients (player_id=1-10)            │
│  ├─ Headless executables                │
│  ├─ IA LimboAI (comportamento auto)     │
│  ├─ Prediction + reconciliação           │
│  ├─ Snapshot validation                 │
│  └─ Latência simulada (50-200ms)        │
│                                          │
│  Network Simulation (Python)            │
│  ├─ Latência variável                   │
│  ├─ Packet loss                         │
│  ├─ Kill/restart aleatório              │
│  └─ State hash comparison               │
│                                          │
│  C++ Tests (Multiplayer)                │
│  ├─ Snapshot/Rollback                   │
│  ├─ Prediction Correction               │
│  ├─ Serialization                       │
│  └─ Latency Variance                    │
│                                          │
│  Python Utilities (Orchestration)       │
│  ├─ Demo build                          │
│  ├─ Server launcher                     │
│  ├─ Client launcher com retry           │
│  ├─ Network simulation                  │
│  └─ State validator                     │
│                                          │
│  CI/CD Pipeline                         │
│  ├─ Build demo headless                 │
│  ├─ Run 10 clients simultâneos          │
│  ├─ Stress test (chaos)                 │
│  ├─ Analyze snapshots                   │
│  └─ Report comprehensive                │
│                                          │
└─────────────────────────────────────────┘
```

---

## 🔨 Fase 1: Core C++ Multiplayer Tests (Dias 1-5)

### 1.1 NEW: test_as_multiplayer_sync.h (Bridge Test)

**Arquivo**: `src/tests/bridge/test_as_multiplayer_sync.h` (~400 linhas)

**Objetivo**: Testar sincronização low-level entre múltiplos ASComponents

```cpp
// Exemplo de teste
TEST_CASE("[ASComponent] Multiplayer Sync - Server Correction") {
    // Simular 2 players
    ASComponent *server_comp = create_test_component();
    ASComponent *client_comp = create_test_component();

    // Client predicts movement
    client_comp->set_attribute_base_value_by_tag("Health", 85.0f);
    client_comp->add_tag("state.hurt");

    // Server validates diferentemente (damage is 90 not 85)
    server_comp->set_attribute_base_value_by_tag("Health", 90.0f);

    // Client snapshot before correction
    auto client_snap_before = client_comp->capture_snapshot_state();

    // Server sends correction via RPC (simulated)
    auto server_snap = server_comp->capture_snapshot_state();

    // Client applies correction
    client_comp->restore_snapshot_state(server_snap);

    // Verify client matches server exactly
    CHECK(client_comp->get_attribute_base_value_by_tag("Health")
          == doctest::Approx(server_comp->get_attribute_base_value_by_tag("Health")));
    CHECK(client_comp->get_tags() == server_comp->get_tags());
}
```

**Testes Principais**:

1. ✅ Server-client snapshot matching
2. ✅ Rollback determinism
3. ✅ Tag synchronization
4. ✅ Attribute precision under latency
5. ✅ Multi-player cascade (10 players)
6. ✅ Ability activation sync

### 1.2 NEW: test_as_prediction_correction.h

Testa **input prediction + server reconciliation**:

- Cliente prediz movimento
- Server valida
- Mismatch → rollback + correction RPC
- Cliente aplica fix

### 1.3 NEW: test_as_network_serialization.h

Testa **encode/decode de state**:

- Serializar ASComponent para Dictionary
- Transmitir (simular rede)
- Deserializar no cliente
- Verificar bit-perfect match

### 1.4 UPDATE: test_main.cpp

Adicionar defines para testes multiplayer:

```cpp
#define AS_MULTIPLAYER_TESTS_ENABLED 1
#include "src/tests/bridge/test_as_multiplayer_sync.h"
#include "src/tests/bridge/test_as_prediction_correction.h"
#include "src/tests/bridge/test_as_network_serialization.h"
```

---

## 🎮 Fase 2: Demo Refactor (Dias 6-10)

### 2.1 REMOVE: Static Player Instance

**Arquivo**: `demo/scenes/level.tscn`

**Problema**: Player é hardcoded na cena
**Solução**: Usar `MultiplayerSpawner` + dinâmico spawn

```gdscript
# demo/scenes/level.gd (atualizado)

extends Node2D

@export var player_scene: PackedScene = preload("res://player/player.tscn")

var player_container: Node

func _ready():
    # Remove player hardcoded
    if has_node("Player"):
        $Player.queue_free()

    # Criar container para players dinâmicos
    player_container = Node.new()
    player_container.name = "Players"
    add_child(player_container)

    # Conectar a MultiplayerGameManager
    var mp_manager = get_node("/root/MultiplayerGameManager")
    mp_manager.spawn_player_requested.connect(_on_spawn_player)

func _on_spawn_player(player_id: int, position: Vector2):
    """Spawn um novo player dinamicamente"""
    var player = player_scene.instantiate()
    player.name = "Player_%d" % player_id
    player.position = position
    player.network_id = player_id
    player_container.add_child(player)

    # Sync via MultiplayerSynchronizer
    player.setup_network_sync(player_id)
```

### 2.2 NEW: MultiplayerController (Autoload)

**Arquivo**: `demo/autoload/multiplayer_controller.gd` (novo, ~350 linhas)

**Responsabilidades**:

- Setup ENet (server/client)
- Sincronizar ticks (60Hz)
- Distribuir snapshots
- RPC de correção
- Lifecycle de players

```gdscript
# demo/autoload/multiplayer_controller.gd

extends Node

const TICK_RATE = 60
const SNAPSHOT_INTERVAL = 10

var is_server: bool = false
var is_client: bool = false
var current_tick: int = 0

signal spawn_player_requested(player_id: int, position: Vector2)
signal player_joined(player_id: int)
signal player_left(player_id: int)
signal tick_processed(tick: int)

func _ready():
    var player_id = _get_player_id()

    if player_id == 0:
        await _setup_server()
    else:
        await _setup_client(player_id)

func _setup_server():
    is_server = true

    var peer = ENetMultiplayerPeer.new()
    peer.create_server(7777, 11)

    multiplayer.multiplayer_peer = peer
    multiplayer.peer_connected.connect(_on_peer_connected)
    multiplayer.peer_disconnected.connect(_on_peer_disconnected)

    # Spawn 10 AI players para teste
    for player_id in range(1, 11):
        var spawn_pos = _get_spawn_position(player_id)
        spawn_player_requested.emit(player_id, spawn_pos)

func _process(delta):
    if is_server or is_client:
        _process_tick()

func _process_tick():
    # Core tick logic
    # - Server: validate + apply + broadcast
    # - Client: predict + receive corrections

    current_tick += 1
    tick_processed.emit(current_tick)

@rpc("call_remote")
func synchronize_state(player_id: int, state_dict: Dictionary):
    """Server broadcasts state to all clients"""
    if multiplayer.is_server():
        for peer_id in multiplayer.get_peers():
            _synchronize_state.rpc_id(peer_id, player_id, state_dict)

@rpc("authority")
func apply_correction(player_id: int, snapshot_data: Dictionary, tick: int):
    """Server sends correction to specific client"""
    # Client applies correction when received
    pass

func _get_player_id() -> int:
    var env = OS.get_environment("MP_GHA_PLAYER_ID")
    return int(env) if env else -1

func _get_spawn_position(player_id: int) -> Vector2:
    # Spawn spread em diferentes plataformas
    var positions = [
        Vector2(100, 300),
        Vector2(500, 300),
        Vector2(900, 300),
        Vector2(200, 500),
        Vector2(600, 500),
        Vector2(200, 100),
        Vector2(600, 100),
        Vector2(400, 400),
        Vector2(800, 200),
        Vector2(300, 600),
    ]
    return positions[player_id - 1] if player_id <= len(positions) else Vector2(400, 300)
```

### 2.3 UPDATE: player.tscn

Adicionar sincronização de rede:

```gdscript
# player.tscn (atualizado com nós)

Player (CharacterBody2D)
├─ [NEW] MultiplayerSynchronizer
│  ├─ sync_position: true
│  ├─ sync_velocity: true
│  ├─ sync_animation: true
│  └─ authority: self.network_id
│
├─ ASComponent
├─ AnimationPlayer
├─ [NEW] BehaviorTreePlayer (para IA)
│  └─ behavior_tree: "res://resources/behavior_trees/mp_player_ai.tres"
│
└─ (resto da estrutura)
```

### 2.4 NEW: player_ai.bt (LimboAI Behavior Tree)

**Arquivo**: `demo/resources/behavior_trees/mp_player_ai.tres` (novo)

**Estrutura** (Deterministic + Seed-based):

```
BTPlayer: mp_player_ai
├─ Root: Sequence [deterministic, seed=42]
│
├─ Behavior 1: Patrol (60% do tempo)
│  ├─ SelectPatform [Random]
│  ├─ WalkToTarget [distance < 50px]
│  │  └─ Apply "motion.walk" tag
│  ├─ IF obstacle detected
│  │  └─ Jump [motion.jump]
│  │
│  └─ [Timer: 180 ticks] change direction
│
├─ Behavior 2: Combat (20% do tempo)
│  ├─ DetectNearbyPlayers [range 200px]
│  ├─ IF found
│  │  ├─ Move towards closest
│  │  ├─ Attack Selection
│  │  │  ├─ IF stamina > 50 → Dash Attack
│  │  │  ├─ ELIF health < 50 → Evade
│  │  │  └─ ELSE → Light Attack
│  │  │
│  │  └─ [cooldown: 120 ticks]
│  │
│  └─ ELSE → Return to Patrol
│
├─ Behavior 3: Ability Use (10% tempo)
│  ├─ Check stamina > 70
│  ├─ Select Ability (weighted random)
│  │  ├─ 40% dash
│  │  ├─ 30% hyperdash
│  │  ├─ 20% attack.charged
│  │  └─ 10% nothing (cooldown)
│  │
│  └─ Activate ability
│
└─ [Blackboard Sync every 10 ticks]
   ├─ position → RPC
   ├─ health → from ASComponent
   ├─ stamina → from ASComponent
   └─ ai_state → for debugging
```

**Características**:

- ✅ **Determinístico**: seed-based RNG
- ✅ **Sincronizado**: posição via RPC
- ✅ **Inteligente**: patrolha, combate, habilidades
- ✅ **Testável**: sem input externo necessário

---

## 🌐 Fase 3: Python Orchestration (Dias 11-16)

### 3.1 MODIFY: runner.py (Chaos Engine)

**Arquivo**: `utility/multiplayer/runner.py` (reescrito, ~200 linhas)

```python
#!/usr/bin/env python3

import subprocess
import time
import os
import random
import threading
from pathlib import Path

class MultiplayerTestRunner:
    """
    Runner para teste multiplayer robusto com:
    - Latência simulada (50-200ms)
    - Chaos monkey (kill/restart random clients)
    - State hash validation
    - Detailed logging
    """

    def __init__(self, demo_binary: str, num_players: int = 10):
        self.demo_binary = demo_binary
        self.num_players = num_players
        self.processes = {}
        self.log_dir = Path("utility/multiplayer/logs")
        self.log_dir.mkdir(parents=True, exist_ok=True)

        # Network simulation params
        self.latency_min_ms = 50
        self.latency_max_ms = 200

        # Chaos params
        self.chaos_enabled = True
        self.chaos_kill_probability = 0.1  # 10% chance per client per tick
        self.chaos_kill_duration_s = random.randint(5, 15)

    def launch_server(self) -> subprocess.Popen:
        """Inicia servidor"""
        print("[MP-RUN] Launching server...")

        env = os.environ.copy()
        env["MP_GHA_PLAYER_ID"] = "0"
        env["MP_SERVER_PORT"] = "7777"

        log_file = self.log_dir / "server.log"

        proc = subprocess.Popen(
            [self.demo_binary, "--headless"],
            env=env,
            stdout=open(log_file, "w"),
            stderr=subprocess.STDOUT
        )

        time.sleep(2)  # Wait for server ready
        print(f"[MP-RUN] Server started (PID {proc.pid})")
        return proc

    def launch_client(self, player_id: int) -> subprocess.Popen:
        """Inicia cliente com latência simulada"""
        latency_ms = random.randint(self.latency_min_ms, self.latency_max_ms)

        env = os.environ.copy()
        env["MP_GHA_PLAYER_ID"] = str(player_id)
        env["MP_SERVER_PORT"] = "7777"
        env["MP_SIMULATED_LATENCY_MS"] = str(latency_ms)

        log_file = self.log_dir / f"player_{player_id}.log"

        proc = subprocess.Popen(
            [self.demo_binary, "--headless"],
            env=env,
            stdout=open(log_file, "w"),
            stderr=subprocess.STDOUT
        )

        print(f"[MP-RUN] Client {player_id} launched (latency {latency_ms}ms)")
        return proc

    def chaos_monkey(self):
        """Thread que mata/reinicia clientes aleatoriamente"""
        while True:
            for player_id in range(1, self.num_players + 1):
                if not self.chaos_enabled:
                    break

                if random.random() < self.chaos_kill_probability:
                    print(f"[CHAOS] Killing player {player_id}...")

                    proc = self.processes.get(player_id)
                    if proc and proc.poll() is None:
                        proc.kill()

                        # Wait to reconnect
                        time.sleep(self.chaos_kill_duration_s)

                        # Restart
                        print(f"[CHAOS] Restarting player {player_id}...")
                        new_proc = self.launch_client(player_id)
                        self.processes[player_id] = new_proc

            time.sleep(1)  # Check every second

    def validate_state_hashes(self) -> bool:
        """Compara state hashes de todos players"""
        hashes = {}

        for log_file in self.log_dir.glob("player_*.log"):
            with open(log_file) as f:
                for line in f:
                    if "[STATE_HASH]" in line:
                        player_id = int(log_file.stem.split("_")[1])
                        # Parse hash from line
                        hash_val = line.split("[STATE_HASH]")[1].strip()
                        hashes[player_id] = hash_val

        # Check all hashes match
        if len(hashes) > 1:
            first_hash = list(hashes.values())[0]
            for player_id, hash_val in hashes.items():
                if hash_val != first_hash:
                    print(f"[ERROR] Player {player_id} hash mismatch!")
                    return False

        print(f"[MP-RUN] ✅ All {len(hashes)} players synchronized")
        return True

    def run_test(self, duration_s: int = 300) -> bool:
        """Executa teste completo"""
        print(f"[MP-RUN] Starting {self.num_players}-player test ({duration_s}s)...")

        # Launch server
        self.processes[0] = self.launch_server()

        # Launch clients
        for player_id in range(1, self.num_players + 1):
            self.processes[player_id] = self.launch_client(player_id)

        # Start chaos monkey thread
        if self.chaos_enabled:
            threading.Thread(target=self.chaos_monkey, daemon=True).start()

        # Wait for test duration
        start = time.time()
        while time.time() - start < duration_s:
            # Check if server is alive
            if self.processes[0].poll() is not None:
                print("[ERROR] Server crashed!")
                return False

            time.sleep(5)

        print("[MP-RUN] Test duration reached")

        # Kill all processes
        for proc in self.processes.values():
            if proc.poll() is None:
                proc.kill()

        # Validate state hashes
        return self.validate_state_hashes()

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--demo-binary", required=True)
    parser.add_argument("--num-players", type=int, default=10)
    parser.add_argument("--duration", type=int, default=300)
    args = parser.parse_args()

    runner = MultiplayerTestRunner(args.demo_binary, args.num_players)
    success = runner.run_test(args.duration)

    exit(0 if success else 1)
```

### 3.2 MODIFY: log_analyzer.py (Deep Analysis)

**Arquivo**: `utility/multiplayer/log_analyzer.py` (reescrito, ~300 linhas)

````python
#!/usr/bin/env python3

import re
from pathlib import Path
from collections import defaultdict
import json

class MultiplayerLogAnalyzer:
    """
    Análise profunda de logs multiplayer:
    - Sincronização de estado
    - Detecção de desync
    - Métricas de rede
    - Visualização Mermaid
    """

    def __init__(self, log_dir: str):
        self.log_dir = Path(log_dir)
        self.results = defaultdict(list)
        self.state_hashes = defaultdict(list)
        self.divergence_points = []

    def analyze_all_logs(self) -> dict:
        """Parse todos logs e extrai métricas"""

        for log_file in self.log_dir.glob("player_*.log"):
            player_id = int(log_file.stem.split("_")[1])

            with open(log_file) as f:
                tick = 0
                for line in f:
                    # Parse snapshot
                    if "[SNAPSHOT]" in line:
                        tick += 1
                        self._parse_snapshot(player_id, tick, line)

                    # Parse state hash
                    if "[STATE_HASH]" in line:
                        hash_val = line.split("[STATE_HASH]")[1].strip()
                        self.state_hashes[player_id].append((tick, hash_val))

                    # Parse errors
                    if "[ERROR]" in line:
                        self.results[player_id].append({
                            "tick": tick,
                            "error": line
                        })

        self._detect_divergence()
        return self._build_report()

    def _detect_divergence(self):
        """Detecta onde players divergem"""
        if not self.state_hashes:
            return

        all_players = list(self.state_hashes.keys())
        if len(all_players) < 2:
            return

        # Compare first player to others
        player_0 = all_players[0]
        hashes_0 = dict(self.state_hashes[player_0])

        for player_id in all_players[1:]:
            hashes_other = dict(self.state_hashes[player_id])

            for tick, hash_0 in hashes_0.items():
                if tick in hashes_other:
                    if hashes_other[tick] != hash_0:
                        self.divergence_points.append({
                            "tick": tick,
                            "player_1": player_0,
                            "player_2": player_id
                        })

    def _build_report(self) -> dict:
        """Constrói relatório consolidado"""

        report = {
            "total_players": len(self.state_hashes),
            "divergences": len(self.divergence_points),
            "state_hash_matches": len([p for p in self.state_hashes.values() if p])
        }

        if self.divergence_points:
            report["first_divergence_tick"] = self.divergence_points[0]["tick"]
            report["mermaid_diagram"] = self._generate_desync_diagram()
        else:
            report["status"] = "✅ ALL PLAYERS SYNCHRONIZED"

        return report

    def _generate_desync_diagram(self) -> str:
        """Gera diagrama Mermaid mostrando desync"""

        diagram = """
sequenceDiagram
    participant Server as Server (0)
        """

        for player_id in range(1, 11):
            diagram += f"\n    participant P{player_id} as Player {player_id}"

        # Add timeline
        for point in self.divergence_points[:5]:  # Show first 5
            tick = point["tick"]
            p1 = point["player_1"]
            p2 = point["player_2"]

            diagram += f"\n    Note over P{p1},P{p2}: ❌ Divergence at tick {tick}"

        diagram += "\n    ```"
        return diagram

if __name__ == "__main__":
    analyzer = MultiplayerLogAnalyzer("utility/multiplayer/logs")
    report = analyzer.analyze_all_logs()

    print(json.dumps(report, indent=2))
````

---

## 🔧 Fase 4: CI/CD Pipeline (Dias 17-20)

### 4.1 NEW: build_demo.yml

**Arquivo**: `.github/workflows/build_demo.yml` (novo)

```yaml
name: 🔨 Build Demo Multiplayer

on:
  workflow_call:
    outputs:
      demo-path:
        value: dist/demo/demo.x86_64

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4.2.2

      - name: Install Dependencies
        run: |
          apt-get update
          apt-get install -y scons python3

      - name: Build Ability System
        run: scons target=template_debug platform=linux arch=x86_64

      - name: Setup Godot
        run: |
          # Download Godot 4.6.1 headless
          wget https://github.com/godotengine/godot-builds/releases/download/...
          chmod +x godot

      - name: Export Demo
        run: |
          ./godot --headless --export-debug "Linux/X11" dist/demo/demo.x86_64

      - name: Cache Demo Build
        uses: actions/cache/save@v4
        with:
          path: dist/demo/
          key: ${{ github.sha }}-demo-build
```

### 4.2 MODIFY: test_multiplayer.yml (Stress Testing)

```yaml
name: 🌐 Multiplayer Stress Test

on:
  push:
    branches: [dev]

jobs:
  build-demo:
    uses: ./.github/workflows/build_demo.yml

  multiplayer-normal:
    name: 🌐 Normal Network
    needs: build-demo
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4.2.2

      - name: Restore Demo
        uses: actions/cache/restore@v4
        with:
          path: dist/demo/
          key: ${{ github.sha }}-demo-build

      - name: Run Test
        timeout-minutes: 10
        run: |
          python utility/multiplayer/runner.py \
            --demo-binary dist/demo/demo.x86_64 \
            --num-players 10 \
            --duration 300

      - name: Analyze
        run: |
          python utility/multiplayer/log_analyzer.py \
            --log-dir utility/multiplayer/logs

  multiplayer-chaos:
    name: 🔥 Chaos (Disconnects)
    needs: build-demo
    runs-on: ubuntu-latest

    steps:
      # Similar to normal, mas com chaos enabled
```

---

## 🧪 Fase 5: Testing & Validation (Dias 21-25)

### 5.1 Local Test Checklist

```bash
# 1. Build demo
python build/build.py

# 2. Run local server + 3 clients
python utility/multiplayer/runner.py \
  --demo-binary dist/demo/demo.x86_64 \
  --num-players 3

# 3. Verify logs
cat utility/multiplayer/logs/player_1.log | grep "\[SNAPSHOT\]" | wc -l
# Esperado: ~18000 (300s × 60Hz)

# 4. Validate hashes
python utility/multiplayer/log_analyzer.py \
  --log-dir utility/multiplayer/logs
# Esperado: ✅ ALL PLAYERS SYNCHRONIZED
```

### 5.2 Validation Metrics

```
✅ Network Synchronization
   - 10 players conectados simultaneamente
   - State hash matches 100% (ou flagged)
   - Divergence detected e reportado

✅ Gameplay Mechanics
   - Players movem entre plataformas
   - Jump/Dash funcionam
   - Combate sincroniza
   - Abilities ativam corretamente

✅ Resilience
   - Desconexão detectada
   - Reconexão automática
   - Snapshot aplicado corretamente
   - Sem crash em 5+ minutos

✅ Performance
   - Latência 50-200ms simulada
   - Nenhuma stuttering visível
   - CPU < 80%
   - Memory < 500MB (per instance)
```

---

## 📊 Expected Deliverables

### Code

```
✅ C++ Tests (~1000 linhas)
   ├─ test_as_multiplayer_sync.h
   ├─ test_as_prediction_correction.h
   └─ test_as_network_serialization.h

✅ GDScript (~800 linhas)
   ├─ multiplayer_controller.gd
   ├─ network_player.gd (wrapper)
   └─ Scene updates

✅ LimboAI
   ├─ mp_player_ai.tres
   └─ Custom tasks

✅ Python (~600 linhas)
   ├─ runner.py (reescrito)
   ├─ log_analyzer.py (reescrito)
   └─ build utilities

✅ CI/CD
   ├─ build_demo.yml
   └─ test_multiplayer_full.yml

✅ Demo Executable
   ├─ dist/demo/demo.x86_64
   └─ Fully functional headless
```

### Metrics

```
Performance:
- 10 clients em ENet
- 60Hz tick rate
- 10-tick snapshot interval
- 50-200ms latência simulada
- 99.9% sync accuracy (ou better)

Quality:
- 100% test coverage (C++)
- Deterministic simulation
- Reproducible with seed
- Clean logs + analysis
```

---

## 🚨 Critical Success Factors

| Item                | Requirement                 | Verification                |
| ------------------- | --------------------------- | --------------------------- |
| **Determinism**     | Mesmos input → Mesmo output | Seed-based RNG, tick-based  |
| **Synchronization** | 10 players em sync          | State hash comparison       |
| **Rollback**        | Correction snapshot applica | Attribute/tag verification  |
| **Latency**         | 50-200ms simulado           | Python intercept + delay    |
| **Chaos**           | Kill/restart clientes       | State recovery verification |
| **CI/CD**           | Execução automática         | GHA workflow complete       |

---

## 📝 Summary

Este plano implementa **infraestrutura multiplayer de produção** com:

✅ **Real Gameplay** - Demo real com 10 AI players automatizados
✅ **Robust Sync** - Snapshot/Rollback com C++ tests
✅ **Network Simulation** - Latência, packet loss, chaos
✅ **Deterministic** - Reproducível com seed
✅ **Automated IA** - LimboAI behavior trees
✅ **Comprehensive Testing** - C++, Python, CI/CD
✅ **Professional Reports** - Detailed metrics + analysis

**Timeline**: 25-30 dias para implementação completa

---

**Status**: Ready for Implementation
**Reviewer**: You (Bruno)
**Date**: 2026-04-04
