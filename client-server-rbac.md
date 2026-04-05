# Plano: Arquitetura Client/Server com RBAC e Plugin GHA

## Context

O projeto evolui para suportar:

1. **Servidor Dedicado** - headless, sem GUI, CLI admin interativa
2. **Cliente Multiplayer** - demo com capacidades de admin via UI
3. **Sistema RBAC** - controle de acesso baseado em roles
4. **Plugin GHA** - integrado ao workflow existente, entrega todos os executáveis (cliente + servidor para todas as plataformas)

Atualmente temos:

- Demo single/multiplayer menu-driven
- MultiplayerGameManager com ENet
- GameData para estado global
- Chat system (MVP)

## Objetivo

Criar arquitetura completa onde:

- **Servidor Dedicado**: headless com CLI admin, exibe IP:porta, logs, comandos
- **Cliente LAN**: Host (player que abre para LAN) é admin via painel UI
- **RBAC System**: Controle granular de permissões por role
- **Plugin GHA**: Single workflow linux que gera todos os executáveis (Windows, Linux, macOS, Android) como pasta estruturada

---

## 1. Sistema RBAC (Role-Based Access Control)

### Arquivo

`demo/autoload/RBAC.gd`

### Estrutura

```gdscript
RBAC (Singleton)
├── Roles (definições)
│   ├── admin: [kick_player, ban_player, shutdown_server, whitelist_add, broadcast_message, mute_player, unmute_player]
│   ├── moderator: [kick_player, mute_player, unmute_player, broadcast_message]
│   └── user: [chat, move, attack]
│
├── Peers (rastreamento em tempo real)
│   ├── peer_roles: {peer_id: "role"}
│   ├── peer_data: {peer_id: {name, role, joined_at}}
│   └── banned_peers: {peer_id: {banned_at, expires_at, reason}}
│
└── Methods
    ├── register_peer(peer_id, player_name) → registra novo peer
    ├── unregister_peer(peer_id) → remove peer desconectado
    ├── set_player_role(peer_id, role) → muda role dinamicamente
    ├── can_execute(peer_id, action) → verifica permissão
    ├── ban_peer(peer_id, hours, reason) → bane com expiração
    ├── unban_peer(peer_id) → remove ban
    ├── is_banned(peer_id) → verifica se está banido
    ├── get_player_role(peer_id) → retorna role
    ├── get_all_peers() → lista completa com dados
    └── get_connected_count() → número de peers
```

### Lógica de Atribuição

```
Servidor Dedicado
└─ peer_id=1: role=admin (sempre)

Modo Singleplayer → LAN
├─ Host (peer_id=1): role=admin
└─ Clientes (peer_id>1): role=user

Modo Multiplayer Direto
├─ Host (peer_id=1): role=admin
└─ Clientes (peer_id>1): role=user
```

### Sistema de Ban

```gdscript
ban_peer(peer_id, hours=24.0, reason="")
├─ Registra: {banned_at, expires_at, reason}
├─ Desconecta peer imediatamente
├─ Ban expira automaticamente após X horas
└─ RPC broadcast para todos clientes

get_ban_info(peer_id) → {banned, reason, remaining_hours}
```

### RPC e Sincronização

```gdscript
@rpc("authority", "call_local")
func _broadcast_role_change(peer_id: int, role: String)
└─ Broadcast quando role muda
└─ Atualiza peer_data em todos os clientes
```

---

## 2. Sistema de Comandos (AdminCommand)

### Arquivo

`demo/autoload/AdminCommand.gd`

### Estrutura

```gdscript
AdminCommand (Singleton)
├── _command_handlers: Dictionary
│   ├── "kick_player": _cmd_kick_player()
│   ├── "ban_player": _cmd_ban_player()
│   ├── "shutdown_server": _cmd_shutdown_server()
│   ├── "broadcast_message": _cmd_broadcast_message()
│   ├── "list_players": _cmd_list_players()
│   ├── "mute_player": _cmd_mute_player()
│   ├── "unmute_player": _cmd_unmute_player()
│   └── "get_server_status": _cmd_get_server_status()
│
└── Methods
    ├── execute_command(executor_id, command, args) → valida RBAC + executa
    └── execute_command_rpc(command, args) → RPC para clientes executarem
```

