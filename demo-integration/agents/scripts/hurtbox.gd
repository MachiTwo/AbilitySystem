#*
#* hurtbox.gd
#* =============================================================================
#* Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
class_name Hurtbox
extends Area2D
## Area that registers damage via AbilitySystem.

var last_attack_vector: Vector2


func take_damage(amount: float, source: Hitbox) -> void:
	last_attack_vector = owner.global_position - source.owner.global_position
	
	# Fallback if ASC is missing but damage is called
	var asc = owner.get_node_or_null("ASComponent")
	if asc:
		var effect = load("res://resources/effects/damage_instant.tres")
		var spec = asc.make_outgoing_spec(effect, amount)
		asc.apply_effect_spec_to_self(spec)
