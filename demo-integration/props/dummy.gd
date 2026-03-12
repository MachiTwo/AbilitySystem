extends CharacterBody2D

@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var hurtbox: Hurtbox = $Hurtbox
@onready var root: Node2D = $Root
@onready var asc: ASComponent = $ASComponent


func _ready() -> void:
	# Link ASC to AnimationPlayer for Cues
	asc.set_animation_player(animation_player)
	
	if asc.get_container():
		asc.apply_container(asc.get_container())
		# Unlock all catalog abilities so they can be activated
		for ab in asc.get_container().get_abilities():
			asc.unlock_ability_by_resource(ab)
	
	# Connect to attribute changes to react to damage via AbilitySystem
	asc.attribute_changed.connect(_on_attribute_changed)


func _on_attribute_changed(attribute: StringName, old_value: float, new_value: float) -> void:
	print("DEBUG: Dummy attribute changed: ", attribute, " from ", old_value, " to ", new_value)
	if attribute == &"attribute.health" and new_value < old_value:
		# Sync direction based on the last hit
		var dir = -signf(hurtbox.last_attack_vector.x)
		if dir != 0:
			root.scale.x = dir
		
		# Trigger the hurt ability through the AbilitySystem
		asc.try_activate_ability_by_tag(&"state.hurt")


func get_facing() -> float:
	return signf(root.scale.x)