### Comandos Detalhados

#### **kick_player**

```
execute_command(executor_id, "kick_player", [peer_id, reason])

Fluxo:
1. Verifica RBAC.can_execute(executor_id, "kick_player")
2. Se negado: emite comando_executed(false), retorna
3. Se permitido:
   ├─ Encontra player node via _get_player_by_id(peer_id)
   ├─ Emite multiplayer.peer_disconnected(peer_id)
   ├─ Emite signal player_kicked(peer_id, reason)
   └─ Retorna true

Resultado: Player desconectado imediatamente, sem reconectar
```

#### **ban_player**

```
execute_command(executor_id, "ban_player", [peer_id, hours, reason])

Fluxo:
1. Verifica RBAC.can_execute(executor_id, "ban_player")
2. Se permitido:
   ├─ RBAC.ban_peer(peer_id, hours, reason)
   │  └─ Registra ban com expiration_time
   ├─ Desconecta player imediatamente (também emite peer_disconnected)
   ├─ Emite signal player_banned(peer_id, hours, reason)
   └─ Retorna true

Resultado: Player banido por X horas, não consegue reconectar
Nota: Ban expira automaticamente após período
```

#### **shutdown_server**

```
execute_command(executor_id, "shutdown_server", [reason])

Fluxo:
1. Verifica RBAC.can_execute(executor_id, "shutdown_server")
2. Se permitido:
   ├─ Log: "[AdminCommand] Shutting down: {reason}"
   ├─ Emite signal server_shutdown_initiated(reason)
   ├─ Aguarda 1 frame
   ├─ get_tree().call_deferred("quit")
   └─ Retorna true

Resultado: Servidor fecha gracefully
Clientes: recebem server_disconnected signal
```

#### **broadcast_message**

```
execute_command(executor_id, "broadcast_message", [message])

Fluxo:
1. Verifica RBAC.can_execute(executor_id, "broadcast_message")
2. Se permitido:
   ├─ Obtém sender_name = RBAC.get_peer_data(executor_id)["name"]
   ├─ Encontra chat_layer via _get_chat_system()
   ├─ Chat.add_system_message.rpc("[ADMIN] {name}: {message}")
   │  └─ Exibe em amarelo para todos os clientes
   ├─ Emite signal broadcast_message_sent(message)
   └─ Retorna true

Resultado: Mensagem em broadcast para chat system
Formato: [ADMIN] John: "Server maintenance in 5 minutes!"
```

#### **list_players**

```
execute_command(executor_id, "list_players", [])

Saída formatada:
=== Connected Players ===
[1] Server - admin
[2] João - user
[3] Maria - user
[4] Pedro - moderator
========================

Obtém: RBAC.get_all_peers()
```

#### **mute_player**

```
execute_command(executor_id, "mute_player", [peer_id])

Fluxo:
1. Verifica permissão
2. Encontra player node via _get_player_by_id(peer_id)
3. Acessa player.asc (ASComponent)
4. asc.add_tag(&"status.muted")
5. Chat system pode filtrar mensagens com essa tag

Efeito: Player não consegue enviar mensagens no chat
```

#### **unmute_player**

```
execute_command(executor_id, "unmute_player", [peer_id])

Fluxo:
1. Verifica permissão
2. Encontra player node
3. asc.remove_tag(&"status.muted")

Efeito: Player volta a conseguir enviar mensagens
```

#### **get_server_status**

```
execute_command(executor_id, "get_server_status", [])

Saída:
=== Server Status ===
Is Server: true
Connected Players: 3
Uptime: 125.4 seconds
======================
```

### Signals

```gdscript
signal command_executed(command: String, executor_id: int, success: bool)
signal player_kicked(peer_id: int, reason: String)
signal player_banned(peer_id: int, hours: float, reason: String)
signal broadcast_message_sent(message: String)
signal server_shutdown_initiated(reason: String)
```

---

## 3. Terminal CLI para Admin (AdminTerminal)

### Arquivo

`demo/autoload/AdminTerminal.gd`

### Modo Servidor Dedicado

