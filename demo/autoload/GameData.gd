extends Node
## Global game data - player identity and game state
## Persists across scene changes

var player_name: String = "Player"
var player_id: int = -1
var game_mode: String = ""  # "singleplayer", "multiplayer_server", "multiplayer_client"
var server_host: String = "127.0.0.1"
var server_port: int = 7777

func get_player_identifier() -> String:
	return "%s (ID: %d)" % [player_name, player_id]

func reset() -> void:
	"""Reset to defaults"""
	player_name = "Player"
	player_id = -1
	game_mode = ""
	server_host = "127.0.0.1"
	server_port = 7777
