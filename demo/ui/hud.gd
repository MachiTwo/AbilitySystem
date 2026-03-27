extends Control

var asc: ASComponent
var player_ref: Player
var time_elapsed: float = 0.0

@onready var label_state: Label = $Panel/MarginContainer/VBoxContainer/LabelState
@onready var label_context: Label = $Panel/MarginContainer/VBoxContainer/LabelContext

@onready var label_item: Label = $Panel2/MarginContainer/VBoxContainer/LabelItem
@onready var label_compose: RichTextLabel = $Panel2/MarginContainer/VBoxContainer/LabelCompose

var _current_state: StringName = &""

const TAG_COLORS := {
	&"state.idle": Color(0.785, 0.786, 0.761),
	&"state.walk": Color(0.5, 0.5, 0.5),
	&"state.run": Color(0.25, 0.5, 1.0),
	&"state.dash": Color(0.4, 0.4, 0.8),
	&"state.jump": Color(0.5, 1.0, 0.5),
	&"state.fall": Color(1.0, 1.0, 0.0),
	&"state.hurt": Color(1.0, 0.5, 0.0),
	&"state.dead": Color(1.0, 0.0, 0.0),
	&"state.hyperdash": Color(0.0, 0.8, 0.8),
	&"attack.combo1": Color(1.0, 0.0, 0.0),
	&"attack.combo2": Color(1.0, 0.27, 0.0),
	&"attack.combo3": Color(1.0, 0.0, 0.27),
	&"attack.heavy": Color(0.8, 0.0, 0.0),
	&"attack.dash_attack": Color(0.53, 0.0, 0.8),
}

func _on_tag_changed(tag: StringName, is_present: bool) -> void:
	if is_present and tag in TAG_COLORS:
		_current_state = tag
	_update_labels()

func _ready() -> void:
	var level = get_parent()
	_find_asc()

func _process(delta: float) -> void:
	time_elapsed += delta
	if not is_instance_valid(asc) or not is_instance_valid(player_ref):
		_find_asc()

	_update_bars()
	_update_timer()
	_update_status_list()

func _update_status_list() -> void:
	if not is_instance_valid(asc) or not label_compose: return
	
	var final_text = "[color=#aaaaaa]Active Tags:[/color]\n"
	var tags = asc.get_tags()
	
	if tags.is_empty():
		final_text += "  (none)\n"
	else:
		for tag in tags:
			var color_hex = "ffffff"
			if tag in TAG_COLORS:
				color_hex = TAG_COLORS[tag].to_html(false)
			final_text += "[color=#%s]• %s[/color]\n" % [color_hex, String(tag)]
	
	final_text += "\n[color=#aaaaaa]Unlocked Abilities:[/color]\n"
	var abilities = asc.get_unlocked_abilities()
	
	if abilities.is_empty():
		final_text += "  (none)"
	else:
		for spec in abilities:
			if not spec: continue
			var ability = spec.get_ability()
			if not ability: continue
			
			var a_tag = ability.get_ability_tag()
			var a_name = String(a_tag).replace("ability.", "").capitalize()
			
			var status_color = "aaaaaa"
			if spec.is_active():
				status_color = "55ff55"
			elif spec.get_cooldown_remaining() > 0:
				status_color = "ff5555"
				a_name += " (%.1fs)" % spec.get_cooldown_remaining()
			
			final_text += "[color=#%s]✔ %s[/color]\n" % [status_color, a_name]
	
	label_compose.text = final_text

func _find_asc() -> void:
	var player = get_tree().get_first_node_in_group("Player")
	if player and is_instance_valid(player):
		player_ref = player as Player
		asc = player.get_node_or_null("ASComponent")
		if asc:
			if not asc.is_connected("tag_changed", _on_tag_changed):
				asc.connect("tag_changed", _on_tag_changed)

func _update_labels() -> void:
	if not is_instance_valid(asc) or not label_state: return

	var display_state = "none"
	var tags = asc.get_tags()
	for tag in tags:
		if tag in TAG_COLORS:
			display_state = String(tag)
			break
	label_state.text = "HUD State: %s" % display_state

func _update_bars() -> void:
	if not is_instance_valid(asc): return

	var hp_bar = get_node_or_null("Panel/VBoxContainer/HealthBar")
	if not hp_bar: hp_bar = get_node_or_null("Panel/MarginContainer/VBoxContainer/HealthBar")
	if hp_bar:
		var hp_label = hp_bar.get_node_or_null("Label")
		var cur_health = asc.get_attribute_value_by_tag(&"health")
		hp_bar.value = cur_health
		if hp_label: hp_label.text = "HP: %d / 100" % int(cur_health)

	var st_bar = get_node_or_null("Panel/VBoxContainer/StaminaBar")
	if not st_bar: st_bar = get_node_or_null("Panel/MarginContainer/VBoxContainer/StaminaBar")
	if st_bar:
		var st_label = st_bar.get_node_or_null("Label")
		var cur_stamina = asc.get_attribute_value_by_tag(&"stamina")
		st_bar.value = cur_stamina
		if st_label: st_label.text = "ST: %d / 100" % int(cur_stamina)

func _update_timer() -> void:
	var timer_label = get_node_or_null("TimerContainer/TimerLabel")
	if not timer_label: return

	var m = int(time_elapsed / 60)
	var s = int(time_elapsed) % 60
	var ms = int((time_elapsed - int(time_elapsed)) * 100)
	timer_label.text = "%02d:%02d:%02d" % [m, s, ms]
	
	if _current_state in TAG_COLORS:
		timer_label.modulate = TAG_COLORS[_current_state]