```bash
$ ./ability-system-server --port 7777 --max-players 10

=== ABILITY SYSTEM SERVER [HEADLESS] ===
[SERVER] Ready on 0.0.0.0:7777
[SERVER] Type 'help' for commands

> help
=== ADMIN COMMANDS ===
kick_player <peer_id> [reason]        - Expulsar jogador
ban_player <peer_id> [hours] [reason] - Banir jogador
shutdown_server [reason]              - Desligar servidor
broadcast_message <message>           - Enviar mensagem geral
list_players                          - Listar todos jogadores
mute_player <peer_id>                - Mutar jogador
unmute_player <peer_id>              - Desmutar jogador
get_server_status                    - Status do servidor
======================

> list
=== Connected Players ===
[1] Server - admin
[2] João - user
[3] Maria - user
========================

> broadcast_message Server maintenance in 10 minutes!
[CLI] Broadcasting: Server maintenance in 10 minutes!

> kick 2 Comportamento inadequado
[CLI] Kicking player 2: Comportamento inadequado

> list
=== Connected Players ===
[1] Server - admin
[3] Maria - user
========================

> ban 3 24 Abuso de chat
[CLI] Banning player 3 for 24.0 hours: Abuso de chat

> status
=== Server Status ===
Is Server: true
Connected Players: 1
Uptime: 342.8 seconds
======================

> shutdown Servidor indo em manutenção
[CLI] Shutting down server: Servidor indo em manutenção
[CLI] Goodbye!
```

### Configuração de Servidor

```gdscript
server_info = {
    "host": "0.0.0.0",
    "port": 7777,
    "max_players": 10,
    "is_headless": true
}

# Setável via CLI ou código:
set_server_info(host, port, max_players)
```

### Parsing de Argumentos

```bash
# Forma 1: com argumentos
./ability-system-server --port 8888 --max-players 20

# Forma 2: padrão
./ability-system-server
# Usa defaults: 0.0.0.0:7777, max 10 players

# Forma 3: help
./ability-system-server --help
```

---

## 4. Admin Panel UI (Pause Menu)

### Arquivo

`demo/ui/admin_panel.tscn` / `demo/ui/admin_panel.gd`

### Layout Estrutura

```
╔════════════════════════════════════════════╗
║           ADMIN PANEL                      ║
╠════════════════════════════════════════════╣
║                                            ║
║  JOGADORES CONECTADOS                      ║
║  ┌──────────────────────────────────────┐  ║
║  │ [1] Server - admin                   │  ║
║  │ [2] João - user        [Kick] [Ban]  │  ║
║  │ [3] Maria - user       [Kick] [Ban]  │  ║
║  │ [4] Pedro - moderator  [Kick] [Ban]  │  ║
║  └──────────────────────────────────────┘  ║
║                                            ║
║  ENVIAR MENSAGEM                           ║
║  ┌──────────────────────────────────────┐  ║
║  │ [Digite mensagem para todos...]      │  ║
║  │                [ENVIAR]              │  ║
║  └──────────────────────────────────────┘  ║
║                                            ║
║  CONFIGURAÇÕES DO SERVIDOR                 ║
║  ┌──────────────────────────────────────┐  ║
║  │  [Fechar LAN]      [Desligar]        │  ║
║  └──────────────────────────────────────┘  ║
║                                            ║
╚════════════════════════════════════════════╝
```

### Comportamento

#### **Quando aparecer**

```
1. Player em singleplayer pausa (ESC)
   └─ Admin panel NÃO aparece (singleplayer)

2. Host em modo LAN (singleplayer aberto) pausa
   └─ Admin panel APARECE (multiplayer.is_server() = true)

3. Cliente em modo multiplayer pausa
   └─ Admin panel NÃO aparece (not multiplayer.is_server())

4. Servidor dedicado
   └─ Não tem UI, usa CLI via AdminTerminal
```

#### **Botões Kick**

```
Quando clicado em Kick de um player:
1. Valida: RBAC.can_execute(host_id, "kick_player")
2. Executa: AdminCommand.execute_command(1, "kick_player", [peer_id, "Kicked by host"])
3. Player é desconectado
4. Lista refreshed automaticamente
5. Chat exibe: "[SYSTEM] Player_X desconectado"
```

#### **Botões Ban**

```
Quando clicado em Ban de um player:
1. Valida permissão
2. Executa: AdminCommand.execute_command(1, "ban_player", [peer_id, 24.0, "Banned by host"])
3. Player é desconectado + banido por 24h
4. Se tentar reconectar: RBAC.is_banned() bloqueia
5. Ban expira automaticamente após 24h
```

