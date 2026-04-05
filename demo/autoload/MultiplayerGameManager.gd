extends Node
## Central multiplayer manager for Ability System demo
## Handles: ENet setup, player spawning, synchronization, snapshots

const TICK_RATE = 60
const SNAPSHOT_INTERVAL = 10

var is_server: bool = false
var is_client: bool = false
var current_tick: int = 0
var connected_players: Dictionary = {}

signal player_spawned(player_id: int, player_node: Node)
signal player_despawned(player_id: int)
signal tick_processed(tick: int)

@export var server_port: int = 7777
@export var server_host: String = "127.0.0.1"
@export var max_players: int = 11  # 1 server + 10 clients

var player_container: Node
var player_scene: PackedScene = preload("res://player/player.tscn")
var game_mode_configured: bool = false
var current_mode: String = ""  # "singleplayer", "multiplayer_server", "multiplayer_client"

func _ready() -> void:
	print("[MP-MANAGER] Initializing multiplayer manager...")

	# Create player container
	player_container = Node.new()
	player_container.name = "Players"
	add_child(player_container)

	# Check if running in CI/CD mode (GitHub Actions)
	var player_id = _get_player_id()
	if player_id >= 0:
		print("[MP-MANAGER] CI/CD mode detected, using automated setup")
		_configure_from_environment(player_id)
		game_mode_configured = true
	else:
		print("[MP-MANAGER] Waiting for menu to configure game mode...")

func start_game(mode: String, host: String = "127.0.0.1", port: int = 7777) -> void:
	"""Start game with specified configuration (called from menu)"""
	print("[MP-MANAGER] Starting game with mode: %s" % mode)

	game_mode_configured = true
	current_mode = mode
	server_host = host
	server_port = port

	match mode:
		"singleplayer":
			print("[MP-MANAGER] Singleplayer mode")
			is_server = false
			is_client = false
			# Load level and spawn local player
			var error = await get_tree().change_scene_to_file("res://scenes/level.tscn")
			if error == OK:
				# Spawn local player after level loads
				await get_tree().process_frame
				var spawn_pos = _get_spawn_position(1)
				_spawn_player(1, spawn_pos)
		"multiplayer_server":
			print("[MP-MANAGER] Multiplayer SERVER mode")
			await _setup_server()
			await get_tree().change_scene_to_file("res://scenes/level.tscn")
		"multiplayer_client":
			print("[MP-MANAGER] Multiplayer CLIENT mode")
			await _setup_client(1)
			await get_tree().change_scene_to_file("res://scenes/level.tscn")
		_:
			print("[MP-MANAGER] ERROR: Unknown mode: %s" % mode)

func _configure_from_environment(player_id: int) -> void:
	"""Configure from CI/CD environment variables"""
	if player_id == 0:
		current_mode = "multiplayer_server"
		_setup_server()
	elif player_id > 0:
		current_mode = "multiplayer_client"
		_setup_client(player_id)

func _setup_server() -> void:
	print("[MP-MANAGER] Setting up SERVER mode...")
	is_server = true

	var peer = ENetMultiplayerPeer.new()
	var error = peer.create_server(server_port, max_players)

	if error != OK:
		print("[MP-MANAGER] ERROR: Failed to create server: %d" % error)
		get_tree().quit(1)
		return

	multiplayer.multiplayer_peer = peer
	multiplayer.peer_connected.connect(_on_peer_connected)
	multiplayer.peer_disconnected.connect(_on_peer_disconnected)

	print("[MP-MANAGER] Server listening on %s:%d" % [server_host, server_port])

func _setup_client(player_id: int) -> void:
	print("[MP-MANAGER] Setting up CLIENT mode (player %d)..." % player_id)
	is_client = true

	# Retry logic for connection
	var max_retries = 5
	for attempt in range(max_retries):
		print("[MP-MANAGER] Connecting to %s:%d (attempt %d/%d)..." % [server_host, server_port, attempt + 1, max_retries])

		var peer = ENetMultiplayerPeer.new()
		var error = peer.create_client(server_host, server_port)

		if error != OK:
			print("[MP-MANAGER] Connection attempt %d failed: %d" % [attempt + 1, error])
			if attempt < max_retries - 1:
				await get_tree().create_timer(0.5).timeout
			continue

		multiplayer.multiplayer_peer = peer
		multiplayer.connected_to_server.connect(_on_connected_to_server)
		multiplayer.server_disconnected.connect(_on_server_disconnected)

		# Wait for connection
		var timeout = 30.0
		var elapsed = 0.0
		while not multiplayer.is_connected_to_server() and elapsed < timeout:
			await get_tree().process_frame
			elapsed += get_process_delta_time()

		if multiplayer.is_connected_to_server():
			print("[MP-MANAGER] Successfully connected to server!")
			return

	print("[MP-MANAGER] ERROR: Failed to connect after %d attempts" % max_retries)
	get_tree().quit(1)

