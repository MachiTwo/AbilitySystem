#*
#* hitbox.gd
#* =============================================================================
#* Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
class_name Hitbox
extends Area2D
## Area that deals damage.

## Damage value to apply.
@export var damage: float = 1.0

## Push back the victim.
@export var knockback_enabled: bool = false

## Desired pushback speed.
@export var knockback_strength: float = 500.0


func _ready() -> void:
	area_entered.connect(_area_entered)


func _area_entered(hurtbox: Hurtbox) -> void:
	if hurtbox.owner == owner:
		return
	
	print("DEBUG: Hitbox (", owner.name, ") entered Hurtbox (", hurtbox.owner.name, ")")
	
	# Update direction vector for visual reactions before applying damage
	hurtbox.last_attack_vector = (hurtbox.global_position - global_position).normalized()

	# Try to use AbilitySystem
	var target_asc = hurtbox.owner.get_node_or_null("ASComponent")
	if target_asc:
		var effect = load("res://resources/effects/damage_instant.tres")
		var owner_asc = owner.get_node_or_null("ASComponent")
		if owner_asc:
			var spec = owner_asc.make_outgoing_spec(effect, damage, hurtbox.owner)
			owner_asc.apply_effect_spec_to_target(spec, target_asc)
			return
		else:
			# If owner has no ASC, still apply damage to target
			var spec = target_asc.make_outgoing_spec(effect, damage)
			target_asc.apply_effect_spec_to_self(spec)
			return

	# If target has no ASC, fall back to simple damage (if possible)
	if hurtbox.has_method(&"take_damage"):
		hurtbox.take_damage(damage, self)


func get_knockback() -> Vector2:
	var knockback: Vector2
	if knockback_enabled:
		knockback = Vector2.RIGHT.rotated(global_rotation) * knockback_strength
	return knockback
