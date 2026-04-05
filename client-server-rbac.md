# Plano: Arquitetura Client/Server com RBAC e Plugin GHA

## Context

O projeto precisa evoluir para suportar:

1. **Servidor Dedicado** - headless, sem GUI, rodando via terminal
2. **Cliente Multiplayer** - demo compilada com capacidades de admin
3. **Sistema RBAC** - controle de acesso baseado em roles
4. **Plugin GHA** - que prepare addon, cliente e servidor para deployment

Atualmente temos:

- Demo single/multiplayer menu-driven
- MultiplayerGameManager com ENet
- GameData para estado global
- Chat system (MVP)

## Objetivo

Criar uma arquitetura completa onde:

- **Servidor Dedicado**: Modo headless com CLI admin, exibe IP:porta, logs estruturados, comandos de admin
- **Cliente LAN**: Host (player que abre para LAN) é admin; pode executar comandos via UI
- **Servidor Dedicado**: Servidor é admin (não um player do jogo)
- **Plugin GHA**: Automatiza build do addon, cliente e servidor

## Arquitetura Proposta

### 1. Sistema RBAC (Role-Based Access Control)

**Arquivo**: `demo/autoload/RBAC.gd`

```
RBAC (Singleton)
├── Roles
│   ├── admin: ["kick_player", "ban_player", "shutdown_server", "whitelist_add", "broadcast_message"]
│   └── user: ["chat", "move", "attack"]
├── Players
│   ├── {network_id}: {name, role, permissions}
│   └── server: {name="Server", role="admin"}
├── Methods
│   ├── set_player_role(peer_id, role)
│   ├── can_execute(peer_id, action)
│   ├── execute_command(peer_id, command)
│   └── get_player_role(peer_id)
```

**Lógica**:

- Servidor dedicado: peer_id=1, role=admin
- Client host (singleplayer → LAN): peer_id=1, role=admin
- Outros clientes: peer_id>1, role=user
- Permissões checadas antes de executar ações

### 2. Terminal CLI para Admin

**Arquivo**: `demo/autoload/AdminTerminal.gd`

**Modo Servidor Dedicado**:

```
[SERVER] Ready on 127.0.0.1:7777
> help
  kick <id>             - Expulsar jogador
  ban <id> [hours]      - Banir jogador
  list                  - Listar jogadores conectados
  say <message>         - Broadcast de mensagem
  shutdown              - Desligar servidor

> kick 2
Kicked player Player_2

> list
  ID  Name            Connected  Role
  1   Server          ✓          admin
  2   João            ✓          user
  3   Maria           ✓          user

> say Server maintenance in 10 minutes
```

**Modo Client LAN Host**:

```
[ADMIN PANEL] - Host Admin Console
  Kick Player
  Ban Player
  Broadcast Message
  Close to LAN
```

### 3. Servidor Dedicado (Headless)

**Arquivo**: `demo/server/server_main.gd`

```gdscript
extends SceneTree

func _init():
    # Headless mode - sem rendering
    if not OS.get_environment("HEADLESS"):
        OS.set_environment("HEADLESS", "1")

    # Setup
    MultiplayerGameManager.start_game("multiplayer_server", "0.0.0.0", 7777)

    # CLI
    AdminTerminal.start_interactive_cli()

func _process(delta):
    # Logs estruturados
    if frame % 300 == 0:  # A cada ~5 segundos
        _log_server_status()

func _log_server_status():
    print("[SERVER] Connected: %d | Time: %s" % [
        RBAC.get_connected_count(),
        Time.get_ticks_msec()
    ])
```

**Entry Point**: `demo/server/server.tscn` (simples SceneTree)

### 4. Cliente com Admin Panel

**Arquivo**: `demo/ui/admin_panel.tscn` / `demo/ui/admin_panel.gd`

Integrado ao pause menu:

```
[PAUSED]
├─ Resume
├─ [Admin Panel] (visível se host)
│  ├─ Jogadores
│  │  ├─ João → [Kick] [Ban]
│  │  └─ Maria → [Kick] [Ban]
│  ├─ Broadcast
│  │  └─ [Text Input] [Send]
│  └─ Server Settings
│     ├─ Close to LAN
│     └─ Shutdown
├─ Settings
└─ Quit to Menu
```

### 5. Plugin GHA Package

**Arquivo**: `.github/workflows/build.yml` (modificado)

Preparar:

1. **Addon** - Ability System compilado
2. **Cliente** - Demo compilada para:
   - Windows (64-bit)
   - Linux (64-bit)
   - macOS (Universal)
3. **Servidor** - Demo serverless compilada para:
   - Windows (64-bit)
   - Linux (64-bit)
   - macOS (Intel)

Estrutura de release:

```
ability-system-v1.0.0/
├── addon/
│   └── ability_system/  (plugin compilado)
├── client/
│   ├── windows/         (ability-system-client.exe)
│   ├── linux/           (ability-system-client)
│   └── macos/           (ability-system-client)
├── server/
│   ├── windows/         (ability-system-server.exe)
│   ├── linux/           (ability-system-server)
│   └── macos/           (ability-system-server)
├── README.md
├── QUICKSTART.md
└── LICENSE
```

### 6. Fluxo Completo

#### Usuário Local (Singleplayer → LAN)

