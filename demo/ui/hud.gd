extends CanvasLayer

@onready var health_bar = %HealthBar

func _ready():
	var player = get_tree().get_first_node_in_group("player")
	if player:
		var asc = player.get_node("ASComponent")
		asc.attribute_changed.connect(_on_player_attribute_changed)
		# Initialize value
		_update_health(asc.get_attribute_value_by_tag(&"Health"))

func _on_player_attribute_changed(attr: StringName, _old: float, new: float):
	if attr == &"Health":
		_update_health(new)

func _update_health(value: float):
	health_bar.value = value