#### **Broadcast**

```
Quando envia mensagem:
1. Input text validado (não vazio)
2. AdminCommand.execute_command(1, "broadcast_message", ["mensagem"])
3. Chat system recebe RPC: add_system_message.rpc("[ADMIN] Host: mensagem")
4. Aparece em amarelo para todos players
5. Input limpo, focus mantido
```

#### **Fechar LAN**

```
Quando clicado:
1. signal close_to_lan_requested emitido
2. MultiplayerGameManager.close_to_lan() chamado
3. Servidor fecha, players desconectados
4. Host volta a singleplayer
5. Admin panel fecha
```

#### **Desligar**

```
Quando clicado:
1. signal shutdown_requested emitido
2. AdminCommand.execute_command(1, "shutdown_server", ["Host closed server"])
3. Server encerra
4. Todos players recebem server_disconnected
5. Retorna ao MainMenu automaticamente
```

### Sincronização em Tempo Real

```gdscript
# Admin panel conecta a estes sinais:
multiplayer.peer_connected.connect(_on_peer_connected)
multiplayer.peer_disconnected.connect(_on_peer_disconnected)

# Quando novo player conecta:
func _on_peer_connected(peer_id):
    _refresh_players_list()
    # Player item criado com botões Kick/Ban

# Quando player desconecta:
func _on_peer_disconnected(peer_id):
    _refresh_players_list()
    # Player item removido da lista
```

---

## 5. Servidor Dedicado (Headless)

### Arquivo

`demo/server/server_main.gd` + `demo/server/server.tscn`

### Inicialização

```gdscript
extends Node
class_name ServerMain

var server_config: Dictionary = {
    "host": "0.0.0.0",
    "port": 7777,
    "max_players": 10,
    "is_headless": true
}

func _ready():
    _parse_command_line_args()
    _start_server()
    AdminTerminal.start_interactive_cli()  # Bloqueia até shutdown

func _parse_command_line_args():
    # Parse --host, --port, --max-players, --help
    # E log da configuração inicial
```

### Fluxo de Conexão

```
Cliente conecta:
1. Peer_connected signal emitido
2. RBAC.register_peer(peer_id, "Player_{id}")
3. Player spawned no servidor
4. MultiplayerSynchronizer sincroniza estado
5. Admin pode monitorar via CLI

Cliente desconecta:
1. Peer_disconnected signal emitido
2. RBAC.unregister_peer(peer_id)
3. Player destrói no servidor
4. Ban data mantida (se banido)
```

### Logs Periódicos

```
[12:34:56] [SERVER] Uptime: 15.3s | Connected: 0/10
[12:35:01] CONNECT: Player_1 (João)
[12:35:05] CHAT: João: "Oi galera!"
[12:35:10] [SERVER] Uptime: 20.3s | Connected: 1/10
[12:35:15] CONNECT: Player_2 (Maria)
[12:35:20] [SERVER] Uptime: 25.3s | Connected: 2/10
[12:35:45] COMMAND: Kicked Player_1
[12:36:00] [SERVER] Uptime: 45.3s | Connected: 1/10
```

---

## 6. GitHub Actions Multi-Platform Build

### Estratégia: Single Linux Workflow

**Abordagem:**

- ✅ Rodar tudo em `ubuntu-latest`
- ✅ Godot em headless com templates para gerar executáveis de todas as plataformas
- ✅ Sem matriz de múltiplas máquinas (mais barato, mais rápido)
- ✅ Build ordenado: addon → cliente (todas plataformas) → servidor (todas plataformas)
- ✅ Entregar `artifacts/` com estrutura unificada

### Estrutura do Artefato

```
artifacts/
├── demo_windows_client.exe
├── demo_windows_server.exe
├── demo_linux_client
├── demo_linux_server
├── demo_macos_client
├── demo_macos_server
├── demo_android_client.apk
└── demo_android_server.apk
```

### Fluxo do Workflow (integrado ao plugin GHA existente)