```
1. Inicia Cliente (Demo compilada)
   → UserRegistration
   → MainMenu
   → Seleciona "Singleplayer"
   → Level carrega

2. Pausa + "Open to LAN"
   → MultiplayerGameManager muda para server mode
   → Exibe porta (ex: 7778) e ID no chat
   → Admin panel aparece

3. Outro jogador se conecta
   → MainMenu → Multiplayer
   → IP: localhost, Port: 7778
   → Conecta
   → Pode chat, jogar
   → Host (admin) pode kickar
```

#### Servidor Dedicado

```
$ ./ability-system-server --port 7778 --max-players 10

[SERVER] Starting...
[SERVER] Ready on 0.0.0.0:7778
[SERVER] Admin console ready

> help
> list
  ID  Name  Connected
  1   -     ✓ (waiting for players)

[12:34:56] CONNECT: Player_1 (João)
[12:35:00] CHAT: João: "Oi galera"
[12:35:15] COMMAND: Kicked Player_1
```

## Arquivos a Criar/Modificar

### CRIAR (9)

1. `demo/autoload/RBAC.gd` - Sistema de controle de acesso
2. `demo/autoload/AdminTerminal.gd` - CLI para admin commands
3. `demo/autoload/AdminCommand.gd` - Processador de comandos
4. `demo/ui/admin_panel.tscn` - UI admin panel
5. `demo/ui/admin_panel.gd` - Script admin panel
6. `demo/server/server_main.gd` - Entry point servidor
7. `demo/server/server.tscn` - Cena servidor
8. `demo/server/headless_config.gd` - Configuração headless
9. `.github/workflows/build.yml` - GitHub Actions build script

### MODIFICAR (4)

1. `demo/autoload/MultiplayerGameManager.gd`
   - Adicionar suporte a modo headless
   - Integrar RBAC na spawning de players
   - RPC para comandos de admin

2. `demo/ui/pause_menu.gd`
   - Mostrar admin panel se host
   - Chamar RBAC para validação

3. `demo/project.godot`
   - Adicionar RBAC e AdminTerminal como autoloads
   - Configurar cena de servidor

4. `demo/player/player.gd`
   - Adicionar peer_id / network_id público
   - RPC para sync de role

## Detalhes de Implementação

### RBAC.gd

```gdscript
extends Node

class_name RBAC

# Definições de roles
const ROLES = {
    "admin": ["kick_player", "ban_player", "shutdown_server", "whitelist_add", "broadcast_message"],
    "user": ["chat", "move", "attack"]
}

var peer_roles: Dictionary = {}  # {peer_id: "role"}
var peer_data: Dictionary = {}   # {peer_id: {name, role}}

func _ready():
    # Server automatically has admin role
    if multiplayer.is_server():
        peer_roles[1] = "admin"
        peer_data[1] = {name: "Server", role: "admin", peer_id: 1}

func set_player_role(peer_id: int, role: String) -> bool:
    if role not in ROLES:
        return false
    peer_roles[peer_id] = role
    _broadcast_role_change(peer_id, role)
    return true

func can_execute(peer_id: int, action: String) -> bool:
    var role = peer_roles.get(peer_id, "user")
    return action in ROLES.get(role, [])

@rpc("any_peer")
func execute_command(peer_id: int, command: String, args: Array) -> void:
    if not can_execute(peer_id, command):
        print("[RBAC] Denied: %s tried %s" % [peer_id, command])
        return
    AdminCommand.execute(command, args, peer_id)
```

### AdminTerminal.gd

```gdscript
extends Node

var is_running: bool = false

func start_interactive_cli() -> void:
    is_running = true
    while is_running:
        var input = _read_line()
        if input.is_empty():
            continue
        _process_command(input)

func _read_line() -> String:
    # Lê do stdin / console
    # Retorna comando digitado

func _process_command(cmd: String) -> void:
    var parts = cmd.split(" ")
    match parts[0]:
        "help": _show_help()
        "list": _list_players()
        "kick": AdminCommand.execute("kick_player", [int(parts[1])], 1)
        "shutdown": AdminCommand.execute("shutdown_server", [], 1)
```

## Timeline de Implementação

### Fase 1: RBAC (1-2 dias)

- RBAC.gd com sistema de roles
- AdminCommand.gd com processador básico
- Integrar com MultiplayerGameManager
- RPC para sincronização

### Fase 2: Terminal CLI (1 dia)

- AdminTerminal.gd interativa
- Comandos: list, kick, shutdown, say
- Parsing de input
- Logs estruturados

### Fase 3: Admin Panel UI (1 dia)

- admin_panel.tscn com layout
- admin_panel.gd com callbacks
- Integração ao pause menu
- Visualização de jogadores

### Fase 4: Servidor Dedicado (1 dia)

- server_main.gd entry point
- server.tscn scene
- headless_config.gd
- Testes de conectividade

### Fase 5: Plugin GHA (1-2 dias)

- Configurar build.yml
- Build multi-plataforma
- Asset packaging
- Testes de release

## Compatibilidade

- ✅ Mantém compatibilidade com menu-driven client
- ✅ Modo CI/CD (MP_GHA_PLAYER_ID) ainda funciona
- ✅ Singleplayer não afetado
- ✅ Chat system reutilizado

## Testes

1. **RBAC**: Verificar permissions
2. **CLI**: Testar comandos (kick, list, shutdown)
3. **Multi-cliente**: 2-3 clientes + 1 servidor
4. **Admin UI**: Funcionalidade no pause menu
5. **Build**: Compilar para 3 plataformas