func _on_peer_connected(peer_id: int) -> void:
	print("[MP-MANAGER] Peer %d connected" % peer_id)
	connected_players[peer_id] = true

	# Register peer in RBAC
	if is_server and RBAC:
		var player_name = "Player_%d" % peer_id
		var game_data = get_node_or_null("/root/GameData")
		if game_data and game_data.player_name != "Player":
			player_name = game_data.player_name
		RBAC.register_peer(peer_id, player_name)

	# Server spawns a player for each connected client
	if is_server and peer_id > 0:
		var spawn_pos = _get_spawn_position(peer_id)
		_spawn_player_for_client.rpc_id(peer_id, peer_id, spawn_pos)

func _on_peer_disconnected(peer_id: int) -> void:
	print("[MP-MANAGER] Peer %d disconnected" % peer_id)
	connected_players.erase(peer_id)

	# Unregister peer from RBAC
	if is_server and RBAC:
		RBAC.unregister_peer(peer_id)

	player_despawned.emit(peer_id)

func _on_connected_to_server() -> void:
	print("[MP-MANAGER] Connected to server!")

func _on_server_disconnected() -> void:
	print("[MP-MANAGER] ERROR: Server disconnected!")
	get_tree().quit(1)

@rpc("call_remote")
func _spawn_player_for_client(player_id: int, position: Vector2) -> void:
	"""Server RPC: Tell client to spawn their player"""
	if not multiplayer.is_server():
		_spawn_player(player_id, position)

func _spawn_player(player_id: int, position: Vector2) -> void:
	"""Spawn a player node locally"""
	var player = player_scene.instantiate()
	player.name = "Player_%d" % player_id
	player.position = position
	player.network_id = player_id
	player_container.add_child(player)

	# Setup network sync if exists
	if player.has_method("setup_network_sync"):
		player.setup_network_sync(player_id)

	print("[MP-MANAGER] Spawned player %d at %s" % [player_id, position])
	player_spawned.emit(player_id, player)

func _get_player_id() -> int:
	var env = OS.get_environment("MP_GHA_PLAYER_ID")
	return int(env) if env else -1

func _get_spawn_position(player_id: int) -> Vector2:
	"""Get spawn position for player (spread across level)"""
	var positions = [
		Vector2(100, 300),   # Player 1
		Vector2(500, 300),   # Player 2
		Vector2(900, 300),   # Player 3
		Vector2(200, 500),   # Player 4
		Vector2(600, 500),   # Player 5
		Vector2(200, 100),   # Player 6
		Vector2(600, 100),   # Player 7
		Vector2(400, 400),   # Player 8
		Vector2(800, 200),   # Player 9
		Vector2(300, 600),   # Player 10
	]

	var idx = player_id - 1  # 1-indexed to 0-indexed
	if idx >= 0 and idx < positions.size():
		return positions[idx]
	return Vector2(400, 300)  # Fallback

func _process(_delta: float) -> void:
	"""Main game loop - tick processing"""
	if is_server or is_client:
		_process_tick()

func _process_tick() -> void:
	"""Process one game tick"""
	current_tick += 1
	tick_processed.emit(current_tick)

	# Every SNAPSHOT_INTERVAL ticks, capture snapshot (server only)
	if is_server and current_tick % SNAPSHOT_INTERVAL == 0:
		_capture_and_broadcast_snapshot()

func _capture_and_broadcast_snapshot() -> void:
	"""Capture game state and broadcast to all clients"""
	if not is_server:
		return

	# TODO: Implement snapshot capture and distribution
	# This would serialize all player states and send to clients for correction
	pass

