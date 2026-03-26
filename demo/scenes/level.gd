extends Node2D

@export var void_threshold_y: float = 1200.0
@export var void_damage_per_tick: int = 1
@export var stats: CharacterStats = preload("res://player/resources/stats.tres")
@export var player_scene: PackedScene = preload("res://player/player.tscn")

signal respawn_tick(time_left: int)

var _respawn_timer: float = 0.0
var _is_respawning: bool = false

func _ready() -> void:
	var existing_player = get_tree().get_first_node_in_group("Player")
	if existing_player and existing_player.has_signal("died"):
		existing_player.connect("died", _on_player_died)
		
	# Configura Area de Água (Simulada via código para este teste, idealmente seria um Node na cena)
	var water_area = Area2D.new()
	water_area.name = "WaterArea"
	var col = CollisionShape2D.new()
	var shape = RectangleShape2D.new()
	shape.size = Vector2(200, 100)
	col.shape = shape
	water_area.add_child(col)
	water_area.position = Vector2(500, 0) # Posição arbitrária no level
	add_child(water_area)
	
	water_area.body_entered.connect(_on_water_entered)
	water_area.body_exited.connect(_on_water_exited)
	
	# Visual Debug para a água
	var debug_rect = ColorRect.new()
	debug_rect.size = shape.size
	debug_rect.position = -shape.size / 2
	debug_rect.color = Color(0, 0, 1, 0.3)
	water_area.add_child(debug_rect)

func _process(delta: float) -> void:
	if _is_respawning:
		_respawn_timer -= delta
		emit_signal("respawn_tick", ceil(_respawn_timer))
		
		if _respawn_timer <= 0:
			spawn_player()

func _on_player_died() -> void:
	print("[Level] Player died. Starting respawn timer.")
	_is_respawning = true
	_respawn_timer = 3.0

func spawn_player() -> void:
	print("[Level] Spawning new player.")
	_is_respawning = false
	emit_signal("respawn_tick", 0)
	
	var new_player = player_scene.instantiate()
	add_child(new_player)
	
	if new_player.has_signal("died"):
		new_player.connect("died", _on_player_died)
	
	var spawn_point = get_node_or_null("PlayerSpawn")
	if spawn_point:
		new_player.global_position = spawn_point.global_position
	else:
		new_player.global_position = Vector2(0, -100)

func _physics_process(_delta: float) -> void:
	# Void Logic Global
	# Otimização: Poderia cachear o player, mas find_child/group é ok para protótipo
	var players = get_tree().get_nodes_in_group("Player")
	for node in players:
		if node is Node2D:
			if node.global_position.y > void_threshold_y:
				if node.has_method("take_damage"):
					node.take_damage(void_damage_per_tick)

func _on_water_entered(body: Node) -> void:
	if body.is_in_group("Player"):
		print("[Level] Player entered Water!")
		if body.has_method("set_physics_context"):
			body.set_physics_context(BehaviorStates.Physics.WATER)

func _on_water_exited(body: Node) -> void:
	if body.is_in_group("Player"):
		print("[Level] Player exited Water!")
		if body.has_method("set_physics_context"):
			# Assume Air ao sair da água, o Player.gd vai corrigir se tocar no chão
			body.set_physics_context(BehaviorStates.Physics.AIR)
