extends StaticBody2D

signal gong_struck

var enabled: bool = true

@onready var animation_player: AnimationPlayer = $AnimationPlayer


@onready var asc: ASComponent = $ASComponent

func _ready() -> void:
	if asc:
		asc.attribute_changed.connect(_on_attribute_changed)

func _on_attribute_changed(attribute: StringName, old_value: float, new_value: float) -> void:
	if attribute == &"attribute.health" and new_value < old_value:
		_on_struck()

func _on_health_damaged(_amount: float, _knockback: Vector2) -> void:
	_on_struck()

func _on_struck() -> void:
	if not enabled:
		return
	animation_player.play(&"struck")
	gong_struck.emit()
	enabled = false
