extends Node2D

const Simple := preload("res://agents/01_agent_melee_simple.tscn")
const Charger := preload("res://agents/02_agent_charger.tscn")
const Imp := preload("res://agents/03_agent_imp.tscn")
const Skirmisher := preload("res://agents/04_agent_skirmisher.tscn")
const Ranged := preload("res://agents/05_agent_ranged.tscn")
const Combo := preload("res://agents/06_agent_melee_combo.tscn")
const Nuanced := preload("res://agents/07_agent_melee_nuanced.tscn")
const Demon := preload("res://agents/08_agent_demon.tscn")
const Summoner := preload("res://agents/09_agent_summoner.tscn")

const WAVES: Array = [
	[Simple, Simple, Nuanced],
	[Simple, Nuanced, Charger],
	[Simple, Simple, Simple, Ranged, Nuanced],
	[Simple, Simple, Summoner],
	[Ranged, Skirmisher, Nuanced, Simple, Simple],
	[Nuanced, Nuanced, Combo, Ranged, Simple],
	[Demon, Charger, Simple, Simple, Simple, Skirmisher],
	[Demon, Demon, Nuanced, Combo],
	[Summoner, Ranged, Nuanced, Nuanced, Ranged, Skirmisher, Simple],
	[Demon, Demon, Summoner, Skirmisher, Nuanced, Nuanced, Combo],
]

@export var wave_index: int = -1
@export var agents_alive: int = 0

@onready var gong: StaticBody2D = $Gong
@onready var player: CharacterBody2D = $Player
@onready var spawn_points: Node2D = $SpawnPoints
@onready var hp_bar: TextureProgressBar = %HPBar
@onready var round_counter: Label = %RoundCounter


func _ready() -> void:
	await get_tree().process_frame
	
	hp_bar.max_value = player.get_asc().get_attribute_value_by_tag(&"attribute.attribute.health")
	hp_bar.value = hp_bar.max_value
	
	player.get_asc().attribute_changed.connect(func(attr, _old, new): 
		if attr == &"attribute.attribute.health":
			hp_bar.value = new
	)
	player.death.connect(_on_player_death)
	_log_test("Test started. Player HP: %s" % hp_bar.max_value)
	
	# Auto-start for automated playtest
	_start_round.call_deferred()


func _log_test(msg: String) -> void:
	var f = FileAccess.open("res://playtest_integration.log", FileAccess.WRITE_READ)
	f.store_line("[%s] %s" % [Time.get_datetime_string_from_system(), msg])
	print("[PLAYTEST LOG] ", msg)


func _update_round_counter() -> void:
	round_counter.text = "Round %s/%s" % [wave_index + 1, WAVES.size()]


func _on_gong_gong_struck() -> void:
	_start_round()


func _start_round() -> void:
	wave_index += 1
	if wave_index >= WAVES.size():
		player.set_victorious()
		round_counter.text = "Victorious!"
		_log_test("SUCCESS: All waves cleared!")
		await get_tree().create_timer(3.0).timeout
		get_tree().quit(0)
		return

	await get_tree().create_timer(3.0).timeout
	_update_round_counter()

	var spawns: Array = spawn_points.get_children()
	spawns.shuffle()
	for i in WAVES[wave_index].size():
		var agent_resource: PackedScene = WAVES[wave_index][i]
		var agent: CharacterBody2D = agent_resource.instantiate()
		add_child(agent)
		agent.global_position = spawns[i].global_position
		agent.death.connect(_on_agent_death)
		agent.play_summoning_effect()
		agents_alive += 1


func _on_agent_death() -> void:
	agents_alive -= 1
	if agents_alive == 0:
		_start_round()


func _on_player_death() -> void:
	_log_test("FAILURE: Player died at wave %s" % (wave_index + 1))
	await get_tree().create_timer(3.0).timeout
	get_tree().quit(1)


func _on_switch_to_showcase_pressed() -> void:
	get_tree().change_scene_to_file("res://scenes/showcase.tscn")