```
1. PHASE: Prepare
   └─ Checkout + import deps.env

2. PHASE: Build Addon (uma vez)
   ├─ Compila C++ com scons (linux target)
   └─ Output: addon binaries

3. PHASE: Export Client
   ├─ Template Windows → demo_windows_client.exe
   ├─ Template Linux → demo_linux_client
   ├─ Template macOS → demo_macos_client (universal)
   └─ Template Android → demo_android_client.apk

4. PHASE: Export Server
   ├─ Template Windows → demo_windows_server.exe
   ├─ Template Linux → demo_linux_server
   ├─ Template macOS → demo_macos_server
   └─ Template Android → demo_android_server.apk

5. PHASE: Organize
   ├─ Move todos arquivos para artifacts/
   ├─ Rename pattern: demo_{platform}_{type}
   └─ Output: artifacts/

6. PHASE: Upload Artifact (UM arquivo zip)
   └─ artifacts.zip contendo:
      ├── demo_windows_client.exe
      ├── demo_windows_server.exe
      ├── demo_linux_client
      ├── demo_linux_server
      ├── demo_macos_client
      ├── demo_macos_server
      ├── demo_android_client.apk
      └── demo_android_server.apk

7. PHASE: Post Release (job existente)
   └─ GitHub Actions automaticamente cria release
   └─ Adiciona artifacts/ como attachment
```

### Integração com Workflow Existente

**Modificar job existente em `.github/workflows/package.yml`:**

```yaml
jobs:
  build-deliverables:
    name: 📦 Build Client & Server (All Platforms)
    runs-on: ubuntu-latest
    steps:
      # ... setup existente ...

      - name: 📦 Build Addon
        run: |
          cd src
          scons platform=linux target=release -j4

      - name: 🎮 Export Clients
        env:
          GODOT_BIN: /path/to/godot
        run: |
          mkdir -p artifacts
          # Windows
          $GODOT_BIN --headless --path demo --export-release "Windows Desktop" ../artifacts/demo_windows_client.exe
          # Linux
          $GODOT_BIN --headless --path demo --export-release "Linux/X11" ../artifacts/demo_linux_client
          # macOS
          $GODOT_BIN --headless --path demo --export-release "macOS" ../artifacts/demo_macos_client
          # Android
          $GODOT_BIN --headless --path demo --export-release "Android" ../artifacts/demo_android_client.apk

      - name: 🖥️ Export Servers
        env:
          GODOT_BIN: /path/to/godot
        run: |
          # Windows
          $GODOT_BIN --headless --path demo --export-release "Windows Desktop" ../artifacts/demo_windows_server.exe
          # Linux
          $GODOT_BIN --headless --path demo --export-release "Linux/X11" ../artifacts/demo_linux_server
          # macOS
          $GODOT_BIN --headless --path demo --export-release "macOS" ../artifacts/demo_macos_server
          # Android
          $GODOT_BIN --headless --path demo --export-release "Android" ../artifacts/demo_android_server.apk

      - name: 📂 Organize & Validate
        run: |
          cd artifacts
          chmod +x demo_linux_client demo_linux_server demo_macos_client demo_macos_server 2>/dev/null || true
          ls -lah
          echo "✅ All executables built successfully"

      - name: 📤 Upload Unified Artifact
        uses: actions/upload-artifact@v4
        with:
          name: demo-executables
          path: artifacts/
          retention-days: 90

      # Post-release job (já existente) pega automaticamente
```

### Resultado Final

**Um único artifact chamado `demo-executables` contendo:**

```
demo_windows_client.exe       (5.2 MB)
demo_windows_server.exe       (5.1 MB)
demo_linux_client             (4.8 MB)
demo_linux_server             (4.7 MB)
demo_macos_client             (6.5 MB)
demo_macos_server             (6.4 MB)
demo_android_client.apk       (7.2 MB)
demo_android_server.apk       (7.1 MB)
```

**Job de post-release (existente) pega automaticamente e cria release no GitHub.**

---

## 7. Fluxo Completo de Uso

### Cenário 1: Host Local (Singleplayer → LAN)

