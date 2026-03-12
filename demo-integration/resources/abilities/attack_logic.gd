extends ASAbility

func _on_activate_ability(owner: Object, spec: RefCounted) -> void:
	# owner is ASComponent
	# spec is ASAbilitySpec
	
	# The effects and cues (animations) are handled by the Ability resource automatically.
	# We don't need manual tasks here if the animation itself enables the hitbox.
	pass
