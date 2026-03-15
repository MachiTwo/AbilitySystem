#*
#* showcase.gd
#* =============================================================================
#* Copyright 2024 Serhii Snitsaruk
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*

extends Node2D

@onready var behavior_tree_view: BehaviorTreeView = %BehaviorTreeView
@onready var camera: Camera2D = $Camera2D
@onready var agent_selection: MenuButton = %AgentSelection
@onready var previous: Button = %Previous
@onready var next: Button = %Next
@onready var minimize_description: Button = %MinimizeDescription
@onready var description: RichTextLabel = %Description
@onready var begin_tutorial: Button = %BeginTutorial
@onready var navigation_hint: Label = %NavigationHint
@onready var scene_title: Label = %SceneTitle
@onready var code_popup = %CodePopup
@onready var code_edit = %CodeEdit

var bt_player: BTPlayer
var selected_tree_index: int = -1
var agent_files: Array[String]
var agents_dir: String
var is_tutorial: bool = false


@export var auto_cycle: bool = false
var _cycle_timer: float = 0.0
var _agents_tested: int = 0

func _ready() -> void:
	if OS.get_cmdline_args().has("--playtest-cycle") or OS.has_feature("playtest"):
		auto_cycle = true
	
	print("[SHOWCASE] Initializing. Auto cycle: ", auto_cycle)
	code_popup.hide()

	agent_selection.get_popup().id_pressed.connect(_on_agent_selection_id_pressed)
	previous.pressed.connect(func(): _on_agent_selection_id_pressed(selected_tree_index - 1))
	next.pressed.connect(func(): _on_agent_selection_id_pressed(selected_tree_index + 1))

	_initialize()
	
	if auto_cycle:
		_cycle_timer = 15.0 # Ensure the first agent gets its time
		print("[PLAYTEST] Starting auto-cycle test.")


func _physics_process(delta: float) -> void:
	if auto_cycle:
		if agent_files.is_empty():
			return
			
		_cycle_timer -= delta
		if _cycle_timer <= 0:
			_agents_tested += 1
			print("[PLAYTEST] Progress: %d/%d agents finished." % [_agents_tested, agent_files.size()])
			
			if _agents_tested >= agent_files.size():
				print("[SHOWCASE PLAYTEST] SUCCESS: All agents tested. Quitting.")
				get_tree().quit(0)
				return
			
			var next_id = _agents_tested # Cycle through in order
			_cycle_timer = 15.0
			_on_agent_selection_id_pressed(next_id)
		
		# Update UI with countdown
		scene_title.text = "Showcase (Playtest: %d/%d) [%ds]" % [_agents_tested + 1, agent_files.size(), int(_cycle_timer)]

	if is_instance_valid(bt_player):
		var agent = bt_player.get_parent()
		if is_instance_valid(agent) and agent is CharacterBody2D:
			camera.global_position = agent.global_position

		var inst: BTInstance = bt_player.get_bt_instance()
		if inst:
			var bt_data: BehaviorTreeData = BehaviorTreeData.create_from_bt_instance(inst)
			behavior_tree_view.update_tree(bt_data)


func _initialize() -> void:
	if is_tutorial:
		_populate_agent_files("res://agents/tutorial/")
		begin_tutorial.text = "End Tutorial"
		navigation_hint.text = "Navigate ➤"
		scene_title.text = "Tutorial"
		_on_agent_selection_id_pressed(0)
	else:
		_populate_agent_files("res://agents/")
		begin_tutorial.text = "Begin Tutorial"
		navigation_hint.text = "Select Agent ➤"
		scene_title.text = "Showcase"
		_on_agent_selection_id_pressed(0)


func _attach_camera(agent: CharacterBody2D) -> void:
	# Keep the camera attached to the showcase node, just update its global position.
	camera.global_position = agent.global_position + Vector2(400.0, 0.0)


func _populate_agent_files(p_path: String = "res://agents/") -> void:
	var popup: PopupMenu = agent_selection.get_popup()
	popup.clear()
	popup.reset_size()
	agent_files.clear()
	agents_dir = p_path

	var dir := DirAccess.open(p_path)
	if dir:
		dir.list_dir_begin()
		var file_name: String = dir.get_next()
		while file_name != "":
			if dir.current_is_dir() or file_name.begins_with("agent_base") or not file_name.ends_with(".tscn"):
				file_name = dir.get_next()
				continue
			agent_files.append(file_name.get_file().trim_suffix(".remap"))
			file_name = dir.get_next()
	dir.list_dir_end()
	print("[SHOWCASE] Found %d agent files in %s" % [agent_files.size(), p_path])

	agent_files.sort()
	for i in agent_files.size():
		popup.add_item(agent_files[i], i)


func _load_agent(file_name: String) -> void:
	var agent_res := load(file_name) as PackedScene
	if agent_res == null:
		push_error("Failed to load agent: " + file_name)
		return

	for child in get_children():
		if child is CharacterBody2D and child.name != "Dummy":
			child.queue_free()

	# Ensuring previous agents are cleared before adding a new one.
	await get_tree().process_frame

	var agent: CharacterBody2D = agent_res.instantiate()
	add_child(agent)
	agent.position = Vector2(500, 400) # Initial position in the arena

	bt_player = agent.find_child("BTPlayer")
	if bt_player:
		description.text = _parse_description(bt_player.behavior_tree.description)

	_attach_camera(agent)


func _parse_description(p_desc: String) -> String:
	return p_desc \
			.replace("[SUCCESS]", "[color=PaleGreen]SUCCESS[/color]") \
			.replace("[FAILURE]", "[color=IndianRed]FAILURE[/color]") \
			.replace("[RUNNING]", "[color=orange]RUNNING[/color]") \
			.replace("[comp]", "[color=CornflowerBlue][b]") \
			.replace("[/comp]", "[/b][/color]") \
			.replace("[act]", "[color=white][b]") \
			.replace("[/act]", "[/b][/color]") \
			.replace("[dec]", "[color=MediumOrchid][b]") \
			.replace("[/dec]", "[/b][/color]") \
			.replace("[con]", "[color=orange][b]") \
			.replace("[/con]", "[/b][/color]")


func _on_agent_selection_id_pressed(id: int) -> void:
	if id < 0 or id >= agent_files.size():
		return
	selected_tree_index = id
	await _load_agent(agents_dir.path_join(agent_files[id]))

	if is_instance_valid(bt_player):
		agent_selection.text = bt_player.behavior_tree.resource_path.get_file()
		if agent_selection.text.to_lower() != agent_selection.text:
			# Treat filename as a title
			agent_selection.text = agent_selection.text.trim_suffix(".tres")

	previous.disabled = id == 0
	next.disabled = id == (agent_files.size() - 1)


func _on_switch_to_game_pressed() -> void:
	get_tree().change_scene_to_file("res://scenes/game.tscn")


func _on_minimize_description_button_down() -> void:
	description.visible = not description.visible
	minimize_description.text = "-" if description.visible else "+"


func _on_tutorial_pressed() -> void:
	is_tutorial = not is_tutorial
	_initialize()


func _on_behavior_tree_view_task_selected(_type_name: String, p_script_path: String) -> void:
	if not p_script_path.is_empty():
		var sc: Script = load(p_script_path)
		code_edit.set_source_code(sc.source_code)
		code_popup.popup.call_deferred()
