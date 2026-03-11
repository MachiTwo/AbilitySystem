extends AbilitySystemComponent

func _on_calculate_custom_magnitude(spec: AbilitySystemEffectSpec, index: int) -> float:
	# index -1: Custom Duration for Effects
	# index -2: Custom Duration for Native Ability Cooldown
	# index -3 and below: Custom Costs for Native Ability Costs
	
	if index == -2: 
		# This is our native attack cooldown calculation
		var attack_speed = get_attribute_value_by_tag(&"attack_speed")
		# Base cooldown 0.5s / attack_speed
		return 0.5 / attack_speed
	
	return 0.0
