extends AbilitySystemCueAnimation

func _init():
	cue_tag = &"cue.animation.attack"
	animation_name = "attack_1"

func _on_execute(spec: AbilitySystemCueSpec):
	var attack_speed = spec.source_asc.get_attribute(&"attack_speed")
	var animation_player = spec.source_asc.get_animation_player()
	
	if animation_player and animation_player.has_animation(animation_name):
		var animation = animation_player.get_animation(animation_name)
		var original_length = animation.length
		
	 # Higher attack_speed = faster animation
		var new_length = original_length / attack_speed
		animation_player.play(animation_name, -1, original_length / new_length)