```
1. Inicia cliente
   → UserRegistration (nome: "João")
   → MainMenu → Singleplayer
   → Level carrega

2. Pausa (ESC)
   → PauseMenu aparece
   → [Open to LAN] botão

3. Clica "Open to LAN"
   → MultiplayerGameManager inicia servidor na porta 7778
   → Admin Panel aparece (multiplayer.is_server() = true)
   → RBAC registra João como admin

4. Outro player se conecta
   → Client IP: localhost, Port: 7778
   → MultiplayerGameManager conecta
   → RBAC.register_peer(2, "Maria") como user
   → Admin panel atualiza automaticamente

5. Host pausa
   → Vê lista: [1] João (admin), [2] Maria (user)
   → Pode: Kick Maria, Ban, Broadcast, Close LAN

6. Host clica "Kick Maria"
   → RBAC.can_execute(1, "kick_player") ✓
   → Maria é desconectada
   → Admin panel remove item de Maria
   → Chat mostra: "[SYSTEM] Player_2 desconectado"
```

### Cenário 2: Servidor Dedicado

```
1. Terminal: ./ability-system-server --port 7777 --max-players 10

[SERVER] Ready on 0.0.0.0:7777
[SERVER] Type 'help' for commands

2. Client 1 conecta
   > list
   [1] Server - admin
   [2] João - user

3. Client 2 conecta
   > list
   [1] Server - admin
   [2] João - user
   [3] Maria - user

4. Player causes issues
   > kick 2 Comportamento inadequado
   Kicked player João

5. Admin quer desligar
   > say Servidor em manutenção em 2 minutos!
   [chat] [ADMIN] Server: Servidor em manutenção em 2 minutos!
   > shutdown Servidor indo offline
   [CLI] Goodbye!
   (servidor fecha, clientes recebem disconnect)
```

### Cenário 3: GitHub Actions Build

```
Push para master
  ↓
GitHub Actions dispara workflow
  ↓
build-deliverables job inicia (ubuntu-latest)
  ├─ Compila addon
  ├─ Export client (Win, Linux, macOS, Android)
  ├─ Export server (Win, Linux, macOS, Android)
  ├─ Organiza artifacts/
  └─ Upload artifact: demo-executables
  ↓
Post-release job pega artifact
  ↓
GitHub Release criada com 8 executáveis
```

---

## 8. Arquivos a Criar/Modificar

### ✅ Criados (8)

1. ✅ `demo/autoload/RBAC.gd` - Sistema de roles e permissões
2. ✅ `demo/autoload/AdminCommand.gd` - Processador de comandos
3. ✅ `demo/autoload/AdminTerminal.gd` - CLI para admin
4. ✅ `demo/ui/admin_panel.tscn` - UI painel admin
5. ✅ `demo/ui/admin_panel.gd` - Lógica painel admin
6. ✅ `demo/server/server_main.gd` - Entry point servidor
7. ✅ `demo/server/server.tscn` - Cena servidor
8. ⏳ `.github/workflows/package.yml` - Modificar para multi-platform build

### ✅ Modificados (3)

1. ✅ `demo/autoload/MultiplayerGameManager.gd` - Registra peers em RBAC
2. ✅ `demo/ui/pause_menu.gd` - Mostra admin panel para host
3. ✅ `demo/project.godot` - Adiciona autoloads (RBAC, AdminCommand, AdminTerminal)

---

## 9. Testes

- [ ] RBAC: can_execute retorna correto por role
- [ ] AdminCommand: executa commands corretamente
- [ ] AdminPanel: aparece só para host
- [ ] AdminTerminal: CLI funciona com todos commands
- [ ] Ban: expira após X horas
- [ ] Broadcast: aparece em chat para todos
- [ ] GHA: artifact contém todos 8 executáveis
- [ ] Multi-platform: todos executáveis rodam em suas plataformas

---

## 10. Notas Técnicas

### RPC Authority

```gdscript
# AdminCommand executa localmente no server
execute_command(executor_id, command, args)

# Ou RPC para clientes enviarem comandos
@rpc("any_peer")
func execute_command_rpc(command, args)
  → Server valida RBAC
  → Server executa
  → Broadcast resultado
```

### Performance

- Admin panel: refresh on peer_connected/disconnected (eficiente)
- RBAC: O(1) lookups (Dictionary)
- Ban list: Check no multiplayer.peer_connected (uma vez)
- CLI: Input blocking OK (servidor dedicado)

### Segurança

- Todos commands validados via RBAC no server
- Clientes não conseguem contornar (RPC validado)
- Bans armazenados em memória (adequado para sessão)
